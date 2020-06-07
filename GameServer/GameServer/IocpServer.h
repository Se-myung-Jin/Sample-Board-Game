#pragma once

#include "Iocp.h"
#include "Global.h"

#define BUFSIZE		20

enum PACKET_TYPE
{
	SELF_SET = 99,
	ANOTHER_SET,
	POSITION_INFO,
	ANOTHER_DISCONNECT,
	MOVE_1P_LEFT,
	MOVE_1P_RIGHT,
	MOVE_2P_LEFT,
	MOVE_2P_RIGHT,
	GAME_START,
};

struct ClientChessInfo
{
	ClientInfo* pClientInfo;
	BYTE id;
	BYTE x;
	BYTE z;

	ClientChessInfo()
	{
		ZeroMemory(this, sizeof(ClientChessInfo));
	}

	bool operator < (ClientChessInfo rhs) { return this->id < rhs.id ? true : false; }
	bool operator > (ClientChessInfo rhs) { return this->id > rhs.id ? true : false; }
	bool operator == (ClientChessInfo rhs) { return this->id == rhs.id ? true : false; }
};

class IocpServer : public Iocp
{
	std::list<ClientChessInfo>		m_listJoiner;
	unsigned char					m_Buf[BUFSIZE];
public:
	IocpServer();
	~IocpServer();
	bool		ConnectProcess(ClientInfo* pClientInfo);
	bool		DisConnectProcess(ClientInfo* pClientInfo);
	bool		PacketProcess(const unsigned char* pBuf, ClientInfo* pClientInfo);
};