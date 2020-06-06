#pragma once
class NumericalBaseBall
{
private:
	int targetNum[3]; // number of target
	int inputNum[3]; // number of input
	int sCnt; // 스트라이크 수
	int bCnt; // 볼 수

public:
	NumericalBaseBall() { sCnt = 0; bCnt = 0; };
	void startConfig(int numCnt = 3);
	void inputNumbers(int numCnt = 3); 
	bool compareNums(int numCnt = 3); 
};

