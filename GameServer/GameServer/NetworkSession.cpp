#include "NetworkSession.h"


NetworkSession::NetworkSession(void)
{
}


NetworkSession::~NetworkSession(void)
{
}


bool NetworkSession::Begin()
{
	// TCP 소켓 생성
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == listenSocket)
	{
		return false;
	}

	if (!TCPBind())
		return false;

	if (listen(listenSocket, 5) == SOCKET_ERROR)
		return false;

	return true;
}

bool NetworkSession::TCPBind()
{
	SOCKADDR_IN stServerAddr;
	stServerAddr.sin_family = AF_INET;
	stServerAddr.sin_port = htons(9000);
	stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int nRet = bind(listenSocket, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));

	if (nRet != 0)
	{
		return false;
	}

	return true;
}

bool NetworkSession::Recv(ClientInfo* pClientInfo)
{
	DWORD dwFlag = 0;
	char* pBuf = NULL;

	if (pClientInfo == NULL || pClientInfo->m_socketClient == INVALID_SOCKET)
		return false;

	// Overlapped I/O를 위해 각정보를 셋팅해준다.
	pClientInfo->m_RecvOverlappedEx.m_wsaBuf.len = MAX_PACKET_SIZE;
	pClientInfo->m_RecvOverlappedEx.m_wsaBuf.buf = (CHAR *)pClientInfo->m_RecvOverlappedEx.m_IOCPbuf;

	ZeroMemory(&pClientInfo->m_RecvOverlappedEx.m_wsaOverlapped, sizeof(OVERLAPPED));

	int nRet = WSARecv(pClientInfo->m_socketClient,
		&(pClientInfo->m_RecvOverlappedEx.m_wsaBuf),
		1,
		NULL,
		&dwFlag,
		(LPWSAOVERLAPPED)&(pClientInfo->m_RecvOverlappedEx),
		NULL);

	// socket_error 이면 Client Socket이 끊어진걸로 처리한다.
	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))	return false;

	return true;
}

bool NetworkSession::RegisterSendMsg(ClientInfo* pClientInfo, unsigned char* pMsg, int nLen)
{
	if (pClientInfo->m_SendOverlappedEx.m_busySending) {
		unsigned char *newbuf = (unsigned char *)malloc(nLen);
		memcpy(newbuf, pMsg, nLen);
		pClientInfo->m_SendOverlappedEx.m_SendPacketQueue.push(newbuf);
	}
	else {
		pClientInfo->m_SendOverlappedEx.m_busySending = true;

		unsigned packet_size = pMsg[0];
		memcpy(pClientInfo->m_SendOverlappedEx.m_IOCPbuf, pMsg, packet_size);
		pClientInfo->m_SendOverlappedEx.m_RemainLen = packet_size;
		pClientInfo->m_SendOverlappedEx.m_wsaBuf.buf = (CHAR *)pClientInfo->m_SendOverlappedEx.m_IOCPbuf;
		pClientInfo->m_SendOverlappedEx.m_wsaBuf.len = packet_size;
		Send(pClientInfo);
	}
	return true;
}

bool NetworkSession::Send(ClientInfo* pClientInfo)
{
	if (pClientInfo == NULL ||
		pClientInfo->m_SendOverlappedEx.m_wsaBuf.buf == NULL ||
		pClientInfo->m_SendOverlappedEx.m_wsaBuf.len <= 0 ||
		pClientInfo->m_socketClient == INVALID_SOCKET)
		return false;

	ZeroMemory(&pClientInfo->m_SendOverlappedEx.m_wsaOverlapped, sizeof(OVERLAPPED));

	int nRet = WSASend(pClientInfo->m_socketClient,
		&(pClientInfo->m_SendOverlappedEx.m_wsaBuf),
		1,
		NULL,
		0,
		(LPWSAOVERLAPPED)&(pClientInfo->m_SendOverlappedEx),
		NULL);

	// socket_error 이면 Client Socket이 끊어진걸로 처리한다.
	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))	return false;

	return true;
}