/*
���ϸ� : file_server3.c
��  �� : file �� �����ؼ� �����ϴ� ���� file_server2���� filename �� filesize�� ���� ���� �޴´�.
������ : cc -o file_server3 file_server3.c
���� : file_server3 [port]
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

#define BUF_LEN 128
#define file_SERVER "0.0.0.0"
#define file_PORT "30000"

int main(int argc, char* argv[]) {
	struct sockaddr_in server_addr, client_addr;
	int server_fd, client_fd;			/* ���Ϲ�ȣ */
	int len, msg_size;
	char buf[BUF_LEN + 1];
	unsigned int set = 1;
	char* ip_addr = file_SERVER, * port_no = file_PORT;

	if (argc == 2) {
		port_no = argv[1];
	}
#ifdef WIN32
	printf("Windows : ");
	init_winsock();
#else
	printf("Linux : ");
#endif
	/* ���� ���� */
	if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Server: Can't open stream socket.");
		exit(0);
	}
#ifdef WIN32
	main_socket = server_fd;
#endif

	printf("file_server3 waiting connection..\n");
	printf("server_fd = %d\n", server_fd);
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&set, sizeof(set));

	/* server_addr�� '\0'���� �ʱ�ȭ */
	memset((char*)&server_addr, 0, sizeof(server_addr));
	/* server_addr ���� */
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(port_no));

	/* bind() ȣ�� */
	if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		printf("Server: Can't bind local address.\n");
		exit(0);
	}

	/* ������ ���� ������ ���� */
	listen(server_fd, 5);

	/* iterative  file ���� ���� */
	printf("Server : waiting connection request.\n");
	len = sizeof(client_addr);

	while (1) {
		/* �����û�� ��ٸ� */
		client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len);
		if (client_fd < 0) {
			printf("Server: accept failed.\n");
			exit(0);
		}

		printf("Client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		printf("client_fd = %d\n", client_fd);

		char filename[BUF_LEN];
		FILE* fp;
		int filesize, readsum = 0, nread = 0, n;
		if (recv(client_fd, buf, BUF_LEN, 0) <= 0) {
			printf("filename recv error\n");
			exit(0);
		}
		sscanf(buf, "%s %d", filename, &filesize);
		printf("Received filename : %s size = %d\n", filename, filesize);
		if ((fp = fopen(filename, "wb")) == NULL) {
			printf("file open error\n");
			exit(0);
		}
		readsum = 0;
		if (filesize < BUF_LEN)
			nread = filesize;
		else
			nread = BUF_LEN;
		memset(buf, 0, BUF_LEN + 1);

		while (readsum < filesize) {
			n = recv(client_fd, buf, nread, 0);
			if (n <= 0) {
				printf("\nend of file\n");
				break;			
			}
			printf("read data = %d bytes : %s", n, buf);
			if (fwrite(buf, n, 1, fp) <= 0) {
				printf("fwrite error\n");
				break;			
			}
			readsum += n;
			if ((nread = (filesize - readsum)) > BUF_LEN)
				nread = BUF_LEN;

		}
#ifdef WIN32
		closesocket(client_fd);
#else
		close(client_fd);
#endif

	}
#ifdef WIN32
	closesocket(server_fd);
#else
	close(server_fd);
#endif	
	return(0);
}

