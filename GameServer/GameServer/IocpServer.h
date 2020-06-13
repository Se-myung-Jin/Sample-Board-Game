#pragma once

#include "Iocp.h"
#include "Global.h"

#define BUFSIZE		1024


struct ClientBaseBallInfo
{
	ClientInfo* pClientInfo;

	ClientBaseBallInfo()
	{
		ZeroMemory(this, sizeof(ClientBaseBallInfo));
	}
};

class IocpServer : public Iocp
{
	std::list<ClientBaseBallInfo>		m_listJoiner;
	unsigned char						m_Buf[BUFSIZE];
public:
	IocpServer();
	~IocpServer();
	bool		ConnectProcess(ClientInfo* pClientInfo);
	bool		DisConnectProcess(ClientInfo* pClientInfo);
	bool		PacketProcess(const unsigned char* pBuf, ClientInfo* pClientInfo);
};