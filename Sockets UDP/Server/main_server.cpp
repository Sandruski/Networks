#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"
#include "WinSock2.h"
#include "Ws2tcpip.h"

#include <iostream>
#include <cstdlib>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 8888

#define PAUSE_AND_EXIT() system("pause"); exit(-1)

void printWSErrorAndExit(const char *msg)
{
	wchar_t *s = NULL;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&s, 0, NULL);
	fprintf(stderr, "%s: %S\n", msg, s);
	LocalFree(s);
	PAUSE_AND_EXIT();
}

void server(int port)
{
	// TODO-1: Winsock init
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult == SOCKET_ERROR)
	{
		// Log and handle error
		printWSErrorAndExit("WSAStartup");
		return;
	}

	// TODO-2: Create socket (IPv4, datagrams, UDP)
	SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == INVALID_SOCKET)
	{
		printWSErrorAndExit("socket");
		return;
	}

	// TODO-3: Force address reuse
	sockaddr_in localAddr;
	localAddr.sin_family = AF_INET; // IPv4
	localAddr.sin_port = htons(port); // Port
	localAddr.sin_addr.S_un.S_addr = INADDR_ANY; // Any local IP address
	int enable = 1;
	iResult = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int));	if (iResult == SOCKET_ERROR)
	{
		printWSErrorAndExit("setsockopt");
		return;
	}

	// TODO-4: Bind to a local address
	iResult = bind(s, (sockaddr*)&localAddr, sizeof(localAddr));
	if (iResult == SOCKET_ERROR)
	{
		printWSErrorAndExit("bind");
		return;
	}

	char buf[5];
	int bufSize = 5 * sizeof(char);
	while (true)
	{
		// TODO-5:
		// - Receive 'ping' packet from a remote host
		// - Answer with a 'pong' packet
		// - Control errors in both cases
		sockaddr_in fromAddr;
		int fromSize = sizeof(fromAddr);
		iResult = recvfrom(s, buf, bufSize, 0, (sockaddr*)&fromAddr, &fromSize);		if (iResult == SOCKET_ERROR)
		{
			printWSErrorAndExit("recvfrom");
			return;
		}

		std::cout << buf << std::endl;

		memcpy(&buf, "pong", bufSize);

		iResult = sendto(s, buf, bufSize, 0, (sockaddr*)&fromAddr, fromSize);
		if (iResult == SOCKET_ERROR)
		{
			printWSErrorAndExit("sendto");
			return;
		}
	}

	// TODO-6: Close socket
	iResult = closesocket(s);
	if (iResult == SOCKET_ERROR)
	{
		printWSErrorAndExit("closesocket");
		return;
	}

	// TODO-7: Winsock shutdown
	iResult = WSACleanup();
	if (iResult == SOCKET_ERROR)
	{
		printWSErrorAndExit("WSACleanup");
		return;
	}
}

int main(int argc, char **argv)
{
	server(SERVER_PORT);

	PAUSE_AND_EXIT();
}
