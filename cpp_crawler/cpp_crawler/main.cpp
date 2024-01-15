#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <vector>
#include <sstream>
#include <filesystem>
#include <Windows.h>
// 链接静态库
#pragma comment(lib,"Urlmon.lib")

// 主网址
const std::string web_address = "http://www.netbian.com/";
// 存储网页的内容
const std::string file_name = "./test.txt";
// 存储图片跳转网页的内容
const std::string temp_file = "./temp.txt";
// 图片资源的存储位置
const std::string res_file = "./res";
// 要读取的网页数
const int pages = 4;

void Process();
bool Down_webContexts(const std::string& file_name,const std::string &url);
bool Parse_File(const std::string& file_name,const int index,std::vector<std::string> &vs,const std::string &pattern);

int main() {

	//std::regex r("<li><a href=\"(.*?)\".*?<img.*?</li>");
	//std::string s = "<li><a href=\"/desk/33010.htm\"title=\"武动乾坤绫清竹壁纸\" target=\"_blank\"><img src=\"http://img.netbian.com/file/2024/0113/small002431PhSQM1705076671.jpg\" alt=\"武动乾坤绫清竹壁纸\" /><b>武动乾坤绫清竹壁纸</b></a></li>";
	//std::cout << std::regex_match(s,r);

	Process();
	return 0;
}

// 总的执行过程
void Process()
{
	std::vector<std::string> vs;
	std::string temp_url = web_address;
	// std::ifstream ifs(res_file);
	if (std::filesystem::exists(res_file)) {
		std::filesystem::remove_all(res_file);
	}
	// 创建 res 文件夹
	std::filesystem::create_directory(res_file);
	for (int i = 1; i <= pages; ++i) {
		if (i != 1) {
			temp_url = web_address + "index_" + std::to_string(i) + ".htm";
		}
		std::cout << "第 " << i << " 页网址：" << temp_url << std::endl;
		bool ret = Down_webContexts(file_name, temp_url);
		if (!ret) {
			std::cout << "第 " << i << " 页下载失败!" << std::endl;
			break;
		}
		ret = Parse_File(file_name,i,vs,"<li><a href=\"(.*?)\".*?<img.*?</li>");
		if (!ret) {
			std::cout << "第 " << i << " 页内容解析失败!" << std::endl;
		}
		std::string index_resFile = res_file + "/" + std::to_string(i);
		std::filesystem::create_directory(index_resFile);
		std::vector<std::string> imgs;
		int index = 1;
		//std::cout << vs.size() << std::endl;
		for (const auto &elem:vs) {
			// 清空内容
			imgs.clear();
			std::string img_url = web_address + elem;
			bool ans = Down_webContexts(temp_file,img_url);
			if (!ans) {
				std::cout << "第 " << i << " 页有图片对应的链接网页下载失败!" << std::endl;
				break;
			}
			ans = Parse_File(temp_file,i,imgs, "<div class=\"pic\">.*?<img src=\"(.*?)\".*?</p>");
			if (!ans) {
				std::cout << "第 " << i << " 页有图片对应的链接网页内容解析失败!" << std::endl;
				break;
			}
			// 下载图片
			std::string img_file = res_file;
			img_file = img_file + "/" + std::to_string(i) + "/" + std::to_string(index) + ".jpg";
			// img_file = img_file + std::to_string(index) + ".jpg";
			// std::cout << img_file << std::endl;
			ans = Down_webContexts(img_file,imgs[0]);
			if (!ans) {
				std::cout << "图片下载失败!" << std::endl;
			}
			DeleteFileA(temp_file.c_str());
			Sleep(1000);	// 休息 1s
			index++;
		}
		DeleteFileA(file_name.c_str());
	}
}

/// <summary>
/// 下载网页内容到文本文件中
/// </summary>
/// <param name="file_name"> 保存文件的位置 </param>
/// <param name="url"> 网址 </param>
/// <returns></returns>
bool Down_webContexts(const std::string& file_name, const std::string& url)
{
	// 下载
	HRESULT ret = URLDownloadToFileA(NULL, url.c_str(), file_name.c_str(), 0, NULL);
	return ret == S_OK;
}

/// <summary>
/// 解析文件的内容
/// </summary>
/// <param name="file_name"> 要解析的文件 </param>
/// <param name="index"> 当前对应的页数 </param>
/// <param name="vs"> 解析得到的地址 </param>
/// <param name="pattern"> 匹配模式 </param>
/// <returns> 是否解析成功 </returns>
bool Parse_File(const std::string& file_name,const int index, std::vector<std::string>& vs, const std::string& pattern)
{
	vs.clear();	// 清楚内容
	std::ifstream ifs(file_name,std::ios::in);
	if (!ifs.is_open()) {
		std::cout << "文件打开失败!" << std::endl;
		return false;
	}
	// 获取文件的所有内容
	std::stringstream buf;
	buf << ifs.rdbuf();
	std::string data = buf.str();
	// 匹配模式
	std::regex r_pattern(pattern);
	// 保存匹配结果
	std::smatch result;
	std::string::const_iterator begin = data.begin();	//获取文本开始的迭代器
	std::string::const_iterator end = data.end();		//获取文本结束的迭代器
	//匹配成功返回true,继续下一次匹配.失败则退出循环
	while (std::regex_search(begin, end, result, r_pattern)) { 
		vs.push_back(result[1]);	// 获取第一个捕获组，即图片的链接地址
		begin = result[0].second;	// 更新匹配的开始位置
	}
	ifs.close();
	return true;
}