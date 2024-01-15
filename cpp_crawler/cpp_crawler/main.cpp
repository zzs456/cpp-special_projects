#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <vector>
#include <sstream>
#include <filesystem>
#include <Windows.h>
// ���Ӿ�̬��
#pragma comment(lib,"Urlmon.lib")

// ����ַ
const std::string web_address = "http://www.netbian.com/";
// �洢��ҳ������
const std::string file_name = "./test.txt";
// �洢ͼƬ��ת��ҳ������
const std::string temp_file = "./temp.txt";
// ͼƬ��Դ�Ĵ洢λ��
const std::string res_file = "./res";
// Ҫ��ȡ����ҳ��
const int pages = 4;

void Process();
bool Down_webContexts(const std::string& file_name,const std::string &url);
bool Parse_File(const std::string& file_name,const int index,std::vector<std::string> &vs,const std::string &pattern);

int main() {

	//std::regex r("<li><a href=\"(.*?)\".*?<img.*?</li>");
	//std::string s = "<li><a href=\"/desk/33010.htm\"title=\"�䶯Ǭ��������ֽ\" target=\"_blank\"><img src=\"http://img.netbian.com/file/2024/0113/small002431PhSQM1705076671.jpg\" alt=\"�䶯Ǭ��������ֽ\" /><b>�䶯Ǭ��������ֽ</b></a></li>";
	//std::cout << std::regex_match(s,r);

	Process();
	return 0;
}

// �ܵ�ִ�й���
void Process()
{
	std::vector<std::string> vs;
	std::string temp_url = web_address;
	// std::ifstream ifs(res_file);
	if (std::filesystem::exists(res_file)) {
		std::filesystem::remove_all(res_file);
	}
	// ���� res �ļ���
	std::filesystem::create_directory(res_file);
	for (int i = 1; i <= pages; ++i) {
		if (i != 1) {
			temp_url = web_address + "index_" + std::to_string(i) + ".htm";
		}
		std::cout << "�� " << i << " ҳ��ַ��" << temp_url << std::endl;
		bool ret = Down_webContexts(file_name, temp_url);
		if (!ret) {
			std::cout << "�� " << i << " ҳ����ʧ��!" << std::endl;
			break;
		}
		ret = Parse_File(file_name,i,vs,"<li><a href=\"(.*?)\".*?<img.*?</li>");
		if (!ret) {
			std::cout << "�� " << i << " ҳ���ݽ���ʧ��!" << std::endl;
		}
		std::string index_resFile = res_file + "/" + std::to_string(i);
		std::filesystem::create_directory(index_resFile);
		std::vector<std::string> imgs;
		int index = 1;
		//std::cout << vs.size() << std::endl;
		for (const auto &elem:vs) {
			// �������
			imgs.clear();
			std::string img_url = web_address + elem;
			bool ans = Down_webContexts(temp_file,img_url);
			if (!ans) {
				std::cout << "�� " << i << " ҳ��ͼƬ��Ӧ��������ҳ����ʧ��!" << std::endl;
				break;
			}
			ans = Parse_File(temp_file,i,imgs, "<div class=\"pic\">.*?<img src=\"(.*?)\".*?</p>");
			if (!ans) {
				std::cout << "�� " << i << " ҳ��ͼƬ��Ӧ��������ҳ���ݽ���ʧ��!" << std::endl;
				break;
			}
			// ����ͼƬ
			std::string img_file = res_file;
			img_file = img_file + "/" + std::to_string(i) + "/" + std::to_string(index) + ".jpg";
			// img_file = img_file + std::to_string(index) + ".jpg";
			// std::cout << img_file << std::endl;
			ans = Down_webContexts(img_file,imgs[0]);
			if (!ans) {
				std::cout << "ͼƬ����ʧ��!" << std::endl;
			}
			DeleteFileA(temp_file.c_str());
			Sleep(1000);	// ��Ϣ 1s
			index++;
		}
		DeleteFileA(file_name.c_str());
	}
}

/// <summary>
/// ������ҳ���ݵ��ı��ļ���
/// </summary>
/// <param name="file_name"> �����ļ���λ�� </param>
/// <param name="url"> ��ַ </param>
/// <returns></returns>
bool Down_webContexts(const std::string& file_name, const std::string& url)
{
	// ����
	HRESULT ret = URLDownloadToFileA(NULL, url.c_str(), file_name.c_str(), 0, NULL);
	return ret == S_OK;
}

/// <summary>
/// �����ļ�������
/// </summary>
/// <param name="file_name"> Ҫ�������ļ� </param>
/// <param name="index"> ��ǰ��Ӧ��ҳ�� </param>
/// <param name="vs"> �����õ��ĵ�ַ </param>
/// <param name="pattern"> ƥ��ģʽ </param>
/// <returns> �Ƿ�����ɹ� </returns>
bool Parse_File(const std::string& file_name,const int index, std::vector<std::string>& vs, const std::string& pattern)
{
	vs.clear();	// �������
	std::ifstream ifs(file_name,std::ios::in);
	if (!ifs.is_open()) {
		std::cout << "�ļ���ʧ��!" << std::endl;
		return false;
	}
	// ��ȡ�ļ�����������
	std::stringstream buf;
	buf << ifs.rdbuf();
	std::string data = buf.str();
	// ƥ��ģʽ
	std::regex r_pattern(pattern);
	// ����ƥ����
	std::smatch result;
	std::string::const_iterator begin = data.begin();	//��ȡ�ı���ʼ�ĵ�����
	std::string::const_iterator end = data.end();		//��ȡ�ı������ĵ�����
	//ƥ��ɹ�����true,������һ��ƥ��.ʧ�����˳�ѭ��
	while (std::regex_search(begin, end, result, r_pattern)) { 
		vs.push_back(result[1]);	// ��ȡ��һ�������飬��ͼƬ�����ӵ�ַ
		begin = result[0].second;	// ����ƥ��Ŀ�ʼλ��
	}
	ifs.close();
	return true;
}