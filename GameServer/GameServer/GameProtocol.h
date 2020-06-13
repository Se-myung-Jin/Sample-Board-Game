#pragma once
#include "Global.h"

enum PROTOCOL_TYPE
{
	REQ_GAMESTART,
	ACK_GAMESTART,
	REQ_CHAT,
	ACK_CHAT,
	REQ_BASEBALL_NUMBER,
	ACK_BASEBALL_NUMBER,
	REQ_BASEBALL_RESULT,
	ACK_BASEBALL_RESULT,
};

class Packet
{
public:
	BYTE packType;
	BYTE packSize;
	char* sendBuf;
};

class GameProtocol
{
public:
	virtual Packet* toPacket() = 0;
	virtual void toProtocol(const unsigned char* recvBuf) = 0;
};

class GameStartProtocol : GameProtocol
{
public:
	char* data;

	Packet* toPacket();
	void toProtocol(const unsigned char* recvBuf);
};

class ChatProtocol : GameProtocol
{
public:
	char* data;

	Packet* toPacket();
	void toProtocol(const unsigned char* recvBuf);
};

class BaseBallNumberProtocol : GameProtocol
{
public:
	int num;

	Packet* toPacket();
	void toProtocol(const unsigned char* recvBuf);
};

class BaseBallResultProtocol : GameProtocol
{
public:
	int strikeCnt;
	int ballCnt;

	Packet* toPacket();
	void toProtocol(const unsigned char* recvBuf) {};
};