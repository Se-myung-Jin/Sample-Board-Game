#pragma once

#define RANDNUM 10
#define INPUTSIZE 3

class NumericalBaseBall
{
private:
	int targetNum[3]; // number of target
	int inputNum[3]; // number of input
	int sCnt; // ��Ʈ����ũ ��
	int bCnt; // �� ��

public:
	NumericalBaseBall() { sCnt = 0; bCnt = 0; };
	void startConfig();
	void inputNumbers();
	bool compareNums(int& sCnt, int& bCnt);
	void parseNumArr(int numbers);
};