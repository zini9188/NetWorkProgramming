/*
 ���ϸ� : file_client1.c
 ��  �� : echo_client1���� Keyboard ��� file �� �о ����
 ������ : cc -o file_client1 file_client1.c
 ���� : file_client1 [host IP] [port]
*/
#ifdef WIN32
#include <winsock.h>
#include <signal.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#else
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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

#define ECHO_SERVER "127.0.0.1"
#define ECHO_PORT "30000"
#define BUF_LEN 128

int main(int argc, char* argv[]) {
	int s, n, len_in, len_out;
	struct sockaddr_in server_addr;
	char* ip_addr = ECHO_SERVER, * port_no = ECHO_PORT;
	char buf[BUF_LEN + 1] = { 0 };


	if (argc == 3) {
		ip_addr = argv[1];
		port_no = argv[2];
	}
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



	/* echo ������ �����ּ� ����ü �ۼ� */
	memset((char*)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip_addr);
	server_addr.sin_port = htons(atoi(port_no));

	// ���ϸ� �Է�
	FILE* fp;
	char filename[BUF_LEN] = "data.txt"; // data file ��
	printf("Enter file name : ");
	scanf("%s", filename);
	getchar(); // Enter key ó��.
	if ((fp = fopen(filename, "r")) == NULL) {
		printf("Can't open file %s\n", filename);
		exit(0);
	}

	/* �����û */
	printf("Connecting %s %s\n", ip_addr, port_no);

	if (connect(s, (struct sockaddr*)&server_addr,
		sizeof(server_addr)) < 0) {
		printf("can't connect.\n");
		exit(0);
	}

	while (1) {
		/* ���Ͽ��� �Է��� ���� */
		//printf("Input string : ");
		//if (fgets(buf, BUF_LEN, stdin)) { // gets(buf);
		if (fgets(buf, BUF_LEN, fp)) {
			len_out = strlen(buf);
			buf[BUF_LEN] = '\0';
		}
		else {
			//printf("fgets error\n");
			printf("\nEnd of file\n");
			exit(0);
		}
		/* echo ������ �޽��� �۽� */
		printf("Sending len=%d : %s", len_out, buf);
		if (send(s, buf, len_out, 0) < 0) {
			printf("send error\n");
			exit(0);
		}
		if (strcmp(buf, "exit\n") == 0)
			break;
		if ((n = recv(s, buf, BUF_LEN, 0)) < 0) {
			printf("recv error\n");
			exit(0);
		}
		buf[n] = '\0'; // ���ڿ� ���� NULL �߰�
		printf("Received len=%d : %s", n, buf);
	}
#ifdef WIN32
	closesocket(s);
#else
	close(s);
#endif
	return(0);
}

