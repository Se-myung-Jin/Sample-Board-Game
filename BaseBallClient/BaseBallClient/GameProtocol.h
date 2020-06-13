#pragma once

#include "Global.h"

#define BUFSIZE 256
#define PORTNUM 5432

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
	virtual void toProtocol(char* recvBuf) = 0;
};

class GameStartProtocol : GameProtocol
{
public:
	char* data;

	Packet* toPacket();
	void toProtocol(char* recvBuf);
};

class ChatProtocol : GameProtocol
{
public:
	char* data;

	Packet* toPacket();
	void toProtocol(char* recvBuf);
};

class BaseBallNumberProtocol : GameProtocol
{
public:
	int num;

	Packet* toPacket();
	void toProtocol(char* recvBuf);
	void divNum(char* msg);
};

class BaseBallResultProtocol : GameProtocol
{
public:
	int strikeCnt;
	int ballCnt;

	Packet* toPacket() 
	{ 
		Packet* pack = new Packet();
		return pack;
	}
	void toProtocol(char* recvBuf);
};