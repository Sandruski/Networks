#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"
#include "WinSock2.h"
#include "Ws2tcpip.h"

#include <iostream>
#include <cstdlib>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_ADDRESS "127.0.0.1"

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

void client(const char *serverAddrStr, int port)
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

	// TODO-3: Create an address object with the server address
	sockaddr_in toAddr;
	toAddr.sin_family = AF_INET; // IPv4
	toAddr.sin_port = htons(port); // Port
	const char *toAddrStr = serverAddrStr; // Not so remote… :-P
	inet_pton(AF_INET, toAddrStr, &toAddr.sin_addr);

	char buf[5];
	int bufSize = 5 * sizeof(char);
	while (true)
	{
		// TODO-4:
		// - Send a 'ping' packet to the server
		// - Receive 'pong' packet from the server
		// - Control errors in both cases
		memcpy(&buf, "ping", bufSize);

		iResult = sendto(s, buf, bufSize, 0, (sockaddr*)&toAddr, sizeof(toAddr));
		if (iResult == SOCKET_ERROR)
		{
			printWSErrorAndExit("sendto");
			break;
		}

		sockaddr_in fromAddr;
		int fromSize = sizeof(fromAddr);
		iResult = recvfrom(s, buf, bufSize, 0, (sockaddr*)&fromAddr, &fromSize);		if (iResult == SOCKET_ERROR)
		{
			printWSErrorAndExit("recvfrom");
			break;
		}

		std::cout << buf << std::endl;

		Sleep(500);
	}

	// TODO-5: Close socket
	iResult = closesocket(s);
	if (iResult == SOCKET_ERROR)
	{
		printWSErrorAndExit("closesocket");
		return;
	}

	// TODO-6: Winsock shutdown
	iResult = WSACleanup();
	if (iResult == SOCKET_ERROR)
	{
		printWSErrorAndExit("WSACleanup");
		return;
	}
}

int main(int argc, char **argv)
{
	client(SERVER_ADDRESS, SERVER_PORT);

	PAUSE_AND_EXIT();
}
