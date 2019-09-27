#pragma once
//#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <windows.h>
#include <Lmcons.h>
#include <assert.h>
#include <Windows.h>
//#include <wininet.h>
//#pragma comment(lib,"wininet")

#include <winsock.h>
#include <sstream>
#pragma comment(lib,"Wsock32.lib")

#include <stdlib.h>
#include <fstream>


using namespace std;

string UserName();

namespace POST
{
	int POST(string& login, string status, string countMove, string location, string Date/*, string& Pulse*/);
	int request(const char* hostname, string message);
	//bool PostURL(std::string url, std::string resource, std::string data);
}


//bool POST(string& login, string status, string countMove, string location, string Date/*, string& Pulse*/);