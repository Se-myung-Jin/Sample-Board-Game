#pragma once

#define WIN32_LEAN_AND_MEAN		// ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
#include <stdio.h>
#include <conio.h>
#include <tchar.h>



// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <queue>
#include <list>
#include <algorithm>

#pragma comment (lib, "ws2_32.lib")

#define MAX_SOCKBUF			256			// Recv�� ���� ���� ũ��
#define MAX_CLIENT			30			// �ִ� ������ ��
#define MAX_WORKERTHREAD	5			// �۾��� �������� ����
#define MAX_PACKET_SIZE		256
#define PORTNUM				5432

enum enumOperation
{
	OP_RECV,
	OP_SEND
};

using namespace std;

struct OverlappedEx
{
	WSAOVERLAPPED		m_wsaOverlapped;
	WSABUF				m_wsaBuf;
	unsigned char		m_IOCPbuf[MAX_PACKET_SIZE];		// IOCP send/recv ����
	unsigned char		m_packet_buf[MAX_PACKET_SIZE];	// recv�Ǵ� ��Ŷ�� �����Ǵ� ����
	queue <unsigned char *> m_SendPacketQueue;			// send�� ��Ŷ���� ����
	int					m_RemainLen;					// �̿Ϸ�� ��Ŷũ�⸦ ����
	enumOperation		m_Operation;
	bool				m_busySending;					// ���� IOCP���� ������ �������� ǥ��
	int					receiving_packet_size, received;	// ���� �������� ��Ŷ�� ũ��� ������ ����
};

// Ŭ���̾�Ʈ ������ ������� ����ü
struct ClientInfo
{
	SOCKET				m_socketClient;
	OverlappedEx		m_RecvOverlappedEx;		// RECV Overlapped I/O �۾��� ���� ����
	OverlappedEx		m_SendOverlappedEx;		// SEND Overlapped I/O �۾��� ���� ����

	// �����ڿ��� ��� �������� �ʱ�ȭ
	ClientInfo()
	{
		ZeroMemory(&m_RecvOverlappedEx.m_wsaOverlapped, sizeof(OVERLAPPED));
		ZeroMemory(&m_RecvOverlappedEx.m_wsaBuf, sizeof(WSABUF));
		m_RecvOverlappedEx.m_RemainLen = 0;
		m_RecvOverlappedEx.m_Operation = OP_RECV;
		m_RecvOverlappedEx.receiving_packet_size = 0;
		m_RecvOverlappedEx.received = 0;

		ZeroMemory(&m_SendOverlappedEx.m_wsaOverlapped, sizeof(OVERLAPPED));
		ZeroMemory(&m_SendOverlappedEx.m_wsaBuf, sizeof(WSABUF));
		m_SendOverlappedEx.m_RemainLen = 0;
		m_SendOverlappedEx.m_Operation = OP_SEND;
		m_SendOverlappedEx.m_busySending = false;

		m_socketClient = INVALID_SOCKET;
	}
};