#include "ModuleNetworkingServer.h"

//////////////////////////////////////////////////////////////////////
// ModuleNetworkingServer public methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::start(int port)
{
    // TODO(jesus): TCP listen socket stuff
    // - Create the listenSocket
    // - Set address reuse
    // - Bind the socket to a local interface
    // - Enter in listen mode
    // - Add the listenSocket to the managed list of sockets using addSocket()
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        reportError("socket");
        return false;
    }

    int enable = 1;
    if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(enable)) == SOCKET_ERROR) {
        reportError("setsockopt");
        return false;
    }

    sockaddr_in localAddr;
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(port);
    localAddr.sin_addr.S_un.S_addr = INADDR_ANY;
    if (bind(listenSocket, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR) {
        reportError("bind");
        return false;
    }

    int simultaneousConnections = 5;
    if (listen(listenSocket, simultaneousConnections) == SOCKET_ERROR) {
        reportError("listen");
        return false;
    }

    addSocket(listenSocket);

    state = ServerState::Listening;

    return true;
}

bool ModuleNetworkingServer::isRunning() const
{
    return state != ServerState::Stopped;
}

//////////////////////////////////////////////////////////////////////
// Module virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::update()
{
    return true;
}

bool ModuleNetworkingServer::gui()
{
    if (state != ServerState::Stopped) {
        // NOTE(jesus): You can put ImGui code here for debugging purposes
        ImGui::Begin("Server Window");

        Texture* tex = App->modResources->server;
        ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
        ImGui::Image(tex->shaderResource, texSize);

        ImGui::Text("List of connected sockets:");

        for (auto& connectedSocket : connectedSockets) {
            ImGui::Separator();
            ImGui::Text("Socket ID: %d", connectedSocket.socket);
            ImGui::Text("Address: %d.%d.%d.%d:%d",
                connectedSocket.address.sin_addr.S_un.S_un_b.s_b1,
                connectedSocket.address.sin_addr.S_un.S_un_b.s_b2,
                connectedSocket.address.sin_addr.S_un.S_un_b.s_b3,
                connectedSocket.address.sin_addr.S_un.S_un_b.s_b4,
                ntohs(connectedSocket.address.sin_port));
            ImGui::Text("Player name: %s", connectedSocket.playerName.c_str());
        }

        ImGui::End();
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::isListenSocket(SOCKET socket) const
{
    return socket == listenSocket;
}

void ModuleNetworkingServer::onSocketConnected(SOCKET socket, const sockaddr_in& socketAddress)
{
    // Add a new connected socket to the list
    ConnectedSocket connectedSocket;
    connectedSocket.socket = socket;
    connectedSocket.address = socketAddress;
    connectedSockets.push_back(connectedSocket);
}

void ModuleNetworkingServer::onPacketReceived(SOCKET socket, const InputMemoryStream& packet)
{
    ClientMessage clientMessage;
    packet >> clientMessage;

    switch (clientMessage)
	{
	case ClientMessage::Hello:
	{
		std::string playerName;
		packet >> playerName;

		for (auto& connectedSocket : connectedSockets) {

			OutputMemoryStream packet;

			if (connectedSocket.socket == socket) {

				connectedSocket.playerName = playerName;

				packet << ServerMessage::Welcome;
				packet << "********************\nWELCOME TO THE CHAT\n********************\nType /help to see available commands";
				packet << 1.0f;
				packet << 1.0f;
				packet << 0.0f;
				packet << 1.0f;

				connectedSocket.m_playerColor.x = (double)rand() / (RAND_MAX);
				connectedSocket.m_playerColor.y = (double)rand() / (RAND_MAX);
				connectedSocket.m_playerColor.z = (double)rand() / (RAND_MAX);
				connectedSocket.m_playerColor.w = 1.0f;

				packet << connectedSocket.m_playerColor.x;
				packet << connectedSocket.m_playerColor.y;
				packet << connectedSocket.m_playerColor.z;
				packet << connectedSocket.m_playerColor.w;
			}
			else
			{
				std::string message = "********** " + playerName + " joined **********";
				packet << ServerMessage::ClientConnected;
				packet << message;
				packet << 0.0f;
				packet << 1.0f;
				packet << 0.0f;
				packet << 1.0f;
			}

			if (!sendPacket(packet, connectedSocket.socket))
			{
				disconnect();
				state = ServerState::Stopped;

				break;
			}
		}

		break;
	}

	case ClientMessage::Chat:
	{
		std::string message;
		packet >> message;

		if (message == "/help")
		{
			OutputMemoryStream packet;
			packet << ServerMessage::Help;
			packet << "***** Commands list *****\n/help\n/kick [username]\n/list\n/whisper [username] [message]\n/change_name [username]";
			packet << 1.0f;
			packet << 1.0f;
			packet << 0.0f;
			packet << 1.0f;

			if (!sendPacket(packet, socket))
			{
				disconnect();
				state = ServerState::Stopped;
			}
		}
		else if (message == "/list")
		{
			std::string list = "***** Users list *****";

			for (const auto& connectedSocket : connectedSockets) 
			{
				list += "\n- " + connectedSocket.playerName;
			}

			OutputMemoryStream packet;
			packet << ServerMessage::List;
			packet << list;
			packet << 1.0f;
			packet << 1.0f;
			packet << 0.0f;
			packet << 1.0f;

			if (!sendPacket(packet, socket))
			{
				disconnect();
				state = ServerState::Stopped;
			}
		}
		else if (message.find("/kick") != std::string::npos)
		{
			std::string command = "/kick ";
			if (message.length() <= command.length())
			{
				break;
			}
			std::string playerName = message.substr(command.length());

			for (const auto& connectedSocket : connectedSockets) {

				if (connectedSocket.playerName == playerName) {

					OutputMemoryStream packet;
					packet << ServerMessage::Disconnect;

					if (!sendPacket(packet, connectedSocket.socket))
					{
						disconnect();
						state = ServerState::Stopped;
					}

					break;
				}
			}
		}
		else if (message.find("/whisper") != std::string::npos)
		{
			std::string command = "/whisper ";
			if (message.length() <= command.length())
			{
				break;
			}
			std::string args = message.substr(command.length());

			std::string spacing = " ";
			std::size_t spacingIndex = args.find(spacing);
			std::string toPlayerName = args.substr(0, spacingIndex);
			std::string sentence = args.substr(spacingIndex + spacing.length());

			ConnectedSocket fromConnectedSocket;
			for (const auto& connectedSocket : connectedSockets) 
			{
				if (connectedSocket.socket == socket)
				{
					fromConnectedSocket = connectedSocket;
					break;
				}
			}

			for (const auto& connectedSocket : connectedSockets) 
			{
				if (connectedSocket.playerName == toPlayerName)
				{
					OutputMemoryStream packet;
					packet << ServerMessage::Whisper;
					packet << fromConnectedSocket.playerName + " whispers to " + toPlayerName + ": " + sentence;

					if (!sendPacket(packet, connectedSocket.socket)
						|| !sendPacket(packet, fromConnectedSocket.socket))
					{
						disconnect();
						state = ServerState::Stopped;
					}

					break;
				}
			}
		}
		else if (message.find("/change_name") != std::string::npos)
		{
			std::string command = "/change_name ";
			if (message.length() <= command.length())
			{
				break;
			}
			std::string playerName = message.substr(command.length());

			for (auto& connectedSocket : connectedSockets)
			{
				if (connectedSocket.socket == socket)
				{
					connectedSocket.playerName = playerName;

					OutputMemoryStream packet;
					packet << ServerMessage::ChangeName;
					packet << playerName;

					if (!sendPacket(packet, connectedSocket.socket))
					{
						disconnect();
						state = ServerState::Stopped;
					}

					break;
				}
			}
		}
		else
		{
			ConnectedSocket fromConnectedSocket;
			for (const auto& connectedSocket : connectedSockets) {

				if (connectedSocket.socket == socket) {
					fromConnectedSocket = connectedSocket;
					break;
				}
			}

			for (const auto& connectedSocket : connectedSockets) {

				OutputMemoryStream packet;
				packet << ServerMessage::Chat;
				packet << fromConnectedSocket.playerName + ": " + message;
				packet << fromConnectedSocket.m_playerColor.x;
				packet << fromConnectedSocket.m_playerColor.y;
				packet << fromConnectedSocket.m_playerColor.z;
				packet << fromConnectedSocket.m_playerColor.w;

				if (!sendPacket(packet, connectedSocket.socket))
				{
					disconnect();
					state = ServerState::Stopped;

					break;
				}
			}
		}

		break;
	}

	default:
	{
		break;
	}
    }
}

void ModuleNetworkingServer::onSocketDisconnected(SOCKET socket)
{
	// Remove the connected socket from the list
	for (auto it = connectedSockets.begin(); it != connectedSockets.end(); ++it) {

		if ((*it).socket == socket) {

			std::string playerName = (*it).playerName;
			connectedSockets.erase(it);

			for (const auto& connectedSocket : connectedSockets) {

				OutputMemoryStream packet;
				std::string message = "********** " + playerName + " left **********";
				packet << ServerMessage::ClientDisconnected;
				packet << message;
				packet << 0.5f;
				packet << 0.5f;
				packet << 0.5f;
				packet << 1.0f;

				if (!sendPacket(packet, connectedSocket.socket))
				{
					disconnect();
					state = ServerState::Stopped;
				}
			}

			break;
		}
	}
}
