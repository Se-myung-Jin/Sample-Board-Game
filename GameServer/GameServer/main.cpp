#include "Global.h"
#include "IocpServer.h"

int main()
{
	WSADATA wsaData;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
		return false;
	
	IocpServer server;
	server.Begin();

	printf("서버 동작중... 아무키나 누르면 서버가 종료됩니다.\n");
	getchar();

	server.End();

	return 0;
}
