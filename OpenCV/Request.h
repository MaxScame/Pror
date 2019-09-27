#pragma once
#include <iostream>
#include <windows.h>
#include <Lmcons.h>
#include <assert.h>
#include <Windows.h>
#include <wininet.h>
#pragma comment(lib,"wininet")

using namespace std;

string UserName();

namespace POST
{
	int POST(string& login, string status, string countMove, string location, string Date/*, string& Pulse*/);
}


//bool POST(string& login, string status, string countMove, string location, string Date/*, string& Pulse*/);