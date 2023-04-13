// Network을 사용하진 않는 2명 game
// keyboard만으로 2명이 게임한다.
// 
// Player 1(O) : 방향키
// Player 2(X) :  a(left), w(up), s(down), d(right)
//
// 중간에 Gold를 먹으면 Score가 증가한다.
//
#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

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

#define STAR1 'O' // player1 표시
#define STAR2 'X' // player2 표시
#define BLANK ' ' // ' ' 로하면 흔적이 지워진다 

#define ESC 0x1b //  ESC 누르면 종료

#define SPECIAL1 0xe0 // 특수키는 0xe0 + key 값으로 구성된다.
#define SPECIAL2 0x00 // keypad 경우 0x00 + key 로 구성된다.

#define UP  0x48 // Up key는 0xe0 + 0x48 두개의 값이 들어온다.
#define DOWN 0x50
#define LEFT 0x4b
#define RIGHT 0x4d

#define UP2		'w'//player2 는 AWSD 로 방향키 대신
#define DOWN2	's'
#define LEFT2	'a'
#define RIGHT2	'd'

#define WIDTH 80
#define HEIGHT 24

int Delay = 100; // 100 msec delay, 이 값을 줄이면 속도가 빨라진다.
int keep_moving = 1; // 1:계속이동, 0:한칸씩이동.
int time_out = 30; // 제한시간
int score[2] = {0};
int golds[WIDTH][HEIGHT] = {0}; // 1이면 Gold 있다는 뜻
int goldinterval = 3; // GOLD 표시 간격
int called[2];

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

void putstar(int x, int y,  char ch)
{
	gotoxy(x,y);
	putchar(ch);
}
void erasestar(int x, int y)
{
	gotoxy(x,y);
	putchar(BLANK);
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

void showscore(int player)
{
	textcolor(GREEN2, GRAY2);
	switch(player) {
	case 0: // player 1
		gotoxy(60,0);
		printf("Player1 : %d", score[player]);
		break;
	case 1: // player 2
		gotoxy(10,0);
		printf("Player2 : %d", score[player]);
		break;
	}
	textcolor(BLACK, WHITE);
}
void player1(unsigned char ch)
{
	static int oldx=60,oldy=10, newx=60, newy=10;
	int move_flag = 0;
	static unsigned char last_ch=0;
	
	if (called[0] == 0) { // 처음 또는 Restart
		oldx=60,oldy=10, newx=60, newy=10;
		putstar(oldx,oldy,STAR1);
		called[0] = 1;
		last_ch = 0;
		ch = 0;
	}
	if (keep_moving && ch==0)
		ch = last_ch;
	last_ch = ch;

	switch(ch) {
	case UP:
		if (oldy>1) // 0 은 Status Line
			newy = oldy - 1;
		else { // 벽에 부딛치면 방향을 반대로 이동
			newy = oldy + 1;
			last_ch = DOWN;
		}
		move_flag = 1;
		break;
	case DOWN:
		if (oldy < HEIGHT - 1)
			newy = oldy + 1;
		else {
			newy = oldy - 1;
			last_ch = UP;
		}
		move_flag = 1;
		break;
	case LEFT:
		if (oldx > 0 )
			newx = oldx - 1;
		else {
			newx = oldx + 1;
			last_ch = RIGHT;
		}
		move_flag = 1;
		break;
	case RIGHT:
		if (oldx < WIDTH - 1)
			newx = oldx + 1;
		else {
			newx = oldx - 1;
			last_ch = LEFT;
		}
		move_flag = 1;
		break;
	}
	if (move_flag) {
		erasestar(oldx, oldy); // 마지막 위치의 * 를 지우고
		putstar(newx, newy, STAR1); // 새로운 위치에서 * 를 표시한다.
		oldx = newx; // 마지막 위치를 기억한다.
		oldy = newy;
		if (golds[newx][newy]) {
			score[0]++;
			golds[newx][newy] = 0;
			showscore(0);
		}
	}

}
void player2(unsigned char ch)
{
	static int oldx=20,oldy=10, newx=20, newy=10;
	int move_flag = 0;
	static char last_ch=0;
	
	if (called[1] == 0) { // 처음 또는 Restart
		oldx=20,oldy=10, newx=20, newy=10;
		putstar(oldx,oldy,STAR2);
		called[1] = 1;
		last_ch = 0;
		ch = 0;
	}
	// 방향키가 눌려지지 않은 경우 예전에 이동하던 방향으로 계속 이동
	if (keep_moving && ch==0) 
		ch = last_ch;
	last_ch = ch;
		
	switch(ch) {
	case UP2:
		if (oldy>1) // 0 은 Status Line
			newy = oldy - 1;
		else { // 벽에 부딛치면 방향을 반대로 이동
			newy = oldy + 1;
			last_ch = DOWN2;
		}
		move_flag = 1;
		break;
	case DOWN2:
		if (oldy < HEIGHT - 1)
			newy = oldy + 1;
		else {
			newy = oldy - 1;
			last_ch = UP2;
		}
		move_flag = 1;
		break;
	case LEFT2:
		if (oldx > 0 )
			newx = oldx - 1;
		else {
			newx = oldx + 1;
			last_ch = RIGHT2;
		}
		move_flag = 1;
		break;
	case RIGHT2:
		if (oldx < WIDTH - 1)
			newx = oldx + 1;
		else {
			newx = oldx - 1;
			last_ch = LEFT2;
		}
		move_flag = 1;
		break;
	}
	if (move_flag) {
		erasestar(oldx, oldy); // 마지막 위치의 # 를 지우고
		putstar(newx, newy, STAR2); // 새로운 위치에서 # 를 표시한다.
		oldx = newx; // 마지막 위치를 기억한다.
		oldy = newy;
		move_flag = 1; // 1:계속 이동, 0:한칸씩 이동
		if (golds[newx][newy]) {
			score[1]++;
			golds[newx][newy] = 0;
			showscore(1);
		}
	}
}

// 임의의 위치에 gold를 표시한다.
#define GOLD	"*"
void show_gold()
{
	int x,y;
	x = rand()%WIDTH;
	y = rand()%(HEIGHT-1)+1;  // 제일 상단은 피한다
	textcolor(YELLOW2,GRAY1);
	gotoxy(x, y);
	printf(GOLD);
	golds[x][y] = 1;
	textcolor(BLACK,WHITE);
}

// box 그리기 함수, ch 문자로 (x1,y1) ~ (x2,y2) box를 그린다.
void draw_box(int x1, int y1, int x2, int y2, char ch)
{
	int x, y;
	for (x=x1;x<=x2;x+=1) {
		gotoxy(x, y1);
		printf("%c", ch);
		gotoxy(x, y2);
		printf("%c", ch);
	}
	for (y=y1;y<=y2;y++) {
		gotoxy(x1, y);
		printf("%c", ch);
		gotoxy(x2, y);
		printf("%c", ch);
	}
}

// box 그리기 함수, ch 문자열로 (x1,y1) ~ (x2,y2) box를 그린다.
// 한글 문자를 그리는 용도로 사용 "□" 로 벽을 그리는 경우
void draw_box2(int x1, int y1, int x2, int y2, char *ch)
{
	int x, y;
	int len = strlen(ch);
	for (x=x1;x<=x2;x+=len) {
		gotoxy(x, y1);
		printf("%s", ch);
		gotoxy(x, y2);
		printf("%s", ch);
	}
	for (y=y1;y<=y2;y++) {
		gotoxy(x1, y);
		printf("%s", ch);
		gotoxy(x2, y);
		printf("%s", ch);
	}
}

// 가로줄 
void draw_hline(int y, int x1, int x2, char ch)
{
	gotoxy(x1, y);
	for (;x1<=x2;x1++)
		putchar(ch);
}

//게임 초기화, 처음 시작과 Restar때 호출
void init_game()
{
	int x,y;
	char cmd[100];

	srand(time(NULL));
	score[0] = score[1] = 0;
	called[0] = called[1] = 0;
	for (x=0;x<WIDTH;x++)
		for (y=0;y<HEIGHT;y++)
			golds[x][y] = 0;
	goldinterval = 3;
	time_out = 30;
	keep_moving = 1;
	Delay = 100;

	cls(BLACK, WHITE);
	removeCursor();
	textcolor(BLACK, GRAY2);
	draw_hline(0,0,79,' ');
	textcolor(BLACK, WHITE);
	sprintf(cmd, "mode con cols=%d lines=%d", WIDTH, HEIGHT);
	system(cmd);
}

void show_time(int remain_time)
{
	textcolor(YELLOW2, GRAY2);
	gotoxy(30, 0);
	printf("남은시간 : %02d", remain_time);
	textcolor(BLACK, WHITE);
}

void main()
{
	// 특수키 0xe0 을 입력받으려면 unsigned char 로 선언해야 함
	unsigned char ch;
	int run_time, start_time, remain_time, last_remain_time;
	int gold_time;

START:
	init_game();
	gold_time = 0;
	start_time=time(NULL);
	last_remain_time = remain_time = time_out;

	showscore(0);
	showscore(1);
	show_time(remain_time);
	while (1) {
		// 시간 check
		run_time = time(NULL) - start_time;
		if (run_time>gold_time && (run_time%goldinterval==0)) {
			show_gold();
			gold_time = run_time; // 마지막 GOLD 표시 시간 기억
		}
		remain_time = time_out - run_time;
		if (remain_time<last_remain_time) {
			show_time(remain_time); // 시간이 변할때만 출력
			last_remain_time = remain_time;
		}
		if (remain_time==0) // 시간 종료
			break;

		if (kbhit()==1) {  // 키보드가 눌려져 있으면
			ch = getch(); // key 값을 읽는다
			// ESC 누르면 프로그램 종료
			if (ch == ESC) {
				//gotoxy(25,11);
				//exit(0);
				break;
			}
			if (ch==SPECIAL1 || ch==SPECIAL2) { // 만약 특수키
				// 예를 들어 UP key의 경우 0xe0 0x48 두개의 문자가 들어온다.
				ch = getch();
				// Player1은 방향키로 움직인다.
				switch(ch) {
				case UP:
				case DOWN:
				case LEFT:
				case RIGHT:
					player1(ch);//player1만 방향 전환
					player2(0);
					break;
				default: // 방향 전환이 아니면
					player1(0);
					player2(0);
				}
			} else {
				// Player2은 AWSD 로 움직인다.
				switch(ch) {
				case UP2:
				case DOWN2:
				case LEFT2:
				case RIGHT2://player2만 방향 전환
					player2(ch);
					player1(0);
					break;
				default:// 방향 전환이 아니면
					player1(0);
					player2(0);
				}
			}
		} else { 
			// keyboard 가 눌려지지 않으면 계속 움직인다.
			// 이동중이던 방향으로 계속 이동
			player1(0);
			player2(0);
		}
		Sleep(Delay);
	}
	gotoxy(30, 7);
	textcolor(GREEN2, BLACK);
	if (score[0]>score[1])
		printf(" WINNER IS PLAYER1!! ");
	else if (score[0]<score[1])
		printf(" WINNER IS PLAYER2!! ");
	else
		printf("       DRAW!!        ");
	while (1) {
		int c1, c2;
		do { // 색을 변경하면서 Game Over 출력
			c1 = rand()%16;
			c2 = rand()%16;
		} while (c1==c2);
		textcolor(c1, c2);
		gotoxy(32, 10);
		printf("** Game Over **");
		gotoxy(24, 13);
		textcolor(WHITE, BLACK);
		printf("Hit (R) to Restart (Q) to Quit");
		Sleep(300);
		if (kbhit()) {
			ch = getch();
			if (ch=='r' || ch=='q')
				break;
		}
	}
	if (ch=='r') 
		goto START;
	gotoxy(0,HEIGHT-1);
}
