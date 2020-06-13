#pragma once

#define WIN32_LEAN_AND_MEAN		// 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#include <stdio.h>
#include <conio.h>
#include <tchar.h>



// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <queue>
#include <list>
#include <algorithm>

#pragma comment (lib, "ws2_32.lib")

#define MAX_SOCKBUF			256			// Recv를 위한 버퍼 크기
#define MAX_CLIENT			30			// 최대 접속자 수
#define MAX_WORKERTHREAD	5			// 작업자 쓰레드의 갯수
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
	unsigned char		m_IOCPbuf[MAX_PACKET_SIZE];		// IOCP send/recv 버퍼
	unsigned char		m_packet_buf[MAX_PACKET_SIZE];	// recv되는 패킷이 조립되는 버퍼
	queue <unsigned char *> m_SendPacketQueue;			// send할 패킷들을 저장
	int					m_RemainLen;					// 미완료된 패킷크기를 저장
	enumOperation		m_Operation;
	bool				m_busySending;					// 현재 IOCP에서 보내는 중인지를 표시
	int					receiving_packet_size, received;	// 지금 조립중인 패킷의 크기와 받은양 저장
};

// 클라이언트 정보를 담기위한 구조체
struct ClientInfo
{
	SOCKET				m_socketClient;
	OverlappedEx		m_RecvOverlappedEx;		// RECV Overlapped I/O 작업을 위한 변수
	OverlappedEx		m_SendOverlappedEx;		// SEND Overlapped I/O 작업을 위한 변수

	// 생성자에서 멤버 변수들을 초기화
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