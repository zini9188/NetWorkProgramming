// 2016 1�б� ��Ʈ��ũ���α׷��� - 11��
// �� ���α׷��� gotoxy()�� �̿��Ͽ�
// keyboard ����Ű�� ������ * ����� �����Դϴ�.
//
#include <stdio.h>
#include <conio.h>
#include <Windows.h>

// ���� ����
#define BLACK	0
#define BLUE1	1
#define GREEN1	2
#define CYAN1	3
#define RED1	4
#define MAGENTA1 5
#define YELLOW1	6
#define GRAY1	7
#define GRAY2	8
#define BLUE2	9
#define GREEN2	10
#define CYAN2	11
#define RED2	12
#define MAGENTA2 13
#define YELLOW2	14
#define WHITE	15

#define STAR '*'
#define BLANK ' '

#define ESC 0x1b

#define SPECIAL1 0xe0 // Ư��Ű�� 0xe0 + key ������ �����ȴ�.
#define SPECIAL2 0x00 // keypad ��� 0x00 + key �� �����ȴ�.

#define UP  0x48 // Up key�� 0xe0 + 0x48 �ΰ��� ���� ���´�.
#define DOWN 0x50
#define LEFT 0x4b
#define RIGHT 0x4d

#define UP2		'w'
#define DOWN2	's'
#define LEFT2	'a'
#define RIGHT2	'd'

#define WIDTH 80
#define HEIGHT 24

int Delay = 100; // 100 msec delay, �� ���� ���̸� �ӵ��� ��������.

void removeCursor(void){ // Ŀ���� �Ⱥ��̰� �Ѵ�

	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible=0;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}

void gotoxy(int x, int y) //���� ���ϴ� ��ġ�� Ŀ�� �̵�
{
	COORD pos = {x, y};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);// WIN32API �Լ��Դϴ�. �̰� ���ʿ� �����
}

void putstar(int x,int y,char ch)
{
	gotoxy(x,y);
	putchar(ch);
}
void erasestar(int x,int y)
{
	gotoxy(x,y);
	putchar(' ');//(BLANK);
}

void textcolor(int fg_color, int bg_color)
{
	SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), fg_color | bg_color<<4);
}
// ȭ�� ������ ���ϴ� �������� �����Ѵ�.
void cls(int text_color, int bg_color) 
{
	char cmd[100];
	system("cls");
	sprintf(cmd, "COLOR %x%x", bg_color, text_color);
	system(cmd);

}
// box �׸��� �Լ�, ch ���ڿ��� (x1,y1) ~ (x2,y2) box�� �׸���.
void draw_box(int x1, int y1, int x2, int y2, char ch)
{
	int x, y;
	
}
// box �׸��� �Լ�, ch ���ڿ��� (x1,y1) ~ (x2,y2) box�� �׸���.
void draw_box2(int x1, int y1, int x2, int y2, char *ch)
{
	int x, y;
	int len = strlen(ch);
}

// ����Ű�� * �� �����δ�.
void main()
{
	unsigned char ch; // Ư��Ű 0xe0 �� �Է¹������� unsigned char �� �����ؾ� ��
	int oldx,oldy, newx, newy;
	int keep_moving;
	
	newx = oldx = 10;
	newy = oldy = 10;

	removeCursor();
	cls(BLACK, WHITE);
	putstar(oldx,oldy,STAR);
	ch = 0; // �ʱⰪ ��������
	keep_moving = 0;

	while (1) {
		if (kbhit()==1) {  // Ű���尡 ������ ������
			ch = getch(); // key ���� �д´�
			// ESC ������ ���α׷� ����
			if (ch == ESC)
				exit(0);
			if (ch==SPECIAL1 || ch==SPECIAL2) { // ���� Ư��Ű
				// ���� ��� UP key�� ��� 0xe0 0x48 �ΰ��� ���ڰ� ���´�.
				ch = getch();
				switch(ch) {
				case UP:
				case DOWN:
				case LEFT:
				case RIGHT:
					keep_moving = 1;
					break;
				default: // ����Ű�� �ƴϸ� �����
					keep_moving = 0;
				}
			} else { 
				// Ư�� ���ڰ� �ƴ����� ����Ű ��� ����ϴ� ��� ó��
				switch(ch) {
				case UP2: ch = UP;
					keep_moving = 1;
					break;
				case DOWN2: ch = DOWN;
					keep_moving = 1;
					break;
				case RIGHT2: ch = RIGHT;
					keep_moving = 1;
					break;
				case LEFT2: ch = LEFT;
					keep_moving = 1;
					break;
				default: // ASWD �ƴϸ� �����
					keep_moving = 0;
					break;
				}
			}
		} 
		if (keep_moving) { // �����̰� ������
			//
			// ���� ������ ������ �����ϱ� �߰�
			// ���� ������ Delay �� ���ҽ�Ű�� �ӵ��� ��������.
			//			
			switch(ch) {
			case UP:
				if (oldy>0)
					newy = oldy - 1;
				break;
			case DOWN:
				if (oldy < HEIGHT - 1)
					newy = oldy + 1;
				break;
			case LEFT:
				if (oldx > 0 )
					newx = oldx - 1;
				break;
			case RIGHT:
				if (oldx < WIDTH - 1)
					newx = oldx + 1;
				break;
			}
			erasestar(oldx, oldy); // ������ ��ġ�� * �� �����
			putstar(newx, newy, STAR); // ���ο� ��ġ���� * �� ǥ���Ѵ�.
			oldx = newx; // ������ ��ġ�� ����Ѵ�.
			oldy = newy;
			keep_moving = 0; //1; //1:����̵�, 0:�ѹ��� ��ĭ���̵�
		}
		Sleep(Delay); // Delay�� ���̸� �ӵ��� ��������.
	}

}
