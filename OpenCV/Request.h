#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <sstream>
#include <ctime>
#include <windows.h>
#include <Lmcons.h>
#include <assert.h>
#include <Windows.h>
//#include <wininet.h>
//#pragma comment(lib,"wininet")

//#include <winsock.h>
//#pragma comment(lib,"Wsock32.lib")

#include <stdlib.h>
#include <fstream>


using namespace std;

string UserName();

namespace POST
{
	void SEND_POST(string& Status, string& CountMove, string& Pulse, string& Location);
	/*int request(const char* hostname, string message);*/
	//bool PostURL(std::string url, std::string resource, std::string data);
}