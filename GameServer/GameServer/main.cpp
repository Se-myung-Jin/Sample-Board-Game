#include "Global.h"
#include "IocpServer.h"

int main()
{
	WSADATA wsaData;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
		return false;
	
	IocpServer server;
	server.Begin();

	printf("���� ������... �ƹ�Ű�� ������ ������ ����˴ϴ�.\n");
	getchar();

	server.End();

	return 0;
}
