/*----------------------
 파일명 : chat_server6.c
 기  능 : 채팅서버, chat_server4
 도전과제 1 : 1:1 대화 모드 /with
 도전과제 2 : 1:1 파일전송 /filesend
 사용법 : chat_server4 [port]
------------------------*/
#ifdef WIN32
#define  _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)
#pragma warning(disable:4267)
#pragma warning(disable:4244)
#pragma warning(disable:6054)
#pragma warning(disable:6031)
#pragma warning(disable:6385)

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

#define MAXCLIENTS 64		// 최대 채팅 참가자 수
#define EXIT	"exit"		// 채팅 종료 문자열
int maxfdp;              	// select() 에서 감시해야할 # of socket 변수 getmax() return 값 + 1
int getmax(int);			// 최대 소켓번호 계산
int num_chat = 0;         	// 채팅 참가자 수
int client_fds[MAXCLIENTS];	// 채팅에 참가자 소켓번호 목록
void RemoveClient(int);		// 채팅 탈퇴 처리 함수

#define BUF_LEN	128
#define CHAT_SERVER "0.0.0.0"
#define CHAT_PORT "30000"
char userlist[MAXCLIENTS][BUF_LEN]; // user name 보관용
int usersleep[MAXCLIENTS]; // sleep 상태인지 상태 보관
int userwith[MAXCLIENTS]; // 1:1 대화 상대 보관

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

int main(int argc, char* argv[]) {
	char buf[BUF_LEN], buf1[BUF_LEN], buf2[BUF_LEN];
	int i, j, n, ret;
	int server_fd, client_fd, client_len;
	unsigned int set = 1;
	char* ip_addr = CHAT_SERVER, * port_no = CHAT_PORT;
	fd_set  read_fds;     // 읽기를 감지할 소켓번호 구조체 server_fd 와 client_fds[] 를 설정한다.
	struct sockaddr_in  client_addr, server_addr;
	int client_error[MAXCLIENTS];

#ifdef WIN32
	printf("Windows : ");
	init_winsock();
#else
	printf("Linux : ");
#endif
	/* 소켓 생성 */
	if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Server: Can't open stream socket.");
		exit(0);
	}
#ifdef WIN32
	main_socket = server_fd;
#endif

	printf("chat_server4 waiting connection..\n");
	printf("server_fd = %d\n", server_fd);
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&set, sizeof(set));

	/* server_addr을 '\0'으로 초기화 */
	memset((char*)&server_addr, 0, sizeof(server_addr));
	/* server_addr 세팅 */
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(port_no));

	if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		printf("Server: Can't bind local address.\n");
		exit(0);
	}
	/* 클라이언트로부터 연결요청을 기다림 */
	listen(server_fd, 5);

	while (1) {
		FD_ZERO(&read_fds); // 변수 초기화
		FD_SET(server_fd, &read_fds); // accept() 대상 소켓 설정
		for (i = 0; i < num_chat; i++) // 채팅에 참가중이 모든 client 소켓을 reac() 대상 추가
			FD_SET(client_fds[i], &read_fds);
		maxfdp = getmax(server_fd) + 1;     // 감시대상 소켓의 수를 계산
		if (select(maxfdp, &read_fds, (fd_set*)0, (fd_set*)0, (struct timeval*)0) <= 0) {
			printf("select error <= 0 \n");
			exit(0);
		}
		// 초기 소켓 즉, server_fd 에 변화가 있는지 검사
		if (FD_ISSET(server_fd, &read_fds)) {
			// 변화가 있다 --> client 가 connect로 연결 요청을 한 것
			client_len = sizeof(client_addr);
			client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
			if (client_fd == -1) {
				printf("accept error\n");
			}
			else {
				printf("Client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr),
					ntohs(client_addr.sin_port));
				printf("client_fd = %d\n", client_fd);
				/* 채팅 클라이언트 목록에 추가 */
				printf("client[%d] 입장. 현재 참가자 수 = %d\n", num_chat, num_chat + 1);
				client_fds[num_chat++] = client_fd;
			}
		}

		memset(client_error, 0, sizeof(client_error));
		/* 클라이언트가 보낸 메시지를 모든 클라이언트에게 방송 */
		for (i = 0; i < num_chat; i++) {
			// 각각의 client들의 I/O 변화가 있는지.
			if (FD_ISSET(client_fds[i], &read_fds)) {
				// Read One 또는 client 비정상 종료 확인
				memset(buf, 0, BUF_LEN);
				if ((n = recv(client_fds[i], buf, BUF_LEN, 0)) <= 0) {
					// client 가 비 정상 종료한 경우
					printf("recv error for client[%d]\n", i);
					client_error[i] = 1;
					continue;
				}
				printf("received %d from client[%d] : %s", n, i, buf);
				// "/login username" --> buf1 = /login, buf2 = username
				// "/list" --> buf1 = /list
				// "[username] message .." buf1 = [username], buf2 = message ...
				sscanf(buf, "%s", buf1); // 처음 문자열 분리 [username] 또는 /login
				n = strlen(buf1); // "/login username" or "[username] Hello" 에서 /login 나 [username] 만 분리
				// /login 처리
				if (strncmp(buf1, CHAT_CMD_LOGIN, strlen(CHAT_CMD_LOGIN)) == 0) { // "/login"
					strcpy(userlist[i], buf + n + 1); // username 보관
					printf(" userlist[%d] = %s\n", i, userlist[i]);
					for (j = 0; j < num_chat; j++) {
						if (j != i) { // 본인 제외 다른 사용자에게 입장을 알린다.
						}
					}
					continue;
				}
				// /list 처리
				if (strncmp(buf + n + 1, CHAT_CMD_LIST, strlen(CHAT_CMD_LIST)) == 0) { // "/list"
					printf("Sending user list to client[%d] %s\n", i, userlist[i]);
					memset(buf, 0, BUF_LEN);
					sprintf(buf, "User List\nNo\tname\tstatus\n----------------------\n");
					if (send(client_fds[i], buf, BUF_LEN, 0) < 0) {
						printf("client[%d] send error.", i);
						client_error[i] = 1;
						continue;
					}
					for (j = 0; j < num_chat; j++) {
						memset(buf, 0, BUF_LEN);
						sprintf(buf, "%02d\t%s\t%s\n", j, userlist[j],
							usersleep[j] ? "S" : "O");
						if (send(client_fds[i], buf, BUF_LEN, 0) < 0) {
							printf("client[%d] send error.", i);
							client_error[i] = 1;
							break;
						}
					}
					memset(buf, 0, BUF_LEN);
					sprintf(buf, "----------------------\n");
					if (send(client_fds[i], buf, BUF_LEN, 0) < 0) {
						printf("client[%d] send error.", i);
						client_error[i] = 1;
						continue;
					}
					continue;
				}
				// /to 처리 귓속말 처리
				// [user1] /to user2 message ...

				// /exit 처리
				// exit 대신 /exit 로 변경.
				if (strncmp(buf + n + 1, CHAT_CMD_EXIT, strlen(CHAT_CMD_EXIT)) == 0) { // "/exit"
					RemoveClient(i);
					continue;
				}
				// /sleep 처리
				// /wakeup 처리
				// 모든 채팅 참가자에게 메시지 방송
				//printf("%s", buf);
				// Wrie All
				for (j = 0; j < num_chat; j++) {
					// user가 sleep 이면 어떤 메시지도 수신하지 않는다.
					ret = send(client_fds[j], buf, BUF_LEN, 0);
					if (ret <= 0) {
						printf("send error for client[%d]\n", j);
						client_error[j] = 1;
					}
				}
			}
		}
		// 오류가 난 Client들을 뒤에서 앞으로 가면서 제거한다.
		for (i = num_chat - 1; i >= 0; i--) {
			if (client_error[i])
				RemoveClient(i);
		}
	}
}

/* 채팅 탈퇴 처리 */
void RemoveClient(int i) {
#ifdef WIN32
	closesocket(client_fds[i]);
#else
	close(client_fds[i]);
#endif
	// 마지막 client를 삭제된 자리로 이동 (한칸씩 내릴 필요가 없다)
	printf("client[%d] %s 퇴장. 현재 참가자 수 = %d\n", i, userlist[i], num_chat - 1);
	for (int j = 0; j < num_chat; j++) {
		char buf[BUF_LEN];
		if (j != i) { // 본인 제외 다른 사용자에게 퇴장을 알린다.
		}
	}
	if (i != num_chat - 1) {
		client_fds[i] = client_fds[num_chat - 1]; // socket 정보 
		strcpy(userlist[i], userlist[num_chat - 1]); // username 
		usersleep[i] = usersleep[num_chat - 1]; // sleep 상태
	}
	num_chat--;
}

// client_fds[] 내의 최대 소켓번호 확인
// select(maxfds, ..) 에서 maxfds = getmax(server_fd) + 1
int getmax(int k) {
	int max = k;
	int r;
	for (r = 0; r < num_chat; r++) {
		if (client_fds[r] > max) max = client_fds[r];
	}
	return max;
}