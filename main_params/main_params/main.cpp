#include <iostream>
#include <Windows.h>
using std::cout;
using std::endl;

void test_func(char*,char*);

int main(int argc,char **argv) {
	
	switch (argc) {
	// 没有额外参数
	case 1:
		cout << "请输入参数!" << endl;
		break;
	case 2:
		cout << "参数不足!" << endl;
		break;
	case 3:
		test_func(argv[1],argv[2]);
		break;
	default:
		cout << "参数过多!" << endl;
		break;
	}

	return 0;
}

void test_func(char* target, char* out_string) {
	// 如果 target 不是 attack, 则直接返回
	if (strcmp(target,"attack")) {
		cout << "不存在该命令!" << endl;
		return;
	}
	for (int i = 0; i < 5;++i) {
		cout << "Attacking " << out_string << " ";
		for (int j = 0; j <= i;++j) {
			cout << ".";
		}
		cout << endl;
		// 程序休眠
		Sleep(500);
	}
	cout << "Successful attack!" << endl;
}