#include "Global.h"
#include "NumericalBaseBall.h"


int main() 
{
	NumericalBaseBall sampleBaseBall;

	cout << "----���ھ߱� ����----" << endl;
	sampleBaseBall.startConfig();


	while (1) 
	{
		cout << "\n\n���� �Է�" << endl;
		sampleBaseBall.inputNumbers();


		if (sampleBaseBall.compareNums()) 
		{
			cout << "\n\n----�¸�----" << endl;
			break;
		}
	}

	return 0;
}