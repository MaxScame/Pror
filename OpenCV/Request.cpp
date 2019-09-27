#include "Request.h"

string UserName()
{
	char username[UNLEN + 1];
	DWORD username_len = UNLEN + 1;
	GetUserName(username, &username_len);
	string Name(username);
	return Name;
}

//bool POST(string& login, string status, string countMove, string location, string Date/*, string& Pulse*/)
//{
//	// http://hack.ksu.ru.com:100/php/api/person/create.php?login=TEST&status=START&countMove=70&location=Cabinet1&creationDate=2019-09-27 18:35:00
//	UrlRequest request;
//	string POST_REQ = "http://hack.ksu.ru.com:100/php/api/person/create.php?login=";
//	POST_REQ += login;
//	POST_REQ += "&status=";
//	POST_REQ += status;
//	POST_REQ += "&countMove=";
//	POST_REQ += countMove;
//	POST_REQ += "&location=";
//	POST_REQ += location;
//	POST_REQ += "&creationDate=";
//	POST_REQ += Date;
//	request.host(POST_REQ);
//	//request.addHeader("Content-Type: application/json");
//	auto response = std::move(request.perform());
//	if (response.statusCode() == 200)
//	{
//		cout << "status code = " << response.statusCode() << ", body = *" << response.body() << "*" << endl;
//		return true;
//	}
//	else
//	{
//		cout << "status code = " << response.statusCode() << ", description = " << response.statusDescription() << endl;
//		return false;
//	}
//	return false;
//}

namespace POST
{
	int POST(string& login, string status, string countMove, string location, string Date/*, string& Pulse*/)
	{
		string POST_REQ = "login=";
		POST_REQ += login;
		POST_REQ += "&status=";
		POST_REQ += status;
		POST_REQ += "&countMove=";
		POST_REQ += countMove;
		POST_REQ += "&location=";
		POST_REQ += location;
		POST_REQ += "&creationDate=";
		POST_REQ += Date;
		int post_lenght = POST_REQ.length();
		LPCSTR szAcceptBytes = "*/*";
		//char* szData = new char[post_lenght];
		char szData[] = "login=TEST&status=START&countMove=70&location=Cabinet1&creationDate=2019-09-27 18:35:00";
		strcpy_s(szData, post_lenght, POST_REQ.c_str());
		const char szHeaders[] = "Content-Type: application/x-www-form-urlencoded\r\n";
		DWORD read;
		string buffer(1024, '0');
		//__memset(buffer, 0, sizeof(buffer));
		HINTERNET hInet = InternetOpenA("Mozilla/4.0 (compatible; MSIE 6.0b; Windows NT 5.0; .NET CLR 1.0.2914)", INTERNET_OPEN_TYPE_PRECONFIG, "", "", 0);
		HINTERNET hSession = InternetConnectA(hInet, "hack.ksu.ru.com", 100, "", "", INTERNET_SERVICE_HTTP, 0, 1u);
		HINTERNET hRequest = HttpOpenRequestA(hSession, "POST", "/php/api/person/create.php", NULL, "hack.ksu.ru.com", 0, 0, 1);
		HttpSendRequestA(hRequest, szHeaders, sizeof(szHeaders) - 1, szData, sizeof(szData) - 1);
		InternetReadFile(hRequest, &buffer, sizeof(buffer), &read);
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hSession);
		InternetCloseHandle(hInet);
		return 0;
	}
}