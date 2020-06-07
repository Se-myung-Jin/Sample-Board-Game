#pragma once

#include <winsock2.h>
#include "Global.h"

class NetworkSession
{
public:
	NetworkSession(void);
	~NetworkSession(void);

	bool Begin();
	bool TCPBind();

	bool Recv(ClientInfo* pClientInfo);
	bool RegisterSendMsg(ClientInfo* pClientInfo, unsigned char* pMsg, int nLen);
	bool Send(ClientInfo* pClientInfo);

	SOCKET GetListenSocket() { return listenSocket; }

private:

	SOCKET listenSocket;
};

