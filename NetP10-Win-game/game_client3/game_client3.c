//
// game_client3.c 
// 1:1 게임 모드에서 내가 방향키로 움직이면 key를 누르지 않아도
// 계속 움직이며, 상대편에도 같은 움직임이 표시된다.
// UP, DOWN, LEFT, RIGHT 키 입력 상황만 전달한다.
// 
//
#define _CRT_SECURE_NO_WARNINGS

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
#define BLANK '.' // ' ' 로하면 흔적이 지워진다 

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

#define NONE	'X' // 움직이지 않는다.

#define WIDTH 80
#define HEIGHT 24

int Delay = 50; // 50 msec delay, 이 값을 줄이면 속도가 빨라진다.
int keep_moving = 1; // 1:계속이동, 0:한칸씩이동.
int time_out = 30; // 제한시간
int score[2] = {0};
int golds[WIDTH][HEIGHT] = {0}; // 1이면 Gold 있다는 뜻
int goldinterval = 3; // GOLD 표시 간격
int called[2];

#define STARTX	40
#define STARTY	10

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

void showname(int x, int y,  char *name)
{
	gotoxy(x,y);
	printf(name);
}
void erasename(int x, int y, char *name)
{
	int len;
	gotoxy(x,y);
	len = strlen(name);
	while (len--)
		putchar(' '); // blank 로 지운다.
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
#include <winsock.h>
#include <signal.h>
#include <stdio.h>
#include <conio.h>

char *escapechar = "exit";
char myname[256];  					/* 채팅에서 사용할 이름 */
WSADATA wsadata;
int	main_socket;

void exit_callback(int sig)
{
	closesocket(main_socket);
	WSACleanup();
	exit(0);
}

int connect_server(char *ip_addr, int port_no)
{
	struct sockaddr_in server_addr;
	int s;  /* 서버와 연결된 소켓번호 */
	WORD sversion;
	u_long iMode = 1;

	// winsock 사용을 위해 필수적임
	signal(SIGINT, exit_callback);
	sversion = MAKEWORD(1, 1);
	WSAStartup(sversion, &wsadata);


	/* 소켓 생성 */
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Client : Can't open stream socket.\n");
		exit(0);

	}

	/* 채팅 서버의 소켓주소 구조체 server_addr 초기화 */
	memset((char *)&server_addr, '\0', sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip_addr);
	server_addr.sin_port = htons(port_no);

	/* 연결요청 */
	if (connect(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		printf("Client : Can't connect to server.\n");
		exit(0);
	}
	else {
		gotoxy(0, 0);
		printf("%s 서버에 접속되었습니다.", myname);
	}

	ioctlsocket(s, FIONBIO, &iMode); // read 를 non-blocking 으로 만든다.
	return s;
}

void movename(char *name, int oldx, int oldy, int newx, int newy)
{
	erasename(oldx, oldy, name);
	showname(newx, newy, name);
	fflush(stdout);
}
#define CONNECT "CONN"
#define MOVE	"MOVE"

#define MOVEUP	"MOVEUP"
#define MOVEDOWN "MOVEDOWN"
#define MOVELEFT "MOVELEFT"
#define MOVERIGHT "MOVERIGHT"
#define MOVENONE "MOVENONE"

#define BUF_LEN 128
#define BUFSIZE	1500
#define MAXREAD	((BUFSIZE/MSG_LENGTH)*MSG_LENGTH) // MSG_LENGTH 에 align해서 읽는다.

void sendmypos(int sd, char *name, int oldx, int oldy, int newx, int newy)
{
	char buf[BUF_LEN];
	sprintf(buf, "%s %s %d %d %d %d", MOVE, name, oldx, oldy, newx, newy);
	send(sd, buf, BUF_LEN, 0);  // NULL 포함 전송.
}

void sendmymove(int sd, char *cmd, char *name,  int oldx, int oldy, int newx, int newy)
{
	char buf[BUF_LEN];
	sprintf(buf, "%s %s %d %d %d %d", cmd, name, oldx, oldy, newx, newy);
	send(sd, buf, BUF_LEN, 0);  // NULL 포함 전송.
}

void disp_player(int x, int y,  char *name)
{
	gotoxy(x,y);
	printf(name);
}
void erase_player(int x, int y, char *name)
{
	char buf[BUF_LEN];
	memset(buf, ' ' , strlen(name));
	buf[strlen(name)] = '\0';
	gotoxy(x,y);
	printf(buf);
}

int oldx1, oldy1, newx1, newy1;
char name1[BUF_LEN];
void player1_init(char *name, int oldx, int oldy, int newx, int newy)
{
	oldx1 = oldx;
	oldy1 = oldy;
	newx1 = newx;
	newy1 = newy;
	strcpy(name1,name);
}
void player1(unsigned char ch)
{
	//static int oldx=50,oldy=10, newx=50, newy=10;
	int move_flag = 0;
	static int called = 0;
	static unsigned char last_ch=0;

	if (called == 0) { // 최초 위치
		removeCursor();
		disp_player(oldx1,oldy1,name1);
		called = 1;
	}
	if (keep_moving && ch==0)
		ch = last_ch;
	last_ch = ch;

	switch(ch) {
	case UP:
		if (oldy1>1)
			newy1 = oldy1 - 1;
		else { // 벽에 부딛치면 방향을 반대로 이동
			newy1 = oldy1 + 1;
			last_ch = DOWN;
		}
		move_flag = 1;
		break;
	case DOWN:
		if (oldy1 < HEIGHT - 2)
			newy1 = oldy1 + 1;
		else {
			newy1 = oldy1 - 1;
			last_ch = UP;
		}
		move_flag = 1;
		break;
	case LEFT:
		if (oldx1 > 0 )
			newx1 = oldx1 - 1;
		else {
			newx1 = oldx1 + 1;
			last_ch = RIGHT;
		}
		move_flag = 1;
		break;
	case RIGHT:
		if (oldx1 < WIDTH - 1)
			newx1 = oldx1 + 1;
		else {
			newx1 = oldx1 - 1;
			last_ch = LEFT;
		}
		move_flag = 1;
		break;
	}
	if (move_flag) {
		erase_player(oldx1, oldy1, name1); // 마지막 위치의 O 를 지우고
		disp_player(newx1, newy1, name1); // 새로운 위치에서 O 를 표시한다.
		oldx1 = newx1; // 마지막 위치를 기억한다.
		oldy1 = newy1;
	}

}
int oldx2, oldy2, newx2, newy2;
char name2[128];
void player2_init(char *name, int oldx, int oldy, int newx, int newy)
{
	oldx2 = oldx;
	oldy2 = oldy;
	newx2 = newx;
	newy2 = newy;
	strcpy(name2, name);
}
void player2(unsigned char ch)
{
	//static int oldx=20,oldy=10, newx=20, newy=10;
	int move_flag = 0;
	static int called = 0;
	static char last_ch=0;

	if (called == 0) { // 최초 위치 
		disp_player(oldx2,oldy2,name2);
		called = 1;
	}
	// 방향키가 눌려지지 않은 경우 예전에 이동하던 방향으로 계속 이동
	if (keep_moving && ch==0) 
		ch = last_ch;
	last_ch = ch;

	switch(ch) {
	case UP2: // W
		if (oldy2>1)
			newy2 = oldy2 - 1;
		else { // 벽에 부딛치면 방향을 반대로 이동
			newy2 = oldy2 + 1;
			last_ch = DOWN2;
		}
		move_flag = 1;
		break;
	case DOWN2: //S
		if (oldy2 < HEIGHT - 2)
			newy2 = oldy2 + 1;
		else {
			newy2 = oldy2 - 1;
			last_ch = UP2;
		}
		move_flag = 1;
		break;
	case LEFT2://A
		if (oldx2 > 0 )
			newx2 = oldx2 - 1;
		else {
			newx2 = oldx2 + 1;
			last_ch = RIGHT2;
		}
		move_flag = 1;
		break;
	case RIGHT2://D
		if (oldx2 < WIDTH - 1)
			newx2 = oldx2 + 1;
		else {
			newx2 = oldx2 - 1;
			last_ch = LEFT2;
		}
		move_flag = 1;
		break;
	}
	if (move_flag) {
		erase_player(oldx2, oldy2, name2); // 마지막 위치의 X 를 지우고
		disp_player(newx2, newy2, name2); // 새로운 위치에서 X 를 표시한다.
		oldx2 = newx2; // 마지막 위치를 기억한다.
		oldy2 = newy2;
		move_flag = 1; // 1:계속 이동, 0:한칸씩 이동
	}
}

void main(int argc, char *argv[])
{
	// 특수키 0xe0 을 입력받으려면 unsigned char 로 선언해야 함
	unsigned char ch;
	int sd; // socket
	char buf[BUFSIZE];
	int nread; // network 에서 읽은 data 수
	char *p;
	int len;
	int oldx, oldy, newx, newy;
	int move_flag = 0;
	clock_t start, now, time_slot;
	int p2_connected = 0; // 상대편이 연결되었는지 flag
	char *ip_addr = "127.0.0.1"; // local server 사용시.
	int port_no = 30000;

	 /* 게임 참가자 이름 */
	printf("Enter user name : ");
	scanf("%s", myname);

	// game server에 접속
	sd = connect_server(ip_addr, port_no);
	//printf("Player %s connected", name);

	
	cls(BLACK, WHITE);
	srand(time(NULL));
	removeCursor();
	textcolor(BLACK, WHITE);

	oldx = newx = rand() % (WIDTH-10);
	oldy = newy = rand() % (HEIGHT-10)+1;
	showname(newx, newy, myname);
	player1_init(myname, oldx, oldy, oldx, oldy);
	sprintf(buf, "%s %s %d %d %d %d", CONNECT, myname, oldx, oldy, newx, newy);
	send(sd, buf, BUF_LEN, 0); // send my position (NULL 도 보냄)

	nread = 0;
	move_flag = 0; // 계속 움직이게 하려면 move_flag = 1로 두면된다.
	ch = '\0'; // 초기값은 움직이지 않는다.
	while (1) {
		nread = recv(sd, buf, BUF_LEN, 0);// 고정 길이 read
		if (nread == SOCKET_ERROR) {
			//gotoxy(0,0);
			//printf("error=%d", WSAGetLastError());
			if (WSAGetLastError()!=WSAEWOULDBLOCK)
				break;
		}		//gotoxy(0,0); printf("nread = %d %s\n", nread, buf);
		if (nread>0) { // network에서 event 가 온 경우.
			char cmd[BUF_LEN];
			char name[BUF_LEN];
			int oldx, oldy, newx, newy;
			sscanf(buf, "%s %s %d %d %d %d", cmd, name, &oldx, &oldy, &newx, &newy);
			if (strcmp(name, name1)!=0) {// 다른 Player가 보낸 데이타면
				// 다른 player의 위치를 이동한다.
				//movename(name2, oldx2, oldy2, newx2, newy2);
				if (strcmp(cmd, CONNECT)==0) {// 먼저 들어왔는데 새로운 player 가 들어왔다면.
					player2_init(name, oldx, oldy, newx, newy);
					sendmypos(sd, myname, oldx1, oldy1, newx1, newy1); // 내 위치를 알려준다.
					p2_connected = 1;
				} else if (strcmp(cmd, MOVE)==0) { // 나중에 들어왔는데, p2의 위치가 날라오면
					player2_init(name, oldx, oldy, newx, newy);
					p2_connected = 1;
				} else if (strcmp(cmd, MOVEUP)==0)
					ch = UP2;
				else if (strcmp(cmd, MOVEDOWN)==0)
					ch = DOWN2;
				else if (strcmp(cmd, MOVELEFT)==0)
					ch = LEFT2;
				else if (strcmp(cmd, MOVERIGHT)==0)
					ch = RIGHT2;
				else if (strcmp(cmd, MOVENONE)==0)
					ch = NONE;
			} else { // 내 Data 가 다시 도착하는 경우.
				if (strcmp(cmd, MOVEUP)==0)
					ch = UP;
				else if (strcmp(cmd, MOVEDOWN)==0)
					ch = DOWN;
				else if (strcmp(cmd, MOVELEFT)==0)
					ch = LEFT;
				else if (strcmp(cmd, MOVERIGHT)==0)
					ch = RIGHT;
				else if (strcmp(cmd, MOVENONE)==0)
					ch = NONE;
			}

		}
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
				if (p2_connected!=1) // p2 가 연결되어야 나도 움직일 수 있다.
					ch = '\0';

				switch(ch) {
				case UP: 
					sendmymove(sd, MOVEUP, name1, oldx1, oldy1, newx1, newy1);
					break;
				case DOWN:
					sendmymove(sd, MOVEDOWN, name1, oldx1, oldy1, newx1, newy1);
					break;
				case LEFT:
					sendmymove(sd, MOVELEFT, name1, oldx1, oldy1, newx1, newy1);
					break;
				case RIGHT:
					sendmymove(sd, MOVERIGHT, name1, oldx1, oldy1, newx1, newy1);
					break;
				default:
					//sendmymove(sd, MOVENONE, name1, oldx1, oldy1, newx1, newy1);
					break;
				}

			}
		}
		if (nread>0) { // 누군가의 Move 가 있다면.
			switch(ch) {
			case UP:
			case DOWN:
			case LEFT:
			case RIGHT:
				player1(ch);//player1만 방향 전환
				if (p2_connected)
					player2(0);
				break;
			default: // 방향 전환이 아니면
				player1(0);
				if (p2_connected)
					player2(0);
			}
			// Player2은 AWSD 로 움직인다.
			switch(ch) {
			case UP2:
			case DOWN2:
			case LEFT2:
			case RIGHT2://player2만 방향 전환
				if (p2_connected)
					player2(ch);
				player1(0);
				break;
			default:// 방향 전환이 아니면
				player1(0);
				if (p2_connected)
					player2(0);
			}
		} else { 
			// keyboard 가 눌려지지 않으면 계속 움직인다.
			// 이동중이던 방향으로 계속 이동
			player1(0);
			if (p2_connected)
				player2(0);
		}
		Sleep(Delay);

	} // while 
}