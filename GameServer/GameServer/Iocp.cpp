#include "Iocp.h"


DWORD WINAPI CallWorkerThread(LPVOID arg)
{
	Iocp* pIocp = (Iocp*)arg;
	pIocp->WorkerThread();
	return 0;
}

// Client의 접속을 받기위한 쓰레드
DWORD WINAPI CallAccepterThread(LPVOID arg)
{
	Iocp* pIocp = (Iocp*)arg;
	pIocp->AccepterThread();
	return 0;
}

Iocp::Iocp(void)
{
	// 모든 멤버 변수들 초기화
	m_WorkerRun = true;
	m_AccepterRun = true;
	m_ClientCnt = 0;
	m_AccepterThread = NULL;
	m_IOCP = NULL;
	m_socketListen = INVALID_SOCKET;
	ZeroMemory(m_SocketBuf, 1024);
	for (int i = 0; i < MAX_WORKERTHREAD; i++)
		m_WorkerThread[i] = NULL;
	// Client정보를 저장할 구조체를 생성
	m_ClientInfo = new ClientInfo[MAX_CLIENT];
}

Iocp::~Iocp(void)
{
	// 윈속의 사용을 끝낸다.
	WSACleanup();

	// 다 사용한 객체를 삭제
	if (m_ClientInfo)
	{
		delete[] m_ClientInfo;
		m_ClientInfo = NULL;
	}

	delete m_networkSession;
}

// ------------------------------------------ 
// 소켓 함수 오류 출력 후 종료
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
// 소켓 함수 오류 출력
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
	// 윈속 라이브러리 Load, Listen 소켓 생성
	if (!ConfigSession())
		End();

	// IOCP 생성
	if (!CreateIocp())
		End();
	
	// Accept Thread와 Worker Thread 생성
	if (!CreateThreads())
		End();

	return true;
}

bool Iocp::End()
{

	// Thread 종료
	DestroyThread();

	// 객체를 초기화
	for (int i = 0; i < MAX_CLIENT; i++)
	{
		if (m_ClientInfo[i].m_socketClient != INVALID_SOCKET)
		{
			CloseSocket(&m_ClientInfo[i]);
		}
	}

	// 윈속의 사용을 끝낸다.
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
	// CompletionPort객체 생성 요청을 한다.
	m_IOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (m_IOCP == NULL)
	{
		return false;
	}

	return true;
}

bool Iocp::CreateThreads()
{
	// 접속된 클라이언트 주소 정보를 저장할 구조체
	if (!CreateWorkerThread())
		return false;

	if (!CreateAccepterThread())
		return false;

	return true;
}

void Iocp::CloseSocket(ClientInfo* pClientInfo, bool bIsForce)
{
	struct linger stLinger = { 0, 0 };		// SO_DONTLINGER 로 설정

	// bIsForce가 true이면 SO_LINGER, timeout = 0으로 설정하여
	// 강제종료 시킨다. 주의:데이터 손실이 있을 수 있음
	if (bIsForce)
		stLinger.l_linger = 1;

	// socketClose소켓의 데이터 송수신을 모두 중단 시킨다.
	shutdown(pClientInfo->m_socketClient, SD_BOTH);
	// 소켓 옵션을 설정한다.
	setsockopt(pClientInfo->m_socketClient, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));
	// 소켓 연결을 종료 시킨다.
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
	// Client의 접속요청을 받을 쓰레드 생성
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
			// WaitingThread Queue에서 대기중인 쓰레드에 사용자 종료 메세지를 보낸다
			PostQueuedCompletionStatus(m_IOCP, 0, 0, NULL);
		}

		for (int i = 0; i < MAX_WORKERTHREAD; i++)
		{
			// 쓰레드 핸들을 닫고 쓰레드가 종료될 때까지 기다린다.
			CloseHandle(m_WorkerThread[i]);
			WaitForSingleObject(m_WorkerThread[i], INFINITE);
		}
	}

	m_AccepterRun = false;
	closesocket(m_networkSession->GetListenSocket());
	CloseHandle(m_AccepterThread);
	// 쓰레드 종료를 기다린다.
	WaitForSingleObject(m_AccepterThread, INFINITE);

}

void Iocp::AccepterThread()
{
	SOCKADDR_IN ClientAddr;
	int nAddrLen = sizeof(SOCKADDR_IN);

	while (m_AccepterRun)
	{
		// 접속을 받을 구조체의 인덱스를 얻어온다.
		ClientInfo* pClientInfo = GetEmptyClientInfo();
		if (pClientInfo == NULL)
		{
			return;
		}

		// 클라이언트 접속 요청이 들어올 때까지 기다린다.
		pClientInfo->m_socketClient = accept(m_networkSession->GetListenSocket(), (SOCKADDR*)&ClientAddr, &nAddrLen);
		if (pClientInfo->m_socketClient == INVALID_SOCKET)
			continue;

		printf("[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(ClientAddr.sin_addr), ntohs(ClientAddr.sin_port));

		if (m_ClientCnt > MAX_CLIENT - 1)
		{
			printf("인원 초과! 접속을 해지합니다.\n");

			SOCKADDR_IN clientaddr;
			int addrlen = sizeof(clientaddr);
			getpeername(pClientInfo->m_socketClient, (SOCKADDR *)&clientaddr, &addrlen);
			printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

			CloseSocket(pClientInfo);
			continue;
		}

		if (!RegSockToIocp(pClientInfo))
			return;

		// Recv
		if (!m_networkSession->Recv(pClientInfo))
			return;

		// 클라이언트 갯수 증가
		m_ClientCnt++;

		ConnectProcess(pClientInfo);
	}
}

void Iocp::WorkerThread()
{
	// CompletionKey를 받을 포인터 변수
	ClientInfo* pClientInfo = NULL;
	// 함수 호출 성공 여부
	BOOL bSuccess = TRUE;
	// Overlapped I/O작업에서 전송된 데이터 크기
	DWORD dwIoSize = 0;
	// I/O 작업을 위해 요청한 Overlapped 구조체를 받을 포인터
	LPOVERLAPPED lpOverlapped = NULL;

	while (m_WorkerRun)
	{
		bSuccess = GetQueuedCompletionStatus(m_IOCP,
			&dwIoSize,
			(LPDWORD)&pClientInfo,
			&lpOverlapped,
			INFINITE);

		// client가 접속을 끊었을 경우
		if (!bSuccess && dwIoSize == 0 && pClientInfo != NULL)
		{
			// RingBuffer 초기화

			// Todo SendQueue cleanup


			// 접속 종료 처리
			DisConnectProcess(pClientInfo);

			SOCKADDR_IN clientaddr;
			int addrlen = sizeof(clientaddr);
			getpeername(pClientInfo->m_socketClient, (SOCKADDR *)&clientaddr, &addrlen);
			printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

			CloseSocket(pClientInfo);
			// 클라이언트 갯수 감소
			m_ClientCnt--;
			continue;
		}

		if (TRUE == bSuccess && 0 == dwIoSize && NULL == lpOverlapped) {
			m_WorkerRun = false;
			continue;
		}

		OverlappedEx* pOverlappedEx = (OverlappedEx*)lpOverlapped;

		// Overlapped I/O Recv작업 결과 뒤 처리
		// 데이터를 수신한 경우
		if (pOverlappedEx->m_Operation == OP_RECV) {
			unsigned char *buf_ptr = pOverlappedEx->m_IOCPbuf;
			int restDataSize = dwIoSize;
			while (restDataSize) {
				if (0 == pOverlappedEx->receiving_packet_size)   // 미완성 패킷이 존재하지 않는다. 패킷이 처음부터 전송되었다.
					pOverlappedEx->receiving_packet_size = (int)buf_ptr[0];  // 첫번째 바이트가 패킷 크기이다.
				int required = pOverlappedEx->receiving_packet_size - pOverlappedEx->received;

				if (restDataSize < required) { // 더이상 패킷을 만들 수 없다. 루프를 중지한다.
					memcpy(pOverlappedEx->m_packet_buf + pOverlappedEx->received,
						buf_ptr,
						restDataSize);
					m_networkSession->Recv(pClientInfo);
					break;
				}
				else { // 패킷을 완성할 수 있다.
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
			if (0 < pOverlappedEx->m_RemainLen) { // 패킷을 다 보내지 못했음
				pOverlappedEx->m_wsaBuf.buf += dwIoSize;
				pOverlappedEx->m_wsaBuf.len = pOverlappedEx->m_RemainLen;
				m_networkSession->Send(pClientInfo);
			}
			else { // 한 패킷을 다 보냈음. 다음 보낼 패킷 검사
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
				else { // 더이상 보낼것이 없음. Nobusy 세팅 필요
					pOverlappedEx->m_busySending = false;
				}
			}
		}
		else { // Error
		}
	}
}