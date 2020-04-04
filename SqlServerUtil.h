#pragma once

#include<Windows.h>
#include<string>
#include<vector>
#include<fstream>

using namespace std;

class SqlServerUtil {
public:

	static std::string Replace(std::string& s, const std::string& toReplace, const std::string& replaceWith)
	{
		int pos;
		while ((pos = s.find(toReplace)) != std::string::npos)
			s.replace(pos, toReplace.length(), replaceWith);
		return s;
	}

	static string  ReadFile(const char * path)
	{
		std::ifstream t(path);
		return string((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());
	}

	static void WriteFile(const char * path, std::string& content)
	{
		std::ofstream out(path);
		out << content;
		out.close();
	}

	static std::vector<std::string> Split(std::string stringToBeSplitted, std::string delimeter)
	{
		std::vector<std::string> splittedString;
		int startIndex = 0;
		int  endIndex = 0;
		while ((endIndex = stringToBeSplitted.find(delimeter, startIndex)) < stringToBeSplitted.size())
		{
			std::string val = stringToBeSplitted.substr(startIndex, endIndex - startIndex);
			splittedString.push_back(val);
			startIndex = endIndex + delimeter.size();
		}
		if (startIndex < stringToBeSplitted.size())
		{
			std::string val = stringToBeSplitted.substr(startIndex);
			splittedString.push_back(val);
		}
		return splittedString;
	}

	//wstring高字节不为0，返回FALSE
	static bool WStringToString(const std::wstring wstr, std::string &str)
	{
#ifdef WIN32
		LPCWSTR ws = wstr.c_str();
		int wsLen = WideCharToMultiByte(CP_ACP, 0, ws, -1, NULL, 0, NULL, NULL);
		char* dst = new char[wsLen];
		int nResult = WideCharToMultiByte(CP_ACP, 0, ws, -1, dst, wsLen, NULL, NULL);
		if (nResult == 0)
		{
			delete[] dst;
			return false;
		}

		dst[wsLen - 1] = 0;
		str = dst;
		delete[] dst;
#endif
		return true;
	}

	static std::string AsciiCharPtrToUtf8(const char* pszOutBuffer)
	{
		DWORD dwNum = MultiByteToWideChar(CP_ACP, 0, pszOutBuffer, -1, NULL, 0);    // 返回原始ASCII码的字符数目       
		wchar_t* pwText = new wchar_t[dwNum];                                       // 根据ASCII码的字符数分配UTF8的空间
		ZeroMemory(pwText, 0, nLen * 2 + 2);

		MultiByteToWideChar(CP_UTF8, 0, pszOutBuffer, -1, pwText, dwNum);           // 将ASCII码转换成UTF8

		std::string retStr;

		WStringToString(pwText, retStr);

		delete[] pwText;
		pwText = NULL;

		return retStr;
	}
};