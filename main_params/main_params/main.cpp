#include <iostream>
#include <Windows.h>
using std::cout;
using std::endl;

void test_func(char*,char*);

int main(int argc,char **argv) {
	
	switch (argc) {
	// û�ж������
	case 1:
		cout << "���������!" << endl;
		break;
	case 2:
		cout << "��������!" << endl;
		break;
	case 3:
		test_func(argv[1],argv[2]);
		break;
	default:
		cout << "��������!" << endl;
		break;
	}

	return 0;
}

void test_func(char* target, char* out_string) {
	// ��� target ���� attack, ��ֱ�ӷ���
	if (strcmp(target,"attack")) {
		cout << "�����ڸ�����!" << endl;
		return;
	}
	for (int i = 0; i < 5;++i) {
		cout << "Attacking " << out_string << " ";
		for (int j = 0; j <= i;++j) {
			cout << ".";
		}
		cout << endl;
		// ��������
		Sleep(500);
	}
	cout << "Successful attack!" << endl;
}