#include "IocpServer.h"
#include "GameProtocol.h"
#include "NumericalBaseBall.h"


extern NumericalBaseBall* g_BaseBallManager;

IocpServer::IocpServer()
{
	ZeroMemory(m_Buf, BUFSIZE);
}

IocpServer::~IocpServer()
{
	if (!m_listJoiner.empty())
		m_listJoiner.clear();
}

bool IocpServer::ConnectProcess(ClientInfo* pClientInfo)
{
	ClientBaseBallInfo info;
	DWORD dwType = 0;
	info.pClientInfo = pClientInfo;
	
	m_listJoiner.push_back(info);
	return true;
}

bool IocpServer::DisConnectProcess(ClientInfo* pClientInfo)
{
	ClientBaseBallInfo info;
	DWORD dwType = 0;
	unsigned char packet_size;

	ZeroMemory(&info, sizeof(ClientBaseBallInfo));

	std::list<ClientBaseBallInfo>::iterator iter;
	for (iter = m_listJoiner.begin(); iter != m_listJoiner.end(); ++iter)
	{
		if (iter->pClientInfo == pClientInfo)
		{
			info = *iter;
			iter = m_listJoiner.erase(iter);
			break;
		}
	}

	packet_size = sizeof(DWORD) + sizeof(BYTE) * 2;
	ZeroMemory(m_Buf, BUFSIZE);
	CopyMemory(m_Buf, &packet_size, sizeof(BYTE));		// ��Ŷ ũ�� ����
	CopyMemory(m_Buf + sizeof(BYTE), &dwType, sizeof(DWORD));		// ��Ŷ Type ����
	
	// �ٸ� �����ڿ��� ���������� �˸���.
	for (iter = m_listJoiner.begin(); iter != m_listJoiner.end(); ++iter)
	{
		// �������� ���� ����
		m_networkSession->RegisterSendMsg(iter->pClientInfo, m_Buf, packet_size);
	}

	return false;
}

bool IocpServer::PacketProcess(const unsigned char* pBuf, ClientInfo* pClientInfo)
{
	DWORD dwType = 0;
	std::list<ClientBaseBallInfo>::iterator iter;

	dwType = pBuf[1];

	Packet* sendPack;
	GameStartProtocol startProc;
	ChatProtocol chatProc;
	BaseBallNumberProtocol numProc;
	BaseBallResultProtocol resProc;

	switch (dwType)
	{
	case REQ_GAMESTART:
		cout << "REQ_GAMESTART\n";

		sendPack = startProc.toPacket();

		g_BaseBallManager->startConfig();

		for (iter = m_listJoiner.begin(); iter != m_listJoiner.end(); ++iter)
		{
			m_networkSession->RegisterSendMsg(iter->pClientInfo, (unsigned char*)sendPack->sendBuf, sendPack->packSize);
		}
		break;
	case REQ_CHAT:
		cout << "REQ_CHAT\n";
		chatProc.toProtocol(pBuf);
		sendPack = chatProc.toPacket();

		for (iter = m_listJoiner.begin(); iter != m_listJoiner.end(); ++iter)
		{
			m_networkSession->RegisterSendMsg(iter->pClientInfo, (unsigned char*)sendPack->sendBuf, sendPack->packSize);
		}
		break;
	case REQ_BASEBALL_NUMBER:
		cout << "REQ_BASEBALL_NUMBER\n";
		
		numProc.toProtocol(pBuf);
		g_BaseBallManager->parseNumArr(numProc.num);
		
		if (!g_BaseBallManager->compareNums(resProc.strikeCnt, resProc.ballCnt))
		{
			sendPack = numProc.toPacket();
			for (iter = m_listJoiner.begin(); iter != m_listJoiner.end(); ++iter)
			{
				m_networkSession->RegisterSendMsg(iter->pClientInfo, (unsigned char*)sendPack->sendBuf, sendPack->packSize);
			}
		}

		sendPack = resProc.toPacket();
		
		for (iter = m_listJoiner.begin(); iter != m_listJoiner.end(); ++iter)
		{
			m_networkSession->RegisterSendMsg(iter->pClientInfo, (unsigned char*)sendPack->sendBuf, sendPack->packSize);
		}
		break;
	default:
		return false;
	}
	return true;
}