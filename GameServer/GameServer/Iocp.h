#pragma once
#include "NetworkSession.h"
#include "Global.h"


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
	
	ClientInfo*			GetEmptyClientInfo();				// ������� �ʴ� Ŭ���̾�Ʈ ���� ����ü�� ��ȯ

	bool				RegSockToIocp(ClientInfo* pClientInfo);							

	void				WorkerThread();			// �۾��� ������
	void				AccepterThread();		// ������ �޴� ������

	void				DestroyThread();		// �����Ǿ� �ִ� �����带 �ı��Ѵ�.

	virtual bool		ConnectProcess(ClientInfo* pClientInfo) { return true; }
	virtual bool		DisConnectProcess(ClientInfo* pClientInfo) { return true; }
	virtual bool		PacketProcess(const unsigned char* pBuf, ClientInfo* pClientInfo) { return true; }

private:

	ClientInfo*			m_ClientInfo;			// Ŭ���̾�Ʈ ���� ���� ����ü
	SOCKET				m_socketListen;			// Ŭ���̾�Ʈ�� ������ �ޱ����� Listen ����	
	int					m_ClientCnt;			// ���ӵǾ� �ִ� Ŭ���̾�Ʈ ��

	HANDLE				m_WorkerThread[MAX_WORKERTHREAD];	// �۾� ������ �ڵ�
	HANDLE				m_AccepterThread;		// ���� ������ �ڵ�	
	HANDLE				m_IOCP;					// IOCP �ڵ�

	bool				m_WorkerRun;			// �۾� ������ ���� �÷���
	bool				m_AccepterRun;			// ���� ������ ���� �÷���
	char				m_SocketBuf[1024];	// ���� ����

protected:
	NetworkSession*		m_networkSession;
};