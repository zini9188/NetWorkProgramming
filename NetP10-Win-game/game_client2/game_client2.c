// game_client2.c 
// chat server 에 접속한 nickname을 화면에 방향키로 움직이면서 표시한다.
// version up : mouse를 사용해본다. (gotoxy() 함수에서 mouse 이용)
// 주의 : console 창의 속성 편집 옵션에서 [빠른 편집 모드] 를 제거해야 됨.
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <Windows.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <winsock.h>
#include <signal.h>
#include <conio.h>

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

#define WIDTH 80
#define HEIGHT 24

int Delay = 100; // 100 msec delay, 이 값을 줄이면 속도가 빨라진다.
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

#define BUF_LEN	128
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
	//fflush(stdout);
}
#define CONNECT "CONN"
#define MOVE	"MOVE"

void sendmypos(int sd, char *name, int oldx, int oldy, int newx, int newy)
{
	char buf[BUF_LEN];
	sprintf(buf, "%s %s %02d %02d %02d %02d", MOVE, name, oldx, oldy, newx, newy);
	send(sd, buf, BUF_LEN, 0);  // NULL 포함 전송.
}

// initialize screen
HANDLE       hIn, hOut;
void init_scr()
{
	hIn=GetStdHandle(STD_INPUT_HANDLE); 
	hOut=GetStdHandle(STD_OUTPUT_HANDLE); 
	SetConsoleMode(hIn, ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT); 
}

void main(int argc, char *argv[])
{
	// 특수키 0xe0 을 입력받으려면 unsigned char 로 선언해야 함
	unsigned char ch;
	int sd; // socket
	char buf[BUF_LEN];
	int nread; // network 에서 읽은 data 수
	char *p;
	int len;
	int oldx, oldy, newx, newy;
	int move_flag = 0;
	char *ip_addr = "127.0.0.1"; // local server 사용시.
	int port_no = 30000;

	DWORD        dwNOER; 
	INPUT_RECORD rec; // 마우스 입력을 위한..

	/* 게임 참가자 이름 */
	printf("Enter user name : ");
	scanf("%s", myname);

	// game server에 접속
	sd = connect_server(ip_addr, port_no);

	cls(BLACK, WHITE);
	init_scr();
	srand(time(NULL));
	removeCursor();
	textcolor(BLACK, WHITE);

	oldx = newx = rand() % (WIDTH-10);
	oldy = newy = rand() % (HEIGHT-10)+1;
	showname(newx, newy, myname);
	sprintf(buf, "%s %s %02d %02d %02d %02d", CONNECT, myname, oldx, oldy, newx, newy);
	send(sd, buf, BUF_LEN, 0); // send my position (NULL 도 보냄)

	nread = 0;
	while (1) {
		int nevents;
		//
		// Network 의 다른 Player의 이동을 먼저 처리한다.
		//
		nread = recv(sd, buf, BUF_LEN, 0);// 고정 길이 read
		if (nread == SOCKET_ERROR) {
			//gotoxy(0,0);
			//printf("error=%d", WSAGetLastError());
			if (WSAGetLastError()!=WSAEWOULDBLOCK)
				break;
		}
		if (nread > 0) { // network 에서 들어온 data 가 있으면
			char cmd[120];
			char name2[512];
			int oldx2, oldy2, newx2, newy2;
			int len;
			sscanf(buf, "%s %s %d %d %d %d", cmd, name2, &oldx2, &oldy2, &newx2, &newy2);
			if (strcmp(name2, myname)!=0) {// 다른 Player가 보낸 데이타면
				// 다른 player의 위치를 이동한다.
				movename(name2, oldx2, oldy2, newx2, newy2);
				if (strcmp(cmd, CONNECT)==0) // 새로운 player 가 들어왔다면.
					sendmypos(sd, myname, newx, newy, newx, newy); // 내 위치를 알려준다.
			}
		}
		
		move_flag = 0;
		// mouse / keyboard 입력 처리
		GetNumberOfConsoleInputEvents(hIn, (LPDWORD)&nevents);
		if (nevents==0) // Event 가 없으면 읽으러 들어갈 필요가 없다.
			continue;
		ReadConsoleInput(hIn,&rec,1,&dwNOER);
		//gotoxy(0, 0); printf("type=%x", rec.EventType);
		if (rec.EventType == MOUSE_EVENT) 
		{ 
			if (rec.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) 
			{ 
				//gotoxy(0,1); 
				//printf("X:%2d, Y:%2d",rec.Event.MouseEvent.dwMousePosition.X,rec.Event.MouseEvent.dwMousePosition.Y); 
				newx = rec.Event.MouseEvent.dwMousePosition.X;
				newy = rec.Event.MouseEvent.dwMousePosition.Y;
				move_flag = 1;
			} 
		}
		//gotoxy(0, 1); printf("type=%x", rec.EventType);
		if (rec.EventType == KEY_EVENT && rec.Event.KeyEvent.bKeyDown == TRUE) { 
			// 내가 방향키 누르면.
			//gotoxy(0, 2); printf("type=%x", rec.EventType);
			switch(rec.Event.KeyEvent.wVirtualKeyCode) {
				case VK_UP:
					if (oldy>1) // 0 은 Status Line
						newy = oldy - 1;
					else { // 벽에 부딛치면 움직이지 못한다.
						//newy = oldy + 1; // 벽에 부딛치면 방향을 반대로 이동할 때 필요
					}
					move_flag = 1;
					break;
				case VK_DOWN:
					if (oldy < HEIGHT - 1)
						newy = oldy + 1;
					else {
						//newy = oldy - 1;
					}
					move_flag = 1;
					break;
				case VK_LEFT:
					if (oldx > 0 )
						newx = oldx - 1;
					else {
						//newx = oldx + 1;
					}
					move_flag = 1;
					break;
				case VK_RIGHT:
					if (oldx < WIDTH - 1)
						newx = oldx + 1;
					else {
						//newx = oldx - 1;
					}
					move_flag = 1;
					break;
			}
		}
		if (move_flag) {
				movename(myname, oldx, oldy, newx, newy); // 내이름 이동.
				sendmypos(sd, myname, oldx, oldy, newx, newy); // 내 위치 알려준다.
				oldx = newx; // 마지막 위치를 기억한다.
				oldy = newy;
		}
	}
}

