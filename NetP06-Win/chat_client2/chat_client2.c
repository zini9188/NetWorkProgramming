/*
���ϸ� : chat_client2.c
��  �� : ä�� Ŭ���̾�Ʈ, username ó��
���� : chat_client2 [host] [port]
��Ʈ��ũ�� Ű���� ���� ó�� ���
Linux : select() ���
Windows : socket()�� Non-blocking mode �� kbhit()�� �̿��Ͽ� ���� ���� ���
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

	// winsock ����� ���� �ʼ�����
	signal(SIGINT, exit_callback);
	sversion = MAKEWORD(1, 1);
	WSAStartup(sversion, &wsadata);
}
#endif

#define CHAT_SERVER "127.0.0.1"
#define CHAT_PORT "30000"
#define BUF_LEN 128
#define EXIT	"exit"
char username[BUF_LEN];
// user name

int main(int argc, char* argv[]) {
	unsigned char buf1[BUF_LEN + 1], buf2[BUF_LEN + 1];
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

	printf("chat_client2 running.\n");
	// printf("Enter user name : ");
	// username �Է�
	printf("Enter user name : ");
	scanf("%s", username); getchar();


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

	/* ä�� ������ �����ּ� ����ü server_addr �ʱ�ȭ */
	memset((char*)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip_addr);
	server_addr.sin_port = htons(atoi(port_no));

	/* �����û */
	printf("chat_client2 connecting %s %s\n", ip_addr, port_no);

	/* �����û */
	if (connect(s, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		printf("Client : Can't connect to server.\n");
		exit(0);
	}
	else {
		printf("ä�� ������ ���ӵǾ����ϴ�. \n");
	}
#ifdef WIN32
	u_long iMode = 1;
	ioctlsocket(s, FIONBIO, &iMode); // ������ non-blocking ���� �����.

	while (1) {
		// Non-blocking read�̹Ƿ� �����Ͱ� ������ ��ٸ��� �ʰ� 0���� return
		memset(buf1, 0, BUF_LEN);
		memset(buf2, 0, BUF_LEN);
		n = recv(s, buf2, BUF_LEN, 0);
		if (n > 0) { // non-blocking read
			// network���� �о
			// ȭ�鿡 ���
			printf("%s", buf2);
		}
		else if (WSAGetLastError() != WSAEWOULDBLOCK) {
			printf("recv error\n"); // server �� ����Ǿ��ų� ��Ʈ��ũ ����
			break;
		}
		if (kbhit()) { // key �� ���������� read key --> write to chat server
			if (fgets(buf1, BUF_LEN, stdin)) { // Enter key ���� �Է� �ް� ����
				// sprintf() �̿� buf2 �� "[hansung] Hello there" ������ ����� buf2�� ������.
				sprintf(buf2, "[%s] %s", username, buf1);
				if (send(s, buf2, BUF_LEN, 0) < 0) {
					printf("send error.\n");
					break;
				}
				if (strncmp(buf1, EXIT, strlen(EXIT)) == 0) {
					printf("Good bye.\n");
					break;
				}
			}
			else {
				printf("fgets error\n");
				break;
			}
		}
		Sleep(100); // Non-blocking I/O CPU �δ��� ���δ�.
	}
#else
	int maxfdp;
	fd_set read_fds;
	maxfdp = s + 1; // socket�� �׻� 0 ���� ũ�� �Ҵ�ȴ�.
	FD_ZERO(&read_fds);
	while (1) {
		memset(buf1, 0, BUF_LEN);
		memset(buf2, 0, BUF_LEN);

		FD_SET(0, &read_fds); // stdin:0 ǥ���Է��� file ��ȣ = 0 �̴�.
		FD_SET(s, &read_fds); // server �� �߰�� socket ��ȣ

		if (select(maxfdp, &read_fds, (fd_set*)0, (fd_set*)0, (struct timeval*)0) < 0) {
			printf("select error\n");
			exit(0);
		}
		// network I/O ��ȭ �ִ� ���
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
		// keyboard �Է��� �ִ� ���
		if (FD_ISSET(0, &read_fds)) {
			memset(buf1, 0, BUF_LEN);
			if (fgets(buf1, BUF_LEN, stdin) > 0) {
				sprintf(buf2, "[%s] %s", username, buf1);
				if (send(s, buf2, BUF_LEN, 0) < 0) {
					printf("send error.\n");
					break;
				}
				if (strncmp(buf1, EXIT, strlen(EXIT)) == 0) {
					printf("Good bye.\n");
					close(s);
					break;
				}
			}
			else {
				printf("fgets error\n");
				break;
			}

		}
	}
#endif
}