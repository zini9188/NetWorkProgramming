/*----------------------
 ���ϸ� : game_server.c   
 ä�� ������ �״�� ����.
 */

#define MAXLINE  512
#define MAX_SOCK 64

char *escapechar = "exit";
int getmax(int);
void removeClient(int);			/* ä�� Ż�� ó�� �Լ� */
int  maxfdp1;              	/* �ִ� ���Ϲ�ȣ +1 */
int  num_chat = 0;         	/* ä�� ������ �� */
int client_s[MAX_SOCK];    	/* ä�ÿ� ������ ���Ϲ�ȣ ��� */

#include <winsock.h>
#include <signal.h>
#include <stdio.h>
#include <conio.h>

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

	// winsock ����� ���� �ʼ�����
	signal(SIGINT, exit_callback);
	sversion = MAKEWORD(1,1);
	WSAStartup(sversion, &wsadata);
}

int main(int argc, char *argv[])  {
	char rline[MAXLINE], my_msg[MAXLINE];
	char *start = "Connected to chat_server \n";
	int i, j, n;
	int s, client_fd, clilen;
	unsigned int set = 1;

	fd_set  read_fds;     /* �б⸦ ������ ���Ϲ�ȣ ����ü */
	struct sockaddr_in  client_addr, server_addr;

	init_winsock();
	if(argc != 2)  {
		printf("���� :%s port\n", argv[0]);
		exit(0);
	}
	printf("���� ���� ������. port#=%s\n", argv[1]);
	/* �ʱ���� ���� */
	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)  {
		printf("Server: Can't open stream socket.");
		exit(0);
	}
	main_socket=s;

	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&set, sizeof(set));
	/* server_addr ����ü�� ���� ���� */
	memset((char *)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(argv[1]));

	if (bind(s,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) {
		printf("Server: Can't bind local address.\n");
		exit(0);
	}
	/* Ŭ���̾�Ʈ�κ��� �����û�� ��ٸ� */
	listen(s, 5);

	maxfdp1 = s + 1;    /* �ִ� ���Ϲ�ȣ +1 */

	while(1) {
		FD_ZERO(&read_fds);
		FD_SET(s, &read_fds);
		for(i=0; i<num_chat; i++)  FD_SET(client_s[i], &read_fds);
		maxfdp1 = getmax(s) + 1;     /* maxfdp1 �� ��� */
		if (select(maxfdp1, &read_fds, (fd_set *)0, (fd_set *)0,(struct timeval *)0) < 0) {
			printf("select error <= 0 \n");
			exit(0);
		}
		if(FD_ISSET(s, &read_fds)) {
			clilen = sizeof(client_addr);
			client_fd = accept(s, (struct sockaddr *)&client_addr, &clilen);
			if(client_fd == -1)  {
				printf("accept error\n");
				exit(0);
			}
			/* ä�� Ŭ���̾�Ʈ ��Ͽ� �߰� */
			client_s[num_chat] = client_fd;
			printf("%d��° ���� ����. \n", num_chat+1);
			printf("From %s : %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
			num_chat++;
		}

		/* Ŭ���̾�Ʈ�� ���� �޽����� ��� Ŭ���̾�Ʈ���� ��� */
		for(i = 0; i < num_chat; i++)  {
			int nread;
			if(FD_ISSET(client_s[i], &read_fds)) {
				memset(rline, 0, 128);
				if((nread = recv(client_s[i], rline, 128, 0))  <= 0)  {
					removeClient(i);	/* abrupt exit */
					continue;
				}
				/* ���Ṯ�� ó�� */
				if(strstr(rline, escapechar) != NULL) {
					removeClient(i);	/* abrupt exit */
					continue; 
				}
				/* ��� ä�� �����ڿ��� �޽��� ��� */
				//rline[n] = '\0';
				for (j = 0; j < num_chat; j++)  
					send(client_s[j], rline, nread, 0);
				printf("%s\n", rline);
			}
		}
	}
}

/* ä�� Ż�� ó�� */
void removeClient(int i) {
	closesocket(client_s[i]); // windows
	    if(i != num_chat-1) client_s[i] = client_s[num_chat-1];
	    num_chat--;
	    printf("���� ������ 1�� Ż��. ���� ������ �� = %d\n", num_chat);
}

/* clien_s[] ���� �ִ� ���Ϲ�ȣ ���(�ʱ�ġ�� k) */
int getmax(int k) {
	int max = k;    
	    int r;
	    for (r=0; r < num_chat; r++) {
		if (client_s[r] > max ) max = client_s[r];
	}
	return max;
}

