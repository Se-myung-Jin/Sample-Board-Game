#include "Global.h"
#include <string>

PROTOCOL_TYPE checkCommend(char* msg);

int main(int argc, char* argv[])
{
	CoInitialize(NULL);

	NetworkSession network;
	network.Connect();

	char msg[BUFSIZE];
	char* id;

	while (true) {
		fgets(msg, BUFSIZE, stdin);//입력을 받는다.


		GameStartProtocol startProc;
		ChatProtocol     chatProc;
		BaseBallNumberProtocol baseballProc;

		PROTOCOL_TYPE pType = checkCommend(msg);
		switch (pType)
		{
		case PROTOCOL_TYPE::REQ_GAMESTART:
			network.SendData(startProc.toPacket());
			break;
		case PROTOCOL_TYPE::REQ_BASEBALL_NUMBER:
			baseballProc.divNum(msg);
			network.SendData(baseballProc.toPacket());
			break;
		case PROTOCOL_TYPE::REQ_CHAT:
			chatProc.data = msg;
			network.SendData(chatProc.toPacket());
			break;
		default:
			break;
		}
	}
}

PROTOCOL_TYPE checkCommend(char* msg) 
{
	string str(msg);
	string bbStr = str.substr(0, 3);

	if (bbStr == "@GS")
		return PROTOCOL_TYPE::REQ_GAMESTART;

	if (bbStr == "@IO")
		return PROTOCOL_TYPE::REQ_BASEBALL_NUMBER;

	return PROTOCOL_TYPE::REQ_CHAT;
}