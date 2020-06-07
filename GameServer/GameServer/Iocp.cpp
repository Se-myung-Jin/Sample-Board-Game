#include "Iocp.h"


DWORD WINAPI CallWorkerThread(LPVOID arg)
{
	Iocp* pIocp = (Iocp*)arg;
	pIocp->WorkerThread();
	return 0;
}

// Client�� ������ �ޱ����� ������
DWORD WINAPI CallAccepterThread(LPVOID arg)
{
	Iocp* pIocp = (Iocp*)arg;
	pIocp->AccepterThread();
	return 0;
}

Iocp::Iocp(void)
{
	// ��� ��� ������ �ʱ�ȭ
	m_WorkerRun = true;
	m_AccepterRun = true;
	m_ClientCnt = 0;
	m_AccepterThread = NULL;
	m_IOCP = NULL;
	m_socketListen = INVALID_SOCKET;
	ZeroMemory(m_SocketBuf, 1024);
	for (int i = 0; i < MAX_WORKERTHREAD; i++)
		m_WorkerThread[i] = NULL;
	// Client������ ������ ����ü�� ����
	m_ClientInfo = new ClientInfo[MAX_CLIENT];
}

Iocp::~Iocp(void)
{
	// ������ ����� ������.
	WSACleanup();

	// �� ����� ��ü�� ����
	if (m_ClientInfo)
	{
		delete[] m_ClientInfo;
		m_ClientInfo = NULL;
	}

	delete m_networkSession;
}

// ------------------------------------------ 
// ���� �Լ� ���� ��� �� ����
// ------------------------------------------ 
void Iocp::Err_Quit(LPCSTR msg)
{
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(-1);
}

// ------------------------------------------ 
// ���� �Լ� ���� ���
// ------------------------------------------ 
void Iocp::Err_Display(LPCSTR msg)
{
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
}

bool Iocp::Begin()
{
	m_networkSession = new NetworkSession();
	// ���� ���̺귯�� Load, Listen ���� ����
	if (!ConfigSession())
		End();

	// IOCP ����
	if (!CreateIocp())
		End();
	
	// Accept Thread�� Worker Thread ����
	if (!CreateThreads())
		End();

	return true;
}

bool Iocp::End()
{

	// Thread ����
	DestroyThread();

	// ��ü�� �ʱ�ȭ
	for (int i = 0; i < MAX_CLIENT; i++)
	{
		if (m_ClientInfo[i].m_socketClient != INVALID_SOCKET)
		{
			CloseSocket(&m_ClientInfo[i]);
		}
	}

	// ������ ����� ������.
	WSACleanup();

	return true;
}

bool Iocp::ConfigSession()
{
	if (!m_networkSession->Begin())
		return false;

	return true;
}

bool Iocp::CreateIocp()
{
	// CompletionPort��ü ���� ��û�� �Ѵ�.
	m_IOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (m_IOCP == NULL)
	{
		return false;
	}

	return true;
}

bool Iocp::CreateThreads()
{
	// ���ӵ� Ŭ���̾�Ʈ �ּ� ������ ������ ����ü
	if (!CreateWorkerThread())
		return false;

	if (!CreateAccepterThread())
		return false;

	return true;
}

void Iocp::CloseSocket(ClientInfo* pClientInfo, bool bIsForce)
{
	struct linger stLinger = { 0, 0 };		// SO_DONTLINGER �� ����

	// bIsForce�� true�̸� SO_LINGER, timeout = 0���� �����Ͽ�
	// �������� ��Ų��. ����:������ �ս��� ���� �� ����
	if (bIsForce)
		stLinger.l_linger = 1;

	// socketClose������ ������ �ۼ����� ��� �ߴ� ��Ų��.
	shutdown(pClientInfo->m_socketClient, SD_BOTH);
	// ���� �ɼ��� �����Ѵ�.
	setsockopt(pClientInfo->m_socketClient, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));
	// ���� ������ ���� ��Ų��.
	pClientInfo->m_socketClient = INVALID_SOCKET;
}

bool Iocp::CreateWorkerThread()
{
	DWORD uiThreadId = 0;

	for (int i = 0; i < MAX_WORKERTHREAD; i++)
	{
		//		m_hWorkerThread[i] = (HANDLE)_beginthreadex( NULL, 0, &CallWorkerThread, this, CREATE_SUSPENDED, &uiThreadId );
		m_WorkerThread[i] = CreateThread(NULL, 0, CallWorkerThread, this, CREATE_SUSPENDED, &uiThreadId);
		if (m_WorkerThread == NULL)
		{
			return false;
		}
		ResumeThread(m_WorkerThread[i]);
	}

	return true;
}

bool Iocp::CreateAccepterThread()
{
	DWORD uiThreadId = 0;
	// Client�� ���ӿ�û�� ���� ������ ����
	// m_AccepterThread = (HANDLE)_beginthreadex( NULL, 0, &CallAccepterThread, this, CREATE_SUSPENDED, &uiThreadId );
	m_AccepterThread = CreateThread(NULL, 0, CallAccepterThread, this, CREATE_SUSPENDED, &uiThreadId);
	if (m_AccepterThread == NULL)
	{
		return false;
	}

	ResumeThread(m_AccepterThread);
	return true;
}

bool Iocp::RegSockToIocp(ClientInfo* pClientInfo)
{
	HANDLE hIOCP;
	hIOCP = CreateIoCompletionPort((HANDLE)pClientInfo->m_socketClient, m_IOCP, reinterpret_cast<ULONG_PTR>(pClientInfo), 0);
	if (hIOCP == NULL || m_IOCP != hIOCP)
	{
		return false;
	}
	return true;
}

ClientInfo* Iocp::GetEmptyClientInfo()
{
	for (int i = 0; i < MAX_CLIENT; i++)
	{
		if (m_ClientInfo[i].m_socketClient == INVALID_SOCKET)
			return &m_ClientInfo[i];
	}
	return NULL;
}

void Iocp::DestroyThread()
{
	if (m_IOCP != NULL)
	{
		for (int i = 0; i < MAX_WORKERTHREAD; i++)
		{
			// WaitingThread Queue���� ������� �����忡 ����� ���� �޼����� ������
			PostQueuedCompletionStatus(m_IOCP, 0, 0, NULL);
		}

		for (int i = 0; i < MAX_WORKERTHREAD; i++)
		{
			// ������ �ڵ��� �ݰ� �����尡 ����� ������ ��ٸ���.
			CloseHandle(m_WorkerThread[i]);
			WaitForSingleObject(m_WorkerThread[i], INFINITE);
		}
	}

	m_AccepterRun = false;
	closesocket(m_networkSession->GetListenSocket());
	CloseHandle(m_AccepterThread);
	// ������ ���Ḧ ��ٸ���.
	WaitForSingleObject(m_AccepterThread, INFINITE);

}

void Iocp::AccepterThread()
{
	SOCKADDR_IN ClientAddr;
	int nAddrLen = sizeof(SOCKADDR_IN);

	while (m_AccepterRun)
	{
		// ������ ���� ����ü�� �ε����� ���´�.
		ClientInfo* pClientInfo = GetEmptyClientInfo();
		if (pClientInfo == NULL)
		{
			return;
		}

		// Ŭ���̾�Ʈ ���� ��û�� ���� ������ ��ٸ���.
		pClientInfo->m_socketClient = accept(m_networkSession->GetListenSocket(), (SOCKADDR*)&ClientAddr, &nAddrLen);
		if (pClientInfo->m_socketClient == INVALID_SOCKET)
			continue;

		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			inet_ntoa(ClientAddr.sin_addr), ntohs(ClientAddr.sin_port));

		if (m_ClientCnt > MAX_CLIENT - 1)
		{
			printf("�ο� �ʰ�! ������ �����մϴ�.\n");

			SOCKADDR_IN clientaddr;
			int addrlen = sizeof(clientaddr);
			getpeername(pClientInfo->m_socketClient, (SOCKADDR *)&clientaddr, &addrlen);
			printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

			CloseSocket(pClientInfo);
			continue;
		}

		if (!RegSockToIocp(pClientInfo))
			return;

		// Recv
		if (!m_networkSession->Recv(pClientInfo))
			return;

		// Ŭ���̾�Ʈ ���� ����
		m_ClientCnt++;

		ConnectProcess(pClientInfo);
	}
}

void Iocp::WorkerThread()
{
	// CompletionKey�� ���� ������ ����
	ClientInfo* pClientInfo = NULL;
	// �Լ� ȣ�� ���� ����
	BOOL bSuccess = TRUE;
	// Overlapped I/O�۾����� ���۵� ������ ũ��
	DWORD dwIoSize = 0;
	// I/O �۾��� ���� ��û�� Overlapped ����ü�� ���� ������
	LPOVERLAPPED lpOverlapped = NULL;

	while (m_WorkerRun)
	{
		bSuccess = GetQueuedCompletionStatus(m_IOCP,
			&dwIoSize,
			(LPDWORD)&pClientInfo,
			&lpOverlapped,
			INFINITE);

		// client�� ������ ������ ���
		if (!bSuccess && dwIoSize == 0 && pClientInfo != NULL)
		{
			// RingBuffer �ʱ�ȭ

			// Todo SendQueue cleanup


			// ���� ���� ó��
			DisConnectProcess(pClientInfo);

			SOCKADDR_IN clientaddr;
			int addrlen = sizeof(clientaddr);
			getpeername(pClientInfo->m_socketClient, (SOCKADDR *)&clientaddr, &addrlen);
			printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

			CloseSocket(pClientInfo);
			// Ŭ���̾�Ʈ ���� ����
			m_ClientCnt--;
			continue;
		}

		if (TRUE == bSuccess && 0 == dwIoSize && NULL == lpOverlapped) {
			m_WorkerRun = false;
			continue;
		}

		OverlappedEx* pOverlappedEx = (OverlappedEx*)lpOverlapped;

		// Overlapped I/O Recv�۾� ��� �� ó��
		// �����͸� ������ ���
		if (pOverlappedEx->m_Operation == OP_RECV) {
			unsigned char *buf_ptr = pOverlappedEx->m_IOCPbuf;
			int restDataSize = dwIoSize;
			while (restDataSize) {
				if (0 == pOverlappedEx->receiving_packet_size)   // �̿ϼ� ��Ŷ�� �������� �ʴ´�. ��Ŷ�� ó������ ���۵Ǿ���.
					pOverlappedEx->receiving_packet_size = (int)buf_ptr[0];  // ù��° ����Ʈ�� ��Ŷ ũ���̴�.
				int required = pOverlappedEx->receiving_packet_size - pOverlappedEx->received;

				if (restDataSize < required) { // ���̻� ��Ŷ�� ���� �� ����. ������ �����Ѵ�.
					memcpy(pOverlappedEx->m_packet_buf + pOverlappedEx->received,
						buf_ptr,
						restDataSize);
					m_networkSession->Recv(pClientInfo);
					break;
				}
				else { // ��Ŷ�� �ϼ��� �� �ִ�.
					memcpy(pOverlappedEx->m_packet_buf + pOverlappedEx->received,
						buf_ptr,
						required);
					bool ret = PacketProcess(pOverlappedEx->m_packet_buf, pClientInfo);
					pOverlappedEx->received = 0;
					restDataSize -= required;
					buf_ptr += required;
					pOverlappedEx->receiving_packet_size = 0;
				}
			}
			m_networkSession->Recv(pClientInfo);
		}
		else if (pOverlappedEx->m_Operation == OP_SEND) {
			pOverlappedEx->m_RemainLen -= dwIoSize;
			if (0 < pOverlappedEx->m_RemainLen) { // ��Ŷ�� �� ������ ������
				pOverlappedEx->m_wsaBuf.buf += dwIoSize;
				pOverlappedEx->m_wsaBuf.len = pOverlappedEx->m_RemainLen;
				m_networkSession->Send(pClientInfo);
			}
			else { // �� ��Ŷ�� �� ������. ���� ���� ��Ŷ �˻�
				if (!pOverlappedEx->m_SendPacketQueue.empty()) {
					unsigned char *packet_ptr = pOverlappedEx->m_SendPacketQueue.front();
					pOverlappedEx->m_SendPacketQueue.pop();
					unsigned packet_size = packet_ptr[0];
					memcpy(pOverlappedEx->m_IOCPbuf, packet_ptr, packet_size);
					free(packet_ptr);
					pOverlappedEx->m_RemainLen = packet_size;
					pOverlappedEx->m_wsaBuf.buf = (CHAR *)pOverlappedEx->m_IOCPbuf;
					pOverlappedEx->m_wsaBuf.len = pOverlappedEx->m_RemainLen;
					m_networkSession->Send(pClientInfo);
				}
				else { // ���̻� �������� ����. Nobusy ���� �ʿ�
					pOverlappedEx->m_busySending = false;
				}
			}
		}
		else { // Error
		}
	}
}