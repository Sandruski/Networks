#include "ModuleNetworkingClient.h"

bool ModuleNetworkingClient::start(const char* serverAddressStr, int serverPort, const char* pplayerName)
{
    playerName = pplayerName;

    // TODO(jesus): TCP connection stuff
    // - Create the socket
    // - Create the remote address object
    // - Connect to the remote address
    // - Add the created socket to the managed list of sockets using addSocket()
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        reportError("socket");
        return false;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    const char* toAddrStr = serverAddressStr;
    inet_pton(AF_INET, toAddrStr, &serverAddress.sin_addr);

    if (connect(s, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        reportError("connect");
        return false;
    }

    addSocket(s);

	m_messages.clear();

    // If everything was ok... change the state
    state = ClientState::Start;

    return true;
}

bool ModuleNetworkingClient::isRunning() const
{
    return state != ClientState::Stopped;
}

bool ModuleNetworkingClient::update()
{
    if (state == ClientState::Start) {

        OutputMemoryStream packet;
        packet << ClientMessage::Hello;
        packet << playerName;

        if (sendPacket(packet, s)) {
            state = ClientState::Logging;
        } else {
            disconnect();
            state = ClientState::Stopped;
        }
    }

    return true;
}

bool ModuleNetworkingClient::gui()
{
    if (state != ClientState::Stopped) {
        // NOTE(jesus): You can put ImGui code here for debugging purposes
        ImGui::Begin("Client Window");

        Texture* tex = App->modResources->client;
        ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
        ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("Hello %s! Welcome to the chat :)", playerName.c_str());
		if (ImGui::Button("Logout"))
		{
			disconnect();
			state = ClientState::Stopped;
		}

		ImGui::Spacing();
		
		ImGui::BeginChild("Scroll", ImVec2(400, 450), true);
		for (const auto& message : m_messages)
		{
			ImGui::TextColored(message.m_color, "%s", message.m_message.c_str());
		}
		ImGui::EndChild();

		static char text[256] = "";
		if (ImGui::InputText("Line", text, IM_ARRAYSIZE(text), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			OutputMemoryStream packet;
			packet << ClientMessage::Chat;
			packet << text;

			if (!sendPacket(packet, s))
			{
				disconnect();
				state = ClientState::Stopped;
			}

			strcpy_s(text, 256, "");
		}

        ImGui::End();
    }

    return true;
}

void ModuleNetworkingClient::onPacketReceived(SOCKET socket, const InputMemoryStream& packet)
{
	ServerMessage serverMessage;
	packet >> serverMessage;

	switch (serverMessage)
	{
	case ServerMessage::Welcome:
	{
		std::string message;
		ImVec4 color;
		packet >> message;
		packet >> color.x;
		packet >> color.y;
		packet >> color.z;
		packet >> color.w;

		packet >> m_playerColor.x;
		packet >> m_playerColor.y;
		packet >> m_playerColor.z;
		packet >> m_playerColor.w;

		m_messages.push_back(Message(message, color));

		break;
	}

	case ServerMessage::Chat:
	case ServerMessage::ClientConnected:
	case ServerMessage::ClientDisconnected:
	case ServerMessage::Help:
	case ServerMessage::List:
	case ServerMessage::Whisper:
	{
		std::string message;
		ImVec4 color;
		packet >> message;
		packet >> color.x;
		packet >> color.y;
		packet >> color.z;
		packet >> color.w;

		m_messages.push_back(Message(message, color));

		break;
	}

	case ServerMessage::Disconnect:
	{
		disconnect();
		state = ClientState::Stopped;

		break;
	}

	case ServerMessage::ChangeName:
	{
		packet >> playerName;

		break;
	}

	default:
	{
		break;
	}
	}
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
    state = ClientState::Stopped;
}
