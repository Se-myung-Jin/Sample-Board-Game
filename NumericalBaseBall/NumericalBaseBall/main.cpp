#include "Global.h"
#include "NumericalBaseBall.h"


int main() 
{
	NumericalBaseBall sampleBaseBall;

	cout << "----숫자야구 시작----" << endl;
	sampleBaseBall.startConfig();


	while (1) 
	{
		cout << "\n\n숫자 입력" << endl;
		sampleBaseBall.inputNumbers();


		if (sampleBaseBall.compareNums()) 
		{
			cout << "\n\n----승리----" << endl;
			break;
		}
	}

	return 0;
}