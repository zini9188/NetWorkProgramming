// popen_test.c 
// ��ɾ �����ϰ� ����� ���Ϸ� �д� ����
#include <stdio.h>
#include <stdlib.h>
void main()
{
	char str[129] = { 0 };
	FILE* fp;
	char buf[129] = { 0 };
	int n;

	while (1) {
		printf("\nEnter command : ");
		gets(str);
		if (strcmp(str, "quit") == 0)
			break;
		//system(str);
		// file_server4�� gets(str) ���
		// recv(client_fd, buf, 128, 0); �� ����ϰ�
		// if (strncmp("dir", buf, 3)==0) �̸�
		//  popen() ó���� �ϰ� ����� ������ �ȴ�.

#ifdef WIN32
		fp = _popen(str, "r");
#else // Linux �� dir ��� ls -l ���
		if (strcmp("dir", str) == 0)
			strcpy(str, "ls -l");
		fp = popen(str, "r");
#endif
		while (1) {
			memset(buf, 0, 128);
			n = fread(buf, 1, 128, fp);
			if (n <= 0)
				break;
			//printf("n=%d\n", n);
			printf(buf); // ȭ�鿡 ���
			// file_server4��
			// ���� ftp_client4���� dir ��ɾ ��û�Ѵٸ�
			// printf() ��� send(client_fd, buf, 128, 0); ���� �ȴ�.
			// 
		}
#ifdef WIN32
		_pclose(fp);
#else
		pclose(fp);
#endif
	}
}