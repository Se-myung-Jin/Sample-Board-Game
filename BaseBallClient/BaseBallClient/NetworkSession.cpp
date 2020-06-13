#include "NetworkSession.h"
#include "Global.h"


NetworkSession::NetworkSession()
{
	// 윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &m_Wsa) != 0)
		Err_quit("WSAStartup()");

	m_bConnect = FALSE;
}

NetworkSession::~NetworkSession()
{
	// 윈속 종료
	WSACleanup();
}

// 소켓 함수 오류 출력 후 종료
void NetworkSession::Err_quit(LPCSTR msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);

	MessageBox(NULL, static_cast<LPCSTR>(lpMsgBuf), msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	//SendMessage( DXUTGetHWND(), WM_CLOSE, 0, 0 );
}

// 소켓 함수 오류 출력
void NetworkSession::Err_display(LPCSTR msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, static_cast<LPCSTR>(lpMsgBuf), msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
}
BOOL NetworkSession::Connect()
{
	int retval;

	// socket()     
	//   m_Sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0,0,0 );
	m_Sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_Sock == INVALID_SOCKET)
	{
		Err_display("WSASocket()");
		return FALSE;
	}

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORTNUM);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// setsockopt()
	BOOL bNoDelay = TRUE;
	retval = setsockopt(m_Sock, IPPROTO_TCP, TCP_NODELAY, (char*)&bNoDelay, sizeof(bNoDelay));
	if (retval == SOCKET_ERROR)
	{
		Err_display("setsockopt()");
		return FALSE;
	}

	// connect()
	retval = WSAConnect(m_Sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr), NULL, NULL, NULL, NULL);
	if (retval == SOCKET_ERROR)
	{
		Err_display("connect()");
		return FALSE;
	}

	HANDLE hThread;
	DWORD dwThreadID;

	// 데이터를 받기 위한 스레드 생성
	hThread = CreateThread(NULL, 0, ReceiveData, this, 0, &dwThreadID);
	CloseHandle(hThread);

	m_bConnect = TRUE;
	return TRUE;
}

BOOL NetworkSession::DisConnect()
{
	// closesocket()
	if (m_Sock)
		closesocket(m_Sock);
	m_bConnect = FALSE;
	return TRUE;
}


BOOL NetworkSession::SendData(Packet* packet)
{
	WSABUF buf;
	buf.buf = packet->sendBuf;
	buf.len = packet->packSize;
	DWORD data_size = (DWORD)packet->packSize;

	if (WSASend(m_Sock, &buf, 1, &data_size, 0, NULL, NULL) == SOCKET_ERROR)
	{
		Err_display("send()");
		return FALSE;
	}

	return TRUE;
}


DWORD WINAPI ReceiveData(LPVOID arg)
{
	NetworkSession* pNetworkSession = (NetworkSession*)arg;

	int retval;
	char buf[BUFSIZE];
	DWORD dwRecvbytes = 0;
	DWORD dwType = 0;
	char* pPacket = NULL;
	DWORD dwPacketSize = 0;
	DWORD dwRemainSize = 0;

	pPacket = buf;

	// 서버와 데이터 통신
	while (pNetworkSession->GetConnect())
	{
		dwRecvbytes = recv(pNetworkSession->GetSocket(), pPacket, BUFSIZE, 0);
		if (dwRecvbytes == SOCKET_ERROR)
		{
			NetworkSession::Err_display("WSARecv()");
			continue;
		}
		else if (dwRecvbytes == 0)
			break;

		dwRemainSize += dwRecvbytes;

		// 패킷이 다 오지 않은 경우
		if (dwRemainSize < dwPacketSize)
		{
			pPacket = buf + dwRemainSize;
			break;
		}

		dwType = pPacket[1];

		GameStartProtocol startProc;
		ChatProtocol chatProc;
		BaseBallNumberProtocol baseballProc;
		BaseBallResultProtocol baseballNumProc;

		switch (dwType)
		{
		case ACK_GAMESTART:
			startProc.toProtocol(pPacket);
			cout << startProc.data << endl;
			break;
		case ACK_CHAT:
			chatProc.toProtocol(pPacket);
			cout << ">> CHAT- " << chatProc.data << endl;
			break;
		case ACK_BASEBALL_NUMBER:
			baseballProc.toProtocol(pPacket);
			cout << ">> BASEBALL Input Number: " << baseballProc.num << endl;
			break;

		case ACK_BASEBALL_RESULT:
			baseballNumProc.toProtocol(pPacket);
			if (baseballNumProc.strikeCnt == 3)
				cout << "WIN\n";
			cout << ">> BASEBALL- Strike: " << baseballNumProc.strikeCnt << " ,ball: " << baseballNumProc.ballCnt << endl;

			break;
		}

		// 패킷 크기보다 크게 온 경우
		if (dwRemainSize > dwPacketSize)
		{
			// 다음 패킷을 앞으로 복사
			memcpy(buf, buf + dwPacketSize, dwRemainSize - dwPacketSize);
			// 뒷내용은 제거
			memset(buf + dwRemainSize - dwPacketSize, 0, BUFSIZE - (dwRemainSize - dwPacketSize));

			pPacket = buf;

			dwRemainSize -= dwPacketSize;

			continue;
		}
		else
		{
			memset(buf, 0, BUFSIZE);
			pPacket = buf;
			dwRemainSize = 0;
			break;
		}
	} // End while(1)


	return 0;
}