#pragma once
#include "Global.h"

#define BUFSIZE      256

DWORD WINAPI         ReceiveData(LPVOID arg);

class Packet;

class NetworkSession
{
	WSADATA         m_Wsa;
	BOOL         m_bConnect;
	SOCKET         m_Sock;
	HANDLE          sendThread_Handle;

public:
	NetworkSession();
	~NetworkSession();
	static void      Err_quit(LPCSTR msg);
	static void      Err_display(LPCSTR msg);

	SOCKET         GetSocket() const { return m_Sock; }
	BOOL         GetConnect() const { return m_bConnect; }

	BOOL         Connect();
	BOOL         DisConnect();

	BOOL         SendData(Packet* packet);
};

