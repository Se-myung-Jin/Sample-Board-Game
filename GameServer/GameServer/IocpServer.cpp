#include "IocpServer.h"


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
	ClientChessInfo info;
	DWORD dwType = 0;
	unsigned char packet_size;
	info.pClientInfo = pClientInfo;
	info.x = 105;
	info.z = 105;

	// 남아있는 아이디 저장
	std::list<ClientChessInfo>::iterator iter;
	for (iter = m_listJoiner.begin(); iter != m_listJoiner.end(); ++iter)
	{
		if (iter->id == info.id)
			info.id++;
		else
			break;
	}

	// 접속자 위치정보 전송
	packet_size = sizeof(DWORD) * 1 + sizeof(BYTE) * 4;
	dwType = SELF_SET;
	ZeroMemory(m_Buf, BUFSIZE);
	CopyMemory(m_Buf, &packet_size, sizeof(BYTE));		// 패킷 크기 저장
	CopyMemory(m_Buf + sizeof(BYTE), &dwType, sizeof(DWORD));		// 패킷 Type 저장
	CopyMemory(m_Buf + sizeof(DWORD) + sizeof(BYTE), &info.id, sizeof(BYTE));		// Id
	CopyMemory((m_Buf + sizeof(DWORD) + sizeof(BYTE) * 2), &info.x, sizeof(BYTE));		// x 위치
	CopyMemory((m_Buf + sizeof(DWORD) + sizeof(BYTE) * 3), &info.z, sizeof(BYTE));		// z 위치

	m_networkSession->RegisterSendMsg(pClientInfo, m_Buf, packet_size);

	int i = 0;
	for (iter = m_listJoiner.begin(); iter != m_listJoiner.end(); ++iter)
	{
		++i;
		if (i == 1)
		{
			dwType = ANOTHER_SET;
			CopyMemory(m_Buf, &packet_size, sizeof(BYTE));		// 패킷 크기 저장
			CopyMemory(m_Buf + sizeof(BYTE), &dwType, sizeof(DWORD));		// 패킷 Type 저장
			CopyMemory(m_Buf + sizeof(DWORD) + sizeof(BYTE), &info.id, sizeof(BYTE));		// Id
			CopyMemory((m_Buf + sizeof(DWORD) + sizeof(BYTE) * 2), &info.x, sizeof(BYTE));		// x 위치
			CopyMemory((m_Buf + sizeof(DWORD) + sizeof(BYTE) * 3), &info.z, sizeof(BYTE));		// z 위치

			m_networkSession->RegisterSendMsg(iter->pClientInfo, m_Buf, packet_size);
		}
	}

	// 접속자 정보에 추가
	m_listJoiner.push_back(info);
	m_listJoiner.sort();
	return true;
}

bool IocpServer::DisConnectProcess(ClientInfo* pClientInfo)
{
	ClientChessInfo info;
	DWORD dwType = 0;
	unsigned char packet_size;

	ZeroMemory(&info, sizeof(ClientChessInfo));

	std::list<ClientChessInfo>::iterator iter;
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
	dwType = ANOTHER_DISCONNECT;
	ZeroMemory(m_Buf, BUFSIZE);
	CopyMemory(m_Buf, &packet_size, sizeof(BYTE));		// 패킷 크기 저장
	CopyMemory(m_Buf + sizeof(BYTE), &dwType, sizeof(DWORD));		// 패킷 Type 저장
	CopyMemory(m_Buf + sizeof(BYTE) + sizeof(WORD), &info.id, sizeof(BYTE));		// Id

	// 다른 접속자에게 접속해제를 알린다.
	for (iter = m_listJoiner.begin(); iter != m_listJoiner.end(); ++iter)
	{
		// 접속해제 정보 전송
		m_networkSession->RegisterSendMsg(iter->pClientInfo, m_Buf, packet_size);
	}

	return false;
}

bool IocpServer::PacketProcess(const unsigned char* pBuf, ClientInfo* pClientInfo)
{
	DWORD dwType = 0;
	unsigned char packet_size;
	BYTE id = 0, x = 0, z = 0;
	std::list<ClientChessInfo>::iterator iter;

	printf("Receiving Packet:");

	if (pBuf[1] == 103)
		printf(" 1P LEFT ");
	if (pBuf[1] == 104)
		printf(" 1P RIGHT ");
	if (pBuf[1] == 105)
		printf(" 2P LEFT ");
	if (pBuf[1] == 106)
		printf(" 2P RIGHT ");
	if (pBuf[1] == 107)
		printf(" GAME START ");

	printf(" %d", pBuf[1]);

	printf("\n");

	dwType = pBuf[1];
	id = pBuf[2];

	switch (dwType)
	{
	case MOVE_2P_LEFT:

		// 캐릭터 이동		
		for (iter = m_listJoiner.begin(); iter != m_listJoiner.end(); ++iter)
		{
			if (iter->id == 0)
			{
				x = iter->x;
			}

			if (iter->id == 1)
			{
				if (iter->z > 5)
					iter->z = iter->z - 2;
				else
					iter->z = 5;

				z = iter->z;
			}
		}
		break;
	case MOVE_2P_RIGHT:

		// 캐릭터 이동
		for (iter = m_listJoiner.begin(); iter != m_listJoiner.end(); ++iter)
		{
			if (iter->id == 0)
			{
				x = iter->x;
			}

			if (iter->id == 1)
			{
				if (iter->z < 225)
					iter->z = iter->z + 2;
				else
					iter->z = 225;

				z = iter->z;
			}
		}
		break;
	case MOVE_1P_LEFT:

		// 캐릭터 이동
		for (iter = m_listJoiner.begin(); iter != m_listJoiner.end(); ++iter)
		{
			if (iter->id == 0)
			{
				if (iter->x > 5)
					iter->x = iter->x - 2;
				else
					iter->x = 5;

				x = iter->x;
			}

			if (iter->id == 1)
			{
				z = iter->z;
			}
		}
		break;
	case MOVE_1P_RIGHT:

		// 캐릭터 이동
		for (iter = m_listJoiner.begin(); iter != m_listJoiner.end(); ++iter)
		{
			if (iter->id == 0)
			{
				if (iter->x < 225)
					iter->x = iter->x + 2;
				else
					iter->x = 225;

				x = iter->x;
			}

			if (iter->id == 1)
			{
				z = iter->z;
			}
		}
		break;
	case GAME_START:
	{
		for (iter = m_listJoiner.begin(); iter != m_listJoiner.end(); ++iter)
		{
			if (iter->id == 0)
			{
				x = iter->x;
			}

			if (iter->id == 1)
			{
				z = iter->z;
			}
		}

		packet_size = sizeof(DWORD) + sizeof(BYTE) * 4;
		dwType = GAME_START;
		ZeroMemory(m_Buf, BUFSIZE);
		CopyMemory(m_Buf, &packet_size, sizeof(BYTE));		// 패킷 크기 저장
		CopyMemory(m_Buf + sizeof(BYTE), &dwType, sizeof(DWORD));		// 패킷 Type 저장
		CopyMemory(m_Buf + sizeof(DWORD) + sizeof(BYTE), &id, sizeof(BYTE));		// Id
		CopyMemory((m_Buf + sizeof(DWORD) + sizeof(BYTE) * 2), &x, sizeof(BYTE));		// x 위치
		CopyMemory((m_Buf + sizeof(DWORD) + sizeof(BYTE) * 3), &z, sizeof(BYTE));		// z 위치

		for (iter = m_listJoiner.begin(); iter != m_listJoiner.end(); ++iter)
		{
			m_networkSession->RegisterSendMsg(iter->pClientInfo, m_Buf, packet_size);
		}
	}
	break;

	default:
		return false;
	}

	packet_size = sizeof(DWORD) + sizeof(BYTE) * 4;
	dwType = POSITION_INFO;
	ZeroMemory(m_Buf, BUFSIZE);
	CopyMemory(m_Buf, &packet_size, sizeof(BYTE));		// 패킷 크기 저장
	CopyMemory(m_Buf + sizeof(BYTE), &dwType, sizeof(DWORD));		// 패킷 Type 저장
	CopyMemory(m_Buf + sizeof(DWORD) + sizeof(BYTE), &id, sizeof(BYTE));		// Id
	CopyMemory((m_Buf + sizeof(DWORD) + sizeof(BYTE) * 2), &x, sizeof(BYTE));		// x 위치
	CopyMemory((m_Buf + sizeof(DWORD) + sizeof(BYTE) * 3), &z, sizeof(BYTE));		// z 위치

	// 모든 접속자에게 이동된 id의 변경된 좌표 전송
	for (iter = m_listJoiner.begin(); iter != m_listJoiner.end(); ++iter)
	{
		// 위치정보 전송
		m_networkSession->RegisterSendMsg(iter->pClientInfo, m_Buf, packet_size);
	}
	
	return true;
}