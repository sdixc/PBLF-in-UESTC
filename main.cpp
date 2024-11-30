#include <stdio.h>
#include <graphics.h>
#include "tools.h"
#include <time.h>
#include <math.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#define WIN_WIDTH	1000
#define	WIN_HEIGHT	600
enum 
{
	WAN_DOU, XIANG_RI_KUI, ZHI_WU_COUNT
};

IMAGE imgBg;
IMAGE imgBar5;
IMAGE imgCards[ZHI_WU_COUNT];
IMAGE *imgZhiWu[ZHI_WU_COUNT][20];

int curX, curY;//当前选中的植物，在移动过程中的位置
int curZhiWu;	//0未选中 1第一种植物

struct zhiwu {
	int type;			//0没有植物 1选择第一种植物
	int frameIndex;		//序列帧的序号
};

struct zhiwu map[3][9];
struct sunshineBall {
	int x , y;//飘落过程位置 x不变
	int frameIndex;
	int destY;//飘落终点y坐标
	bool used;//是否在使用
        int timer;
        float xoff;
        float yoff;
};
struct sunshineBall balls[10];
IMAGE imgSunshineBall[29];
int sunshine=50;
struct zm {
    int x, y;
    int frameIndex;
    bool used;
    int speed;
    int row ;
    int blood; 
};
struct zm zms[10];
IMAGE imgZM[22];

//子弹的数据类型
struct bullet {
    int x, y;
    int row;
    bool used;
    int speed;
    bool blast;
    int frameIndex;
};

struct bullet bullets[30];
IMAGE imgBulletNormal;
IMAGE imgBullBlast[4];

bool fileExist(const char* name) {

	FILE* fp = fopen(name, "r");
	if (fp == NULL) {
		return false;
	}
	else {
		fclose(fp);
		return true;
	}
}


void gameInit() {
	//加载背景图片
	loadimage(&imgBg, "res/bg.jpg");
	loadimage(&imgBar5, "res/bar5.png");

	memset(imgZhiWu, 0, sizeof(imgZhiWu));
	memset(map, 0, sizeof(map));

	//初始化植物卡牌
	char name[64];
	for (int i = 0; i < ZHI_WU_COUNT; i++) {
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);
		loadimage(&imgCards[i], name);
	
		for (int j = 0; j < 20; j++) {
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i,j + 1);
			//先判断这个文件是否存在
			if (fileExist(name)) {
				imgZhiWu[i][j] = new IMAGE;
				loadimage(imgZhiWu[i][j], name);
			}
			else {
				break;
			}
		}
	}

	curZhiWu = 0;

	memset(balls, 0, sizeof(balls));
	for (int i = 0; i < 29; i++) {
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
		loadimage(&imgSunshineBall[i], name);
	}

	//配置随机种子
	srand(time(NULL));

	//创建游戏图形窗口
	initgraph(WIN_WIDTH,WIN_HEIGHT, 1);

	// 设置字体
LOGFONT f;
gettextstyle(&f);
f.lfHeight = 30;
f.lfWeight = 15;
strcpy(f.lfFaceName, "Segoe UI Black");
f.lfQuality = ANTIALIASED_QUALITY; // 抗锯齿效果
settextstyle(&f);
setbkmode(TRANSPARENT);
setcolor(BLACK);

// 初始化僵尸数据
memset(zms, 0, sizeof(zms));
for (int i = 0; i < 22; i++) {
    sprintf_s(name, sizeof(name), "res/zombie/%d.png", i);
    loadimage(&imgZM[i], name);
}
loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");
memset(bullets, 0, sizeof(bullets));
}
// 初始化豌豆子弹的帧图片数组
loadimage(&imgBullBlast[3], "res/bullets/bullet_blast.png");
for (int i = 0; i < 3; i++) {
    float k = (i + 1) * 0.2;
    loadimage(&imgBullBlast[i], "res/bullets/bullet_blast.png",
              imgBullBlast[3].getwidth() * k,
              imgBullBlast[3].getheight() * k, true);
}
}
char scoreText[8];
sprintf_s(scoreText,sizeof(scoreText),"%d",sunshine);
outtextxy(276,67,scoreText);

drawZM();

int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
for (int i = 0; i < bulletMax; i++) {
    if (bullets[i].used) {
	    if (bullets[i].blast) {
            IMAGE* img = &imgBullBlast[bullets[i].frameIndex];
            putimagePNG(bullets[i].x, bullets[i].y, img);
        } else {
        putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
    }
}
}
EndBatchDraw();//结束双缓冲

void collectSunshine(ExMessage* msg) {
    int count = sizeof(balls) / sizeof(balls[0]);
    int w = imgSunshineBall[0].getWidth();
    int h = imgSunshineBall[0].getHeight();
    for (int i = 0; i < count; i++) {
        if (balls[i].used) {
            int x = balls[i].x;
            int y = balls[i].y;
            if (msg->x > x && msg->x < x + w &&
                msg->y > y && msg->y < y + h) {
                balls[i].used = false;
               //sunshine += 25;
                mciSendString("play res/sunshine.mp3",0,0,0);
	    }
	}
    }
void userClick() {
	ExMessage msg;
	static int status = 0;
	if (peekmessage(&msg)) {
		if (msg.message == WM_LBUTTONDOWN) {
			if(msg.x >338 && msg.x < 338+64*ZHI_WU_COUNT && msg.y < 96){
				int index = (msg.x - 338) / 65;
				//printf("%d\n", index);
				status = 1;
				curZhiWu = index + 1;
				else{
					collectSunshine(&msg);
			}
		}else if(msg.message == WM_MOUSEMOVE &&status == 1){
			curX = msg.x;
			curY = msg.y;
		}else if(msg.message == WM_LBUTTONUP){
			if (msg.x > 256 && msg.y > 179 && msg.y < 489) {
				int row = (msg.y - 179) / 102;
				int col = (msg.x - 256) / 81;
				//printf("%d,%d\n", row, col);

				if (map[row][col].type == 0) {
					map[row][col].type = curZhiWu;
					map[row][col].frameIndex = 0;
				}
			}
			curZhiWu = 0;
			status = 0;
		}
	}
}
void drawZM() {
    int zmCount = sizeof(zms) / sizeof(zms[0]);
    // 遍历每个僵尸
    for (int i = 0; i < zmCount; i++) {
        if (zms[i].used) {
            // 获取僵尸对应的图像指针，imgZM是图像数组，frameIndex是图像帧索引
            IMAGE* img = &imgZM[zms[i].frameIndex];
            // 在指定位置绘制图像，x和y是僵尸的坐标，img->getheight()获取图像高度
            putimagePNG(zms[i].x, zms[i].y - img->getheight(), img);
        }
    }
}
void updateWindow() {
	BeginBatchDraw();//开始缓冲

	putimage(0, 0, &imgBg);
	putimagePNG(250, 0, &imgBar5);
	for (int i = 0; i < ZHI_WU_COUNT; i++) {
		int x = 338 + i * 64;
		int y = 6;
		putimage(x, y, &imgCards[i]);
	}
	

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type > 0) {
				int x = 256 + j * 81;
				int y = 179 + i * 102;
				int zhiWuType = map[i][j].type - 1;
				int index = map[i][j].frameIndex;
				putimagePNG(x, y, imgZhiWu[zhiWuType][index]);
			}

		}
	}

	//渲染拖动过程中的植物
	if (curZhiWu > 0) {
		IMAGE* img = imgZhiWu[curZhiWu - 1][0];
		putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);
	}

	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++) {
		if (balls[i].used||balls[i].xoff) {
			IMAGE* img = &imgSunshineBall[balls[i].frameIndex];
			putimagePNG(balls[i].x, balls[i].y, img);
		}
	}
	EndBatchDraw();//结束双缓冲
}


void creatSunshine() {
	static int fre = 400;
	static int count = 0;
	count++;
	if (count >= fre) {
		fre = 200 + rand() % 200;
		count = 0;

		int ballMax = sizeof(balls) / sizeof(balls[0]);
		int i;
		for (i = 0; i < ballMax && balls[i].used; i++);
		if (i >= ballMax)return;

		balls[i].used = true;
		balls[i].frameIndex = 0;
		balls[i].x = 260 + rand() % (900 - 260);
		balls[i].y = 60;
		balls[i].destY = (rand() % 4) * 90 + 200;
		balls[i].xoff = 0;
		balls[i].yoff = 0;
	}
}

void updateSunshine() {
	int BallMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < BallMax; i++) {
		if (balls[i].used) {
			balls[i].frameIndex = (balls[i].frameIndex + 1) % 29;
			if(balls[i].timer==0)
			{balls[i].y += 2;
			}
			if (balls[i].y >= balls[i].destY) {
				balls[i].timer++;
				if(balls[i].timer>100)
				{balls[i].used = false;
			}
		}
	}else if (balls[i].xoff) {
			 float destY = 0;
        float destX = 262;
        float angle = atan((balls[i].y - destY) / (balls[i].x - destX));
        balls[i].xoff = 4 * cos(angle);
        balls[i].yoff = 4 * sin(angle);
			
    balls[i].x += balls[i].xoff;
    balls[i].y += balls[i].yoff;
    if (balls[i].y < 0 || balls[i].x < 262) {
        balls[i].xoff = 0;
        balls[i].yoff = 0;
        sunshine += 25;
    }
}
}
void createZM() {
static int zmFre = 300;
static int count = 0;
    count++;
    if (count > zmFre) {
        count = 0;
        zmFre = rand() % 200 + 300;
    }
    int i;
    int zmMax = sizeof(zms) / sizeof(zms[0]);
    for (i = 0; i < zmMax && zms[i].used; i++);
    if (i < zmMax) {
        zms[i].used = true;
        zms[i].x = WIN_WIDTH;
	zms[i].row = rand() % 3;
        zms[i].y = 172 + (1 + zms[i].row) * 100;
        zms[i].speed = 1; 
	zms[i].blood = 100;
    }
}
}

void updateZM() {
    int zmMax = sizeof(zms) / sizeof(zms[0]);
	static int count = 0;
    count++;
    if (count > 2) {
        count = 0;
    // 更新僵尸的位置
    for (int i = 0; i < zmMax; i++) {
        if (zms[i].used) {
            zms[i].x -= zms[i].speed;
            if (zms[i].x < 170) {
                std::cout << "GAME OVER\n";
                MessageBox(NULL, "over", "over", 0); 
                exit(0); 
            }
        }
    }
}
static int count2 = 0;
    count2++;
    if (count2 > 4) {
        count2 = 0;
    for (int i = 0; i < zmMax; i++) {
        if (zms[i].used) {
            zms[i].frameIndex = (zms[i].frameIndex + 1) % 22;
        }
    }
}
    }
void shoot(){
int lines[3] = {0};
    int zmCount = sizeof(zms) / sizeof(zms[0]);
    int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
    int dangerX = WIN_WIDTH - imgZM[0].getwidth();
    for (int i = 0; i < zmCount; i++)
    {
        if (zms[i].used && zms[i].x < dangerX)
        {
            lines[zms[i].row] = 1;
        }
    }
    for (int i = 0; i < 3; i++)
    {for (int j = 0; j < 9; j++) {
        if (map[i][j].type == WAN_DOU + 1 && lines[i]) {
            static int count = 0;
            count++;
if (count > 20) {
    count = 0;
    int k;
    for (k = 0; k < bulletMax && bullets[k].used; k++) ;
    if (k < bulletMax) {
        bullets[k].used = true;
        bullets[k].row = i;
        bullets[k].speed = 6;
	bullets[k].blast = false;
	bullets[[k].frameIndex =  0;
        int zWX = 256 + j * 81;
        int zWY = 179 + i * 102 + 14;
        bullets[k].x = zWX + imgZhiWuMap[i][j].type - 1 [0] -> getwidth() - 10;
        bullets[k].y = zWY + 5;
    }
}
void updateBullets() {
    int countMax = sizeof(bullets) / sizeof(bullets[0]);
    for (int i = 0; i < countMax; i++) {
        if (bullets[i].used) {
            bullets[i].x += bullets[i].speed;
            if (bullets[i].x > WIN_WIDTH) {
                bullets[i].used = false;
            }
        }
        // 待实现子弹的碰撞检测
        if (bullets[i].blast) {
            bullets[i].frameIndex++;
            if (bullets[i].frameIndex >= 4) {
                bullets[i].used = false;
            }
        }
    }
}
void collisionCheck() {
    int bCount = sizeof(bullets) / sizeof(bullets[0]);
    int zCount = sizeof(zms) / sizeof(zms[0]);
    for (int i = 0; i < bCount; i++) {
        if (bullets[i].used == false || bullets[i].blast) continue;
        for (int k = 0; k < zCount; k++) {
            if (zms[k].used == false) continue;
            int x1 = zms[k].x + 80;
            int x2 = zms[k].x + 110;
            int x = bullets[i].x;
            if (bullets[i].row == zms[k].row && x > x1 && x < x2) {
        zms[k].blood -= 20;
        bullets[i].blast = true;
        bullets[i].speed = 0;
    }
}
    }
}
void updateGame() {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type > 0) {
				map[i][j].frameIndex++;
				int zhiWuType = map[i][j].type - 1;
				int index = map[i][j].frameIndex;
				if (imgZhiWu[zhiWuType][index] == NULL) {
					map[i][j].frameIndex = 0;
				}
			}
		}
	}
	creatSunshine();
	updateSunshine();
	createZM();
	updateZM();
	shoot();
	updateBullets():
        collisionCheck():
}
void startUI() {
	IMAGE imgBg, imgMenu1, imgMenu2;
	loadimage(&imgBg, "res/menu.png");
	loadimage(&imgMenu1, "res/menu1.png");
	loadimage(&imgMenu2, "res/menu2.png");
	int flag = 0;

	while (1) {
		BeginBatchDraw();
		putimage(0, 0, &imgBg);
		putimagePNG(474, 75,flag? &imgMenu2 :&imgMenu1);
		
		ExMessage msg;
		if (peekmessage(&msg)) {
			if (msg.message == WM_LBUTTONDOWN &&
				msg.x>474 &&msg.x < 474+300 &&
				msg.y >75 &&msg.y <75 +140) {
				flag = 1;
			}
			else if (msg.message == WM_LBUTTONUP &&flag) {
				return;
			}
		}
		EndBatchDraw();
	}
}

int main(void) {
	gameInit();
	startUI();
	int timer = 0;
	bool flag = true;
	while (1) {
		userClick();
		timer += getDelay();
		if (timer > 20) {
			flag = true;
			timer = 0;
		}
		if (flag){
			flag = false;
			updateWindow();
			updateGame();
		}
	}
		system("pause");
	return 0;
}
