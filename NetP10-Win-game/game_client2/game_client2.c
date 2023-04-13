// game_client2.c 
// chat server �� ������ nickname�� ȭ�鿡 ����Ű�� �����̸鼭 ǥ���Ѵ�.
// version up : mouse�� ����غ���. (gotoxy() �Լ����� mouse �̿�)
// ���� : console â�� �Ӽ� ���� �ɼǿ��� [���� ���� ���] �� �����ؾ� ��.
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <Windows.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <winsock.h>
#include <signal.h>
#include <conio.h>

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

#define STAR1 'O' // player1 ǥ��
#define STAR2 'X' // player2 ǥ��
#define BLANK '.' // ' ' ���ϸ� ������ �������� 

#define ESC 0x1b //  ESC ������ ����

#define SPECIAL1 0xe0 // Ư��Ű�� 0xe0 + key ������ �����ȴ�.
#define SPECIAL2 0x00 // keypad ��� 0x00 + key �� �����ȴ�.

#define UP  0x48 // Up key�� 0xe0 + 0x48 �ΰ��� ���� ���´�.
#define DOWN 0x50
#define LEFT 0x4b
#define RIGHT 0x4d

#define UP2		'w'//player2 �� AWSD �� ����Ű ���
#define DOWN2	's'
#define LEFT2	'a'
#define RIGHT2	'd'

#define WIDTH 80
#define HEIGHT 24

int Delay = 100; // 100 msec delay, �� ���� ���̸� �ӵ��� ��������.
int keep_moving = 1; // 1:����̵�, 0:��ĭ���̵�.
int time_out = 30; // ���ѽð�
int score[2] = {0};
int golds[WIDTH][HEIGHT] = {0}; // 1�̸� Gold �ִٴ� ��
int goldinterval = 3; // GOLD ǥ�� ����
int called[2];

#define STARTX	40
#define STARTY	10

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
		putchar(' '); // blank �� �����.
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

#define BUF_LEN	128
char *escapechar = "exit";
char myname[256];  					/* ä�ÿ��� ����� �̸� */
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
	int s;  /* ������ ����� ���Ϲ�ȣ */
	WORD sversion;
	u_long iMode = 1;

	// winsock ����� ���� �ʼ�����
	signal(SIGINT, exit_callback);
	sversion = MAKEWORD(1, 1);
	WSAStartup(sversion, &wsadata);


	/* ���� ���� */
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Client : Can't open stream socket.\n");
		exit(0);
	}

	/* ä�� ������ �����ּ� ����ü server_addr �ʱ�ȭ */
	memset((char *)&server_addr, '\0', sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip_addr);
	server_addr.sin_port = htons(port_no);

	/* �����û */
	if (connect(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		printf("Client : Can't connect to server.\n");
		exit(0);
	}
	else {
		gotoxy(0, 0);
		printf("%s ������ ���ӵǾ����ϴ�.", myname);
	}

	ioctlsocket(s, FIONBIO, &iMode); // read �� non-blocking ���� �����.
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
	send(sd, buf, BUF_LEN, 0);  // NULL ���� ����.
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
	// Ư��Ű 0xe0 �� �Է¹������� unsigned char �� �����ؾ� ��
	unsigned char ch;
	int sd; // socket
	char buf[BUF_LEN];
	int nread; // network ���� ���� data ��
	char *p;
	int len;
	int oldx, oldy, newx, newy;
	int move_flag = 0;
	char *ip_addr = "127.0.0.1"; // local server ����.
	int port_no = 30000;

	DWORD        dwNOER; 
	INPUT_RECORD rec; // ���콺 �Է��� ����..

	/* ���� ������ �̸� */
	printf("Enter user name : ");
	scanf("%s", myname);

	// game server�� ����
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
	send(sd, buf, BUF_LEN, 0); // send my position (NULL �� ����)

	nread = 0;
	while (1) {
		int nevents;
		//
		// Network �� �ٸ� Player�� �̵��� ���� ó���Ѵ�.
		//
		nread = recv(sd, buf, BUF_LEN, 0);// ���� ���� read
		if (nread == SOCKET_ERROR) {
			//gotoxy(0,0);
			//printf("error=%d", WSAGetLastError());
			if (WSAGetLastError()!=WSAEWOULDBLOCK)
				break;
		}
		if (nread > 0) { // network ���� ���� data �� ������
			char cmd[120];
			char name2[512];
			int oldx2, oldy2, newx2, newy2;
			int len;
			sscanf(buf, "%s %s %d %d %d %d", cmd, name2, &oldx2, &oldy2, &newx2, &newy2);
			if (strcmp(name2, myname)!=0) {// �ٸ� Player�� ���� ����Ÿ��
				// �ٸ� player�� ��ġ�� �̵��Ѵ�.
				movename(name2, oldx2, oldy2, newx2, newy2);
				if (strcmp(cmd, CONNECT)==0) // ���ο� player �� ���Դٸ�.
					sendmypos(sd, myname, newx, newy, newx, newy); // �� ��ġ�� �˷��ش�.
			}
		}
		
		move_flag = 0;
		// mouse / keyboard �Է� ó��
		GetNumberOfConsoleInputEvents(hIn, (LPDWORD)&nevents);
		if (nevents==0) // Event �� ������ ������ �� �ʿ䰡 ����.
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
			// ���� ����Ű ������.
			//gotoxy(0, 2); printf("type=%x", rec.EventType);
			switch(rec.Event.KeyEvent.wVirtualKeyCode) {
				case VK_UP:
					if (oldy>1) // 0 �� Status Line
						newy = oldy - 1;
					else { // ���� �ε�ġ�� �������� ���Ѵ�.
						//newy = oldy + 1; // ���� �ε�ġ�� ������ �ݴ�� �̵��� �� �ʿ�
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
				movename(myname, oldx, oldy, newx, newy); // ���̸� �̵�.
				sendmypos(sd, myname, oldx, oldy, newx, newy); // �� ��ġ �˷��ش�.
				oldx = newx; // ������ ��ġ�� ����Ѵ�.
				oldy = newy;
		}
	}
}

