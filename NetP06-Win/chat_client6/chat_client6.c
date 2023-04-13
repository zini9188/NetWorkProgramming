/*
파일명 : chat_client6.c
기  능 : 채팅 클라이언트, chat_client4 + 1:1 대화 모드 + 파일 전송
사용법 : chat_client6 [host] [port]
네트워크와 키보드 동시 처리 방법
Linux : select() 사용
Windows : socket()을 Non-blocking mode 와 kbhit()을 이용하여 폴링 구조 사용
*/
#ifdef WIN32
#define  _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)
#pragma warning(disable:4267)
#pragma warning(disable:4244)

#include <winsock.h>
#include <signal.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#else
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#endif



#ifdef WIN32
WSADATA wsadata;
int	main_socket;

void exit_callback(int sig)
{
	closesocket(main_socket);
	WSACleanup();
	exit(0);
}

void init_winsock()
{
	WORD sversion;
	u_long iMode = 1;

	// winsock 사용을 위해 필수적임
	signal(SIGINT, exit_callback);
	sversion = MAKEWORD(1, 1);
	WSAStartup(sversion, &wsadata);
}
#endif

#define CHAT_SERVER "127.0.0.1"
#define CHAT_PORT "30000"
#define BUF_LEN 128

// chat_client/server3
#define CHAT_CMD_LOGIN		"/login"// connect하면 user name 전송 "/login user1"
#define CHAT_CMD_LIST		"/list"// userlist 요청
#define CHAT_CMD_EXIT		"/exit"// 종료
// chat_client/server4 
#define CHAT_CMD_TO			"/to"// 귓속말 "/to user2 Hi there.."
#define CHAT_CMD_SLEEP		"/sleep"// 대기모드(부재중) 설정
#define CHAT_CMD_WAKEUP		"/wakeup"// wakeup 또는 message 전송하면 자동 wakeup
// 도전 문제를 위한 명령어들
// 도전1 : 1:1 대화
#define CHAT_CMD_WITH		"/with"// [user1] /with user2, user1이 user2와 1:1 대화 요청
#define CHAT_CMD_WITH_YES	"/withyes"// 1:1 대화 허락 [user2] /withyes user1
#define CHAT_CMD_WITH_NO	"/withno"// 1:1 대화 거부 [user2] /withno user1
#define CHAT_CMD_WITH_END	"/end"// 1:1 채팅 종료 [user1] /end or [user2] /end
// 도전2 : 1:1 파일 전송
#define CHAT_CMD_FILESEND	"/filesend"// [user1] /filesend user2 data.txt 파일 전송 요청
#define CHAT_CMD_FILE_YES	"/fileyes"// [user2] /fileyes user1 파일 전송 허락
#define CHAT_CMD_FILE_NO	"/fileno"// [user2] /fileno user1 파일 수신 거부
#define CHAT_CMD_FILE_NAME	"/filename"// [user1] /filename data.txt 765 파일 정보 전달
#define CHAT_CMD_FILE_DATA	"/filedata"// [user1] /filedata data...
#define CHAT_CMD_FILE_END	"/fileend"// [user1] /fileend 파일전송 끝

char username[BUF_LEN]; // user name
void read_key_send(int s, char* buf, char* buf2); // key입력후 보내는 code (Linux/Windows공용)

int main(int argc, char* argv[]) {
	char buf1[BUF_LEN + 1], buf2[BUF_LEN + 1];
	int s, n;
	struct sockaddr_in server_addr;
	char* ip_addr = CHAT_SERVER, * port_no = CHAT_PORT;
	struct timeval tm;
	tm.tv_sec = 0;
	tm.tv_usec = 1000;

	if (argc == 3) {
		ip_addr = argv[1];
		port_no = argv[2];
	}
	printf("chat_client4 running.\n");
	printf("Enter user name : ");
	scanf("%s", username); getchar(); // \n제거

#ifdef WIN32
	printf("Windows : ");
	init_winsock();
#else // Linux
	printf("Linux : ");
#endif 

	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("can't create socket\n");
		exit(0);
	}
#ifdef WIN32
	main_socket = s;
#endif 

	/* 채팅 서버의 소켓주소 구조체 server_addr 초기화 */
	memset((char*)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip_addr);
	server_addr.sin_port = htons(atoi(port_no));

	/* 연결요청 */
	printf("Connecting %s %s\n", ip_addr, port_no);

	/* 연결요청 */
	if (connect(s, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		printf("Client : Can't connect to server.\n");
		exit(0);
	}
	else {
		printf("채팅 서버에 접속되었습니다. \n");
	}
	memset(buf1, 0, BUF_LEN);
	sprintf(buf1, "%s %s", CHAT_CMD_LOGIN, username);
	if (send(s, buf1, BUF_LEN, 0) < 0) {
		printf("username send error\n");
		exit(0);
	}
#ifdef WIN32
	u_long iMode = 1;
	ioctlsocket(s, FIONBIO, &iMode); // 소켓을 non-blocking 으로 만든다.

	while (1) {
		memset(buf1, 0, BUF_LEN);
		memset(buf2, 0, BUF_LEN);
		// Non-blocking read이므로 데이터가 앖으면 기다리지 않고 0으로 return
		n = recv(s, buf2, BUF_LEN, 0);
		if (n > 0) { // non-blocking read
		// network에서 읽어서
		// 화면에 출력
			printf("%s", buf2);
		}
		else if (WSAGetLastError() != WSAEWOULDBLOCK) {
			printf("recv error\n"); // server 가 종료되었거나 네트워크 오류
			break;
		}
		if (kbhit()) { // key 가 눌려있으면 read key --> write to chat server
			read_key_send(s, buf1, buf2);
		}
		Sleep(100); // Non-blocking I/O CPU 부담을 줄인다.
	}
#else
	int maxfdp;
	fd_set read_fds;
	maxfdp = s + 1; // socket은 항상 0 보다 크게 할당된다.
	FD_ZERO(&read_fds);
	while (1) {
		memset(buf1, 0, BUF_LEN);
		memset(buf2, 0, BUF_LEN);

		FD_SET(0, &read_fds); // stdin:0 표준입력은 file 번호 = 0 이다.
		FD_SET(s, &read_fds); // server 와 견결된 socket 번호

		if (select(maxfdp, &read_fds, (fd_set*)0, (fd_set*)0, (struct timeval*)0) < 0) {
			printf("select error\n");
			exit(0);
		}
		// network I/O 변화 있는 경우
		if (FD_ISSET(s, &read_fds)) {
			memset(buf2, BUF_LEN, 0);
			if ((n = recv(s, buf2, BUF_LEN, 0)) > 0) {
				printf("%s", buf2);
			}
			else {
				printf("recv error\n");
				break;
			}
		}
		// keyboard 입력이 있는 경우
		if (FD_ISSET(0, &read_fds)) {
			read_key_send(s, buf1, buf2);
		}
	}
#endif
}

// Keyboard에서 읽어서 서버로 전송하는 함수 (Linux/Windows 공용)
void read_key_send(int s, char* buf1, char* buf2)
{
#ifdef WIN32
	printf("%s> ", username);
#endif	
	memset(buf1, 0, BUF_LEN);
	memset(buf2, 0, BUF_LEN);

	if (fgets(buf1, BUF_LEN, stdin) > 0) {
		sprintf(buf2, "[%s] %s", username, buf1);
		if (send(s, buf2, BUF_LEN, 0) < 0) {
			printf("send error.\n");
			exit(0);
		}
		if (strncmp(buf1, CHAT_CMD_EXIT, strlen(CHAT_CMD_EXIT)) == 0) {
			printf("Good bye.\n");
#ifdef WIN32
			closesocket(s);
#else
			close(s);
#endif
			exit(0);
		}
	}
	else {
		printf("fgets error\n");
		exit(0);
	}
}