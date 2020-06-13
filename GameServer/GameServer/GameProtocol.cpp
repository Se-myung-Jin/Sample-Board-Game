#include "GameProtocol.h"


Packet* GameStartProtocol::toPacket()
{
	Packet* pack = new Packet();
	pack->packType = ACK_GAMESTART;

	char cArr[] = "Game 방에 입장하였습니다. 유저들과 대화가 가능하며 지금부터 NumericalBaseBallGame을 시작합니다.";
	int len = strlen(cArr);

	BYTE headerSize = sizeof(Packet);
	BYTE sendBytes = (len + headerSize);

	char * buf = new char[sendBytes];
	ZeroMemory(buf, sizeof(char)*sendBytes);

	pack->packSize = sendBytes;

	memcpy(buf, &sendBytes, sizeof(BYTE));
	memcpy(buf + sizeof(BYTE), &pack->packType, sizeof(BYTE));
	memcpy(buf + sizeof(BYTE) + sizeof(BYTE), cArr, sizeof(char)*len);

	pack->sendBuf = buf;

	return pack;
}

void GameStartProtocol::toProtocol(const unsigned char* recvBuf)
{
	Packet* pack = new Packet();

	pack->packSize = recvBuf[0];
	pack->packType = recvBuf[1];
}

Packet* ChatProtocol::toPacket()
{
	Packet* pack = new Packet();
	pack->packType = ACK_CHAT;

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

void ChatProtocol::toProtocol(const unsigned char* recvBuf)
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
	pack->packType = ACK_BASEBALL_NUMBER;

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


void BaseBallNumberProtocol::toProtocol(const unsigned char* recvBuf)
{
	Packet* pack = new Packet();

	pack->packSize = recvBuf[0];
	pack->packType = recvBuf[1];

	memcpy(&num, recvBuf + sizeof(BYTE) * 2, sizeof(int));
}

Packet* BaseBallResultProtocol::toPacket()
{
	Packet* pack = new Packet();
	pack->packType = ACK_BASEBALL_RESULT;

	BYTE headerSize = sizeof(Packet);
	BYTE sendBytes = (headerSize + sizeof(int) * 2);
	char * buf = new char[sendBytes];
	ZeroMemory(buf, sizeof(char)*sendBytes);

	pack->packSize = sendBytes;

	memcpy(buf, &sendBytes, sizeof(BYTE));
	memcpy(buf + sizeof(BYTE), &pack->packType, sizeof(BYTE));
	memcpy(buf + sizeof(BYTE) + sizeof(BYTE), &strikeCnt, sizeof(int));
	memcpy(buf + sizeof(BYTE) + sizeof(BYTE) + sizeof(int), &ballCnt, sizeof(int));

	pack->sendBuf = buf;

	return pack;
}