#pragma once
#include "NetworkSession.h"
#include "Global.h"

#include <mutex>

class Iocp
{
public:
	Iocp(void);
	~Iocp(void);

	void				Err_Display(LPCSTR msg);
	void				Err_Quit(LPCSTR msg);

	int					GetClientCnt() const { return m_ClientCnt; }

	bool				Begin();
	bool				End();

	bool				ConfigSession();	
	void				CloseSocket(ClientInfo* pClientInfo, bool bIsForce = false);	

	bool				CreateIocp();						

	bool				CreateThreads();
	bool				CreateWorkerThread();				
	bool				CreateAccepterThread();	
	bool				CreatePacketProcThread();
	
	ClientInfo*			GetEmptyClientInfo();				// 사용하지 않는 클라이언트 정보 구조체를 반환

	bool				RegSockToIocp(ClientInfo* pClientInfo);							

	void				WorkerThread();			// 작업자 쓰레드
	void				AccepterThread();		// 접속을 받는 쓰레드
	void				PacketProcThread();

	void				DestroyThread();		// 생성되어 있는 쓰레드를 파괴한다.

	virtual bool		ConnectProcess(ClientInfo* pClientInfo) { return true; }
	virtual bool		DisConnectProcess(ClientInfo* pClientInfo) { return true; }
	virtual bool		PacketProcess(const unsigned char* pBuf, ClientInfo* pClientInfo) { return true; }

private:

	ClientInfo*			m_ClientInfo;			// 클라이언트 정보 저장 구조체
	SOCKET				m_socketListen;			// 클라이언트의 접속을 받기위한 Listen 소켓	
	int					m_ClientCnt;			// 접속되어 있는 클라이언트 수

	HANDLE				m_WorkerThread[MAX_WORKERTHREAD];	// 작업 쓰레드 핸들
	HANDLE				m_AccepterThread;		// 접속 쓰레드 핸들	
	HANDLE				m_IOCP;					// IOCP 핸들
	HANDLE				m_PacketThread[MAX_WORKERTHREAD];

	bool				m_WorkerRun;			// 작업 쓰레드 동작 플래그
	bool				m_AccepterRun;			// 접속 쓰레드 동작 플래그
	char				m_SocketBuf[1024];	// 소켓 버퍼

	queue<tuple<DWORD, ClientInfo*, LPOVERLAPPED>> que;
	mutex m_mutex;

protected:
	NetworkSession*		m_networkSession;
};