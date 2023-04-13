/*
 파일명 : file_client2.c
 기  능 : file_client1에서 filename 먼저 발송
 컴파일 : cc -o file_client2 file_client2.c
 사용법 : file_client2 [host IP] [port]
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
int   main_socket;

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

#define ECHO_SERVER "127.0.0.1"
#define ECHO_PORT "30000"
#define BUF_LEN 128

int main(int argc, char* argv[]) {
    int s, n, len_in, len_out;
    char cmd[BUF_LEN + 1] = { 0 };
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



    /* echo 서버의 소켓주소 구조체 작성 */
    memset((char*)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    server_addr.sin_port = htons(atoi(port_no));


    /* 연결요청 */
    printf("Connecting %s %s\n", ip_addr, port_no);

    if (connect(s, (struct sockaddr*)&server_addr,
        sizeof(server_addr)) < 0) {
        printf("can't connect.\n");
        exit(0);
    }

    while (1) {
        printf("file_client4> ");
        gets(buf);
        if (strncmp(buf, "put", 3) == 0) {
           
            FILE* fp;
            cmd[5];
            char filename[BUF_LEN];   
            int filesize;
            int readsum = 0, nread;

            sscanf(buf, "%s %s", cmd, filename);
            if ((fp = fopen(filename, "rb")) == NULL) {
                printf("Can't open file %s\n", filename);
                exit(0);
            }          
            fseek(fp, 0, 2);
            filesize = ftell(fp);
            rewind(fp);

            sprintf(buf, "%s %s %d", cmd, filename, filesize);

            if (send(s, buf, BUF_LEN, 0) <= 0) {
                printf("filename send error\n");
                exit(0);
            }

            printf("Sending %s %d bytes\n", filename, filesize);
            readsum = 0;

            if (filesize < BUF_LEN)
                nread = filesize;
            else
                nread = BUF_LEN;

            while (readsum < filesize) {
                int n;
                memset(buf, 0, BUF_LEN + 1);
                n = fread(buf, 1, nread, fp);
                if (n <= 0)
                    break;

                if (send(s, buf, n, 0) <= 0) {
                    printf("send error\n");
                    break;
                }

                readsum += n;
                if ((nread = (filesize - readsum)) > BUF_LEN)
                    nread = BUF_LEN;
            }

            printf("File %s %d bytes transferred\n", filename, filesize);
            fclose(fp);
        }
        else if (strncmp(buf, "get", 3) == 0) {
            char filename[BUF_LEN];
            char cmd[BUF_LEN];
            FILE* fp;
            int filesize, readsum = 0, nread = 0, n;

            if (send(s, buf, BUF_LEN, 0) <= 0) {
                printf("send error\n");
                exit(0);
            }

            if (recv(s, buf, BUF_LEN, 0) <= 0) {
                printf("filename recv error\n");
                exit(0);
            }

            sscanf(buf, "%s %s %d", cmd, filename, &filesize);

            printf("Receiving %s %d bytes\n", filename, filesize);
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
                n = recv(s, buf, nread, 0);
                if (n <= 0) {
                    printf("\nend of file\n");
                    break;
                }

                if (fwrite(buf, n, 1, fp) <= 0) {
                    printf("fwrite error\n");
                    break;
                }

                readsum += n;
                if ((nread = (filesize - readsum)) > BUF_LEN)
                    nread = BUF_LEN;
            }
            printf("\nFile %s %d bytes received.\n", filename, filesize);
            fclose(fp);
        }
        else if (strncmp(buf, "dir", 3) == 0) {
            if (send(s, buf, BUF_LEN, 0) <= 0) {
                printf("filename send error\n");
                exit(0);
            }

            memset(buf, 0, BUF_LEN + 1);
            while (1) {
                if (recv(s, buf, BUF_LEN, 0) <= 0) {
                    printf("recv error\n");
                    break;
                }

                printf("%s", buf);
                if (strcmp(buf, "EOF") == 0) {
                    break;
                }
            }
            printf("\n");
        }
        else if (strncmp(buf, "ldir", 4) == 0) {
            system("dir");
        }

        else if (strncmp(buf, "!", 1) == 0) {
            system(buf + 1);
        }

        else if (strncmp(buf, "quit", 4) == 0) {
            if (send(s, "quit", BUF_LEN, 0) < 0) {
                exit(0);
            }
            exit(0);
        }
    }
#ifdef WIN32
    closesocket(s);
#else
    close(s);
#endif
    return(0);
}
