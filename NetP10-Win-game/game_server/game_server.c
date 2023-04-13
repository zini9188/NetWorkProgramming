/*----------------------
 파일명 : game_server.c   
 채팅 서버를 그대로 응용.
 */

#define MAXLINE  512
#define MAX_SOCK 64

char *escapechar = "exit";
int getmax(int);
void removeClient(int);			/* 채팅 탈퇴 처리 함수 */
int  maxfdp1;              	/* 최대 소켓번호 +1 */
int  num_chat = 0;         	/* 채팅 참가자 수 */
int client_s[MAX_SOCK];    	/* 채팅에 참가자 소켓번호 목록 */

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

	// winsock 사용을 위해 필수적임
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

	fd_set  read_fds;     /* 읽기를 감지할 소켓번호 구조체 */
	struct sockaddr_in  client_addr, server_addr;

	init_winsock();
	if(argc != 2)  {
		printf("사용법 :%s port\n", argv[0]);
		exit(0);
	}
	printf("게임 서버 실행중. port#=%s\n", argv[1]);
	/* 초기소켓 생성 */
	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)  {
		printf("Server: Can't open stream socket.");
		exit(0);
	}
	main_socket=s;

	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&set, sizeof(set));
	/* server_addr 구조체의 내용 세팅 */
	memset((char *)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(argv[1]));

	if (bind(s,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) {
		printf("Server: Can't bind local address.\n");
		exit(0);
	}
	/* 클라이언트로부터 연결요청을 기다림 */
	listen(s, 5);

	maxfdp1 = s + 1;    /* 최대 소켓번호 +1 */

	while(1) {
		FD_ZERO(&read_fds);
		FD_SET(s, &read_fds);
		for(i=0; i<num_chat; i++)  FD_SET(client_s[i], &read_fds);
		maxfdp1 = getmax(s) + 1;     /* maxfdp1 재 계산 */
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
			/* 채팅 클라이언트 목록에 추가 */
			client_s[num_chat] = client_fd;
			printf("%d번째 유저 접속. \n", num_chat+1);
			printf("From %s : %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
			num_chat++;
		}

		/* 클라이언트가 보낸 메시지를 모든 클라이언트에게 방송 */
		for(i = 0; i < num_chat; i++)  {
			int nread;
			if(FD_ISSET(client_s[i], &read_fds)) {
				memset(rline, 0, 128);
				if((nread = recv(client_s[i], rline, 128, 0))  <= 0)  {
					removeClient(i);	/* abrupt exit */
					continue;
				}
				/* 종료문자 처리 */
				if(strstr(rline, escapechar) != NULL) {
					removeClient(i);	/* abrupt exit */
					continue; 
				}
				/* 모든 채팅 참가자에게 메시지 방송 */
				//rline[n] = '\0';
				for (j = 0; j < num_chat; j++)  
					send(client_s[j], rline, nread, 0);
				printf("%s\n", rline);
			}
		}
	}
}

/* 채팅 탈퇴 처리 */
void removeClient(int i) {
	closesocket(client_s[i]); // windows
	    if(i != num_chat-1) client_s[i] = client_s[num_chat-1];
	    num_chat--;
	    printf("게임 참가자 1명 탈퇴. 현재 참가자 수 = %d\n", num_chat);
}

/* clien_s[] 내의 최대 소켓번호 얻기(초기치는 k) */
int getmax(int k) {
	int max = k;    
	    int r;
	    for (r=0; r < num_chat; r++) {
		if (client_s[r] > max ) max = client_s[r];
	}
	return max;
}

