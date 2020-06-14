#include "Global.h"
#include "NumericalBaseBall.h"
#include <ctime>
#include <cstring>


NumericalBaseBall* g_BaseBallManager = new NumericalBaseBall();

void NumericalBaseBall::startConfig()
{
	int i = 0, randomNum = 0;
	int size = RANDNUM;
	int arr[] = { 0,1,2,3,4,5,6,7,8,9 };

	srand((unsigned int)time(NULL));  //시간마다 난수 다르게

	for (i = 0; i < INPUTSIZE; i++, size--)
	{
		randomNum = rand() % size;
		targetNum[i] = arr[randomNum];
		arr[randomNum] = arr[size - 1];

		cout << targetNum[i] << " ";
	}
	cout << endl;
}

void NumericalBaseBall::inputNumbers()
{
	for (int i = 0; i < INPUTSIZE; i++)
		cin >> inputNum[i];
}

bool NumericalBaseBall::compareNums(int&  sCnt, int& bCnt)
{
	int i = 0, j = 0;
	sCnt = 0, bCnt = 0;

	//스트라이크 , 볼 확인
	for (i = 0; i < INPUTSIZE; i++)
	{
		if (targetNum[i] == inputNum[i])
		{
			sCnt++;
			continue;
		}
		for (j = 0; j < INPUTSIZE; j++)
		{
			if (i != j && targetNum[i] == inputNum[j])
			{
				bCnt++;
				continue;
			}
		}
	}
	cout << "결과 [S : " << sCnt << "] , [B : " << bCnt << "]" << endl;

	//3S면 승리, 아니면 스트라이크&볼 갯수 초기화
	if (sCnt == 3) {
		return true;
	}

	return false;
}

void NumericalBaseBall::parseNumArr(int numbers)
{
	for (int i = (INPUTSIZE - 1); i >= 0; i--)
	{
		inputNum[i] = numbers % 10;
		numbers /= 10;
	}
}