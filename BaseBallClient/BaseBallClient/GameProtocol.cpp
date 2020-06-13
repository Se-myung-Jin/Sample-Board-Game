#include "GameProtocol.h"


Packet* GameStartProtocol::toPacket()
{
	Packet* pack = new Packet();
	pack->packType = REQ_GAMESTART;

	BYTE headerSize = sizeof(Packet);

	char * buf = new char[headerSize];
	ZeroMemory(buf, sizeof(char)*headerSize);

	pack->packSize = headerSize;

	memcpy(buf, &headerSize, sizeof(BYTE));
	memcpy(buf + sizeof(BYTE), &pack->packType, sizeof(BYTE));

	pack->sendBuf = buf;

	return pack;
}

void GameStartProtocol::toProtocol(char* recvBuf)
{
	Packet* pack = new Packet();

	pack->packSize = recvBuf[0];
	pack->packType = recvBuf[1];

	int len = pack->packSize - (sizeof(BYTE) * 2);

	data = new char[len];
	memcpy(data, recvBuf + sizeof(BYTE) * 2, len);
}

Packet* ChatProtocol::toPacket()
{
	Packet* pack = new Packet();
	pack->packType = REQ_CHAT;

	int len = strlen(data);
	BYTE headerSize = sizeof(Packet);
	BYTE sendBytes = (len + headerSize);

	char * buf = new char[sendBytes];
	ZeroMemory(buf, sizeof(char)*sendBytes);

	pack->packSize = sendBytes;

	memcpy(buf, &sendBytes, sizeof(BYTE));
	memcpy(buf + sizeof(BYTE), &pack->packType, sizeof(BYTE));
	memcpy(buf + sizeof(BYTE) + sizeof(BYTE), data, sizeof(char)*len);

	pack->sendBuf = buf;

	return pack;
}

void ChatProtocol::toProtocol(char* recvBuf)
{
	Packet* pack = new Packet();

	pack->packSize = recvBuf[0];
	pack->packType = recvBuf[1];

	int len = pack->packSize - (sizeof(BYTE) * 2);

	data = new char[len];
	memcpy(data, recvBuf + sizeof(BYTE) * 2, len);
}

Packet* BaseBallNumberProtocol::toPacket()
{
	Packet* pack = new Packet();
	pack->packType = REQ_BASEBALL_NUMBER;

	BYTE headerSize = sizeof(Packet);
	BYTE sendBytes = (headerSize + sizeof(int));
	char * buf = new char[sendBytes];
	ZeroMemory(buf, sizeof(char)*sendBytes);

	pack->packSize = sendBytes;

	memcpy(buf, &sendBytes, sizeof(BYTE));
	memcpy(buf + sizeof(BYTE), &pack->packType, sizeof(BYTE));
	memcpy(buf + sizeof(BYTE) + sizeof(BYTE), &num, sizeof(int));

	pack->sendBuf = buf;

	return pack;
}


void BaseBallNumberProtocol::toProtocol(char* recvBuf)
{
	Packet* pack = new Packet();

	pack->packSize = recvBuf[0];
	pack->packType = recvBuf[1];

	memcpy(&num, recvBuf + sizeof(BYTE) * 2, sizeof(int));
}

void BaseBallNumberProtocol::divNum(char* msg)
{
	char* buf = new char[sizeof(int)];

	memcpy(buf, msg + (sizeof(char) * 4), sizeof(int));
	num = atoi(buf);
}

void BaseBallResultProtocol::toProtocol(char* recvBuf)
{
	Packet* pack = new Packet();
	
	pack->packSize = recvBuf[0];
	pack->packType = recvBuf[1];

	memcpy(&strikeCnt, recvBuf + sizeof(BYTE) * 2, sizeof(int));
	memcpy(&ballCnt, recvBuf + sizeof(BYTE) * 2 + sizeof(int), sizeof(int));
}