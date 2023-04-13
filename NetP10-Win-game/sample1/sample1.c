// 2016 1학기 네트워크프로그래밍 - 11주
// 이 프로그램은 gotoxy()를 이용하여
// keyboard 방향키를 누르면 * 모양이 움직입니다.
//
#include <stdio.h>
#include <conio.h>
#include <Windows.h>

// 색상 정의
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

#define SPECIAL1 0xe0 // 특수키는 0xe0 + key 값으로 구성된다.
#define SPECIAL2 0x00 // keypad 경우 0x00 + key 로 구성된다.

#define UP  0x48 // Up key는 0xe0 + 0x48 두개의 값이 들어온다.
#define DOWN 0x50
#define LEFT 0x4b
#define RIGHT 0x4d

#define UP2		'w'
#define DOWN2	's'
#define LEFT2	'a'
#define RIGHT2	'd'

#define WIDTH 80
#define HEIGHT 24

int Delay = 100; // 100 msec delay, 이 값을 줄이면 속도가 빨라진다.

void removeCursor(void){ // 커서를 안보이게 한다

	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible=0;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}

void gotoxy(int x, int y) //내가 원하는 위치로 커서 이동
{
	COORD pos = {x, y};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);// WIN32API 함수입니다. 이건 알필요 없어요
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
// 화면 지우기고 원하는 배경색으로 설정한다.
void cls(int text_color, int bg_color) 
{
	char cmd[100];
	system("cls");
	sprintf(cmd, "COLOR %x%x", bg_color, text_color);
	system(cmd);

}
// box 그리기 함수, ch 문자열로 (x1,y1) ~ (x2,y2) box를 그린다.
void draw_box(int x1, int y1, int x2, int y2, char ch)
{
	int x, y;
	
}
// box 그리기 함수, ch 문자열로 (x1,y1) ~ (x2,y2) box를 그린다.
void draw_box2(int x1, int y1, int x2, int y2, char *ch)
{
	int x, y;
	int len = strlen(ch);
}

// 방향키로 * 가 움직인다.
void main()
{
	unsigned char ch; // 특수키 0xe0 을 입력받으려면 unsigned char 로 선언해야 함
	int oldx,oldy, newx, newy;
	int keep_moving;
	
	newx = oldx = 10;
	newy = oldy = 10;

	removeCursor();
	cls(BLACK, WHITE);
	putstar(oldx,oldy,STAR);
	ch = 0; // 초기값 정지상태
	keep_moving = 0;

	while (1) {
		if (kbhit()==1) {  // 키보드가 눌려져 있으면
			ch = getch(); // key 값을 읽는다
			// ESC 누르면 프로그램 종료
			if (ch == ESC)
				exit(0);
			if (ch==SPECIAL1 || ch==SPECIAL2) { // 만약 특수키
				// 예를 들어 UP key의 경우 0xe0 0x48 두개의 문자가 들어온다.
				ch = getch();
				switch(ch) {
				case UP:
				case DOWN:
				case LEFT:
				case RIGHT:
					keep_moving = 1;
					break;
				default: // 방향키가 아니면 멈춘다
					keep_moving = 0;
				}
			} else { 
				// 특수 문자가 아니지만 방향키 대신 사용하는 경우 처리
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
				default: // ASWD 아니면 멈춘다
					keep_moving = 0;
					break;
				}
			}
		} 
		if (keep_moving) { // 움직이고 있으면
			//
			// 벽을 만나면 방향을 변경하기 추가
			// 벽을 만날때 Delay 를 감소시키면 속도가 빨라진다.
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
			erasestar(oldx, oldy); // 마지막 위치의 * 를 지우고
			putstar(newx, newy, STAR); // 새로운 위치에서 * 를 표시한다.
			oldx = newx; // 마지막 위치를 기억한다.
			oldy = newy;
			keep_moving = 0; //1; //1:계속이동, 0:한번에 한칸씩이동
		}
		Sleep(Delay); // Delay를 줄이면 속도가 빨라진다.
	}

}
