#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"
#include "WinSock2.h"
#include "Ws2tcpip.h"

#include <iostream>
#include <cstdlib>

#pragma comment(lib, "ws2_32.lib")

#define LISTEN_PORT 8888

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

	// TODO-2: Create socket (IPv4, stream, TCP)
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET)
	{
		printWSErrorAndExit("socket");
		return;
	}

	// TODO-3: Configure socket for address reuse
	int enable = 1;
	iResult = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(enable));	if (iResult == SOCKET_ERROR)
	{
		printWSErrorAndExit("setsockopt");
		return;
	}

	// TODO-4: Create an address object with any local address
	sockaddr_in localAddr;
	localAddr.sin_family = AF_INET; // IPv4
	localAddr.sin_port = htons(port); // Port
	localAddr.sin_addr.S_un.S_addr = INADDR_ANY; // Any local IP address
	// TODO-5: Bind socket to the local address
	iResult = bind(s, (sockaddr*)&localAddr, sizeof(localAddr));
	if (iResult == SOCKET_ERROR)
	{
		printWSErrorAndExit("bind");
		return;
	}

	// TODO-6: Make the socket enter into listen mode
	int simultaneousConnections = 1;
	iResult = listen(s, simultaneousConnections);
	if (iResult == SOCKET_ERROR)
	{
		printWSErrorAndExit("listen");
		return;
	}

	// TODO-7: Accept a new incoming connection from a remote host
	// Note that once a new connection is accepted, we will have
	// a new socket directly connected to the remote host.
	sockaddr_in remoteAddr;
	int remoteAddrSize = sizeof(remoteAddr);
	SOCKET connectedSocket = accept(s, (sockaddr*)&remoteAddr, &remoteAddrSize);
	if (connectedSocket == INVALID_SOCKET)
	{
		printWSErrorAndExit("accept");
		return;
	}

	char buf[5];
	int bufSize = 5 * sizeof(char);
	while (true)
	{
		// TODO-8:
		// - Wait a 'ping' packet from the client
		// - Send a 'pong' packet to the client
		// - Control errors in both cases
		iResult = recv(connectedSocket, buf, bufSize, 0);
		if (iResult == SOCKET_ERROR)
		{
			printWSErrorAndExit("recv");
			break;
		}
		else if (iResult == 0)
		{
			break;
		}

		std::cout << buf << std::endl;

		memcpy(&buf, "pong", bufSize);

		iResult = send(connectedSocket, buf, bufSize, 0);
		if (iResult == SOCKET_ERROR)
		{
			printWSErrorAndExit("send");
			break;
		}
	}

	// TODO-9: Close socket
	iResult = closesocket(s);
	if (iResult == SOCKET_ERROR)
	{
		printWSErrorAndExit("closesocket");
		return;
	}

	// TODO-10: Winsock shutdown
	iResult = WSACleanup();
	if (iResult == SOCKET_ERROR)
	{
		printWSErrorAndExit("WSACleanup");
		return;
	}
}

int main(int argc, char **argv)
{
	server(LISTEN_PORT);

	PAUSE_AND_EXIT();
}
