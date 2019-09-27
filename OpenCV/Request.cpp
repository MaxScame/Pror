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
		
		return 0;
	}

	int request(const char* hostname, string message)
	{
		WSADATA WsaData;
		WSAStartup(0x0101, &WsaData);
		sockaddr_in       sin;
		int sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == -1) {
			return -100;
		}
		sin.sin_family = AF_INET;
		sin.sin_port = htons(100);

		struct hostent * host_addr = gethostbyname(hostname);

		sin.sin_addr.s_addr = *((int*)*host_addr->h_addr_list);

		connect(sock, (const struct sockaddr *)&sin, sizeof(sockaddr_in));
		
		int er = send(sock, "POST http://hack.ksu.ru.com:100/php/api/person/create.php HTTP/1.1\r\nHost: http://hack.ksu.ru.com\r\nReferer: http://hack.ksu.ru.com\r\nContent-Length: 190\r\n\r\nlogin=HMMMM&status=START&countMove=70&location=Cabinet1&creationDate=2019-09-27 18:35:59", strlen("POST http://hack.ksu.ru.com:100/php/api/person/create.php HTTP/1.1\r\nHost: http://hack.ksu.ru.com\r\nReferer: http://hack.ksu.ru.com\r\nContent-Length: 190\r\n\r\nlogin=HMMMM&status=START&countMove=70&location=Cabinet1&creationDate=2019-09-27 18:35:59"), 0);
		

		cout << "####HEADER####" << endl;
		char c1[1];
		int l, line_length;
		bool loop = true;
		bool bHeader = false;

		while (loop)
		{
			l = recv(sock, c1, 1, 0);
			if (l < 0) loop = false;
			if (c1[0] == '\n')
			{
				if (line_length == 0) loop = false;
				line_length = 0;
				if (message.find("200") != string::npos)
					bHeader = true;
			}
			else {
				if (c1[0] != '\r') {
					line_length++;
				}
			}
			cout << c1[0];
			message += c1[0];
		}

		message = "";
		if (bHeader)
		{
			cout << "####BODY####" << endl;
			char p[1024 * 10];
			l = recv(sock, p, 1024 * 10 - 1, 0);
			p[l] = '\0';
			message += p;
			cout << message.c_str();
		}
		else {
			return -102;
		}
		cout << "end";
		closesocket(sock);
		WSACleanup();

		return 0;
	}

}