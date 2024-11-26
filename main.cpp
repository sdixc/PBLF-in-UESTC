#include <stdio.h>
#include <graphics.h>
#include "tools.h"
#include <time.h>

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
	bool used;	//是否在使用
};
struct sunshineBall balls[10];
IMAGE imgSunshineBall[29];


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
		if (balls[i].used) {
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
	}
}

void updateSunshine() {
	int BallMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < BallMax; i++) {
		if (balls[i].used) {
			balls[i].frameIndex = (balls[i].frameIndex + 1) % 29;
			balls[i].y += 2;

			if (balls[i].y >= balls[i].destY) {
				balls[i].used = false;
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