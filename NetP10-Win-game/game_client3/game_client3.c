//
// game_client3.c 
// 1:1 ���� ��忡�� ���� ����Ű�� �����̸� key�� ������ �ʾƵ�
// ��� �����̸�, ������� ���� �������� ǥ�õȴ�.
// UP, DOWN, LEFT, RIGHT Ű �Է� ��Ȳ�� �����Ѵ�.
// 
//
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

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

#define NONE	'X' // �������� �ʴ´�.

#define WIDTH 80
#define HEIGHT 24

int Delay = 50; // 50 msec delay, �� ���� ���̸� �ӵ��� ��������.
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
#include <winsock.h>
#include <signal.h>
#include <stdio.h>
#include <conio.h>

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
#define MAXREAD	((BUFSIZE/MSG_LENGTH)*MSG_LENGTH) // MSG_LENGTH �� align�ؼ� �д´�.

void sendmypos(int sd, char *name, int oldx, int oldy, int newx, int newy)
{
	char buf[BUF_LEN];
	sprintf(buf, "%s %s %d %d %d %d", MOVE, name, oldx, oldy, newx, newy);
	send(sd, buf, BUF_LEN, 0);  // NULL ���� ����.
}

void sendmymove(int sd, char *cmd, char *name,  int oldx, int oldy, int newx, int newy)
{
	char buf[BUF_LEN];
	sprintf(buf, "%s %s %d %d %d %d", cmd, name, oldx, oldy, newx, newy);
	send(sd, buf, BUF_LEN, 0);  // NULL ���� ����.
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

	if (called == 0) { // ���� ��ġ
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
		else { // ���� �ε�ġ�� ������ �ݴ�� �̵�
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
		erase_player(oldx1, oldy1, name1); // ������ ��ġ�� O �� �����
		disp_player(newx1, newy1, name1); // ���ο� ��ġ���� O �� ǥ���Ѵ�.
		oldx1 = newx1; // ������ ��ġ�� ����Ѵ�.
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

	if (called == 0) { // ���� ��ġ 
		disp_player(oldx2,oldy2,name2);
		called = 1;
	}
	// ����Ű�� �������� ���� ��� ������ �̵��ϴ� �������� ��� �̵�
	if (keep_moving && ch==0) 
		ch = last_ch;
	last_ch = ch;

	switch(ch) {
	case UP2: // W
		if (oldy2>1)
			newy2 = oldy2 - 1;
		else { // ���� �ε�ġ�� ������ �ݴ�� �̵�
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
		erase_player(oldx2, oldy2, name2); // ������ ��ġ�� X �� �����
		disp_player(newx2, newy2, name2); // ���ο� ��ġ���� X �� ǥ���Ѵ�.
		oldx2 = newx2; // ������ ��ġ�� ����Ѵ�.
		oldy2 = newy2;
		move_flag = 1; // 1:��� �̵�, 0:��ĭ�� �̵�
	}
}

void main(int argc, char *argv[])
{
	// Ư��Ű 0xe0 �� �Է¹������� unsigned char �� �����ؾ� ��
	unsigned char ch;
	int sd; // socket
	char buf[BUFSIZE];
	int nread; // network ���� ���� data ��
	char *p;
	int len;
	int oldx, oldy, newx, newy;
	int move_flag = 0;
	clock_t start, now, time_slot;
	int p2_connected = 0; // ������� ����Ǿ����� flag
	char *ip_addr = "127.0.0.1"; // local server ����.
	int port_no = 30000;

	 /* ���� ������ �̸� */
	printf("Enter user name : ");
	scanf("%s", myname);

	// game server�� ����
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
	send(sd, buf, BUF_LEN, 0); // send my position (NULL �� ����)

	nread = 0;
	move_flag = 0; // ��� �����̰� �Ϸ��� move_flag = 1�� �θ�ȴ�.
	ch = '\0'; // �ʱⰪ�� �������� �ʴ´�.
	while (1) {
		nread = recv(sd, buf, BUF_LEN, 0);// ���� ���� read
		if (nread == SOCKET_ERROR) {
			//gotoxy(0,0);
			//printf("error=%d", WSAGetLastError());
			if (WSAGetLastError()!=WSAEWOULDBLOCK)
				break;
		}		//gotoxy(0,0); printf("nread = %d %s\n", nread, buf);
		if (nread>0) { // network���� event �� �� ���.
			char cmd[BUF_LEN];
			char name[BUF_LEN];
			int oldx, oldy, newx, newy;
			sscanf(buf, "%s %s %d %d %d %d", cmd, name, &oldx, &oldy, &newx, &newy);
			if (strcmp(name, name1)!=0) {// �ٸ� Player�� ���� ����Ÿ��
				// �ٸ� player�� ��ġ�� �̵��Ѵ�.
				//movename(name2, oldx2, oldy2, newx2, newy2);
				if (strcmp(cmd, CONNECT)==0) {// ���� ���Դµ� ���ο� player �� ���Դٸ�.
					player2_init(name, oldx, oldy, newx, newy);
					sendmypos(sd, myname, oldx1, oldy1, newx1, newy1); // �� ��ġ�� �˷��ش�.
					p2_connected = 1;
				} else if (strcmp(cmd, MOVE)==0) { // ���߿� ���Դµ�, p2�� ��ġ�� �������
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
			} else { // �� Data �� �ٽ� �����ϴ� ���.
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
		if (kbhit()==1) {  // Ű���尡 ������ ������
			ch = getch(); // key ���� �д´�
			// ESC ������ ���α׷� ����
			if (ch == ESC) {
				//gotoxy(25,11);
				//exit(0);
				break;
			}
			if (ch==SPECIAL1 || ch==SPECIAL2) { // ���� Ư��Ű
				// ���� ��� UP key�� ��� 0xe0 0x48 �ΰ��� ���ڰ� ���´�.
				ch = getch();
				// Player1�� ����Ű�� �����δ�.
				if (p2_connected!=1) // p2 �� ����Ǿ�� ���� ������ �� �ִ�.
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
		if (nread>0) { // �������� Move �� �ִٸ�.
			switch(ch) {
			case UP:
			case DOWN:
			case LEFT:
			case RIGHT:
				player1(ch);//player1�� ���� ��ȯ
				if (p2_connected)
					player2(0);
				break;
			default: // ���� ��ȯ�� �ƴϸ�
				player1(0);
				if (p2_connected)
					player2(0);
			}
			// Player2�� AWSD �� �����δ�.
			switch(ch) {
			case UP2:
			case DOWN2:
			case LEFT2:
			case RIGHT2://player2�� ���� ��ȯ
				if (p2_connected)
					player2(ch);
				player1(0);
				break;
			default:// ���� ��ȯ�� �ƴϸ�
				player1(0);
				if (p2_connected)
					player2(0);
			}
		} else { 
			// keyboard �� �������� ������ ��� �����δ�.
			// �̵����̴� �������� ��� �̵�
			player1(0);
			if (p2_connected)
				player2(0);
		}
		Sleep(Delay);

	} // while 
}