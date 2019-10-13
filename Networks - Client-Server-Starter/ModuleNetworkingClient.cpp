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
			m_messages.clear();

			disconnect();
			state = ClientState::Stopped;
		}
		ImGui::Separator();

		for (const auto& message : m_messages)
		{
			ImGui::TextColored(message.m_color, "%s", message.m_message.c_str());
		}

		static char text[128] = "";
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

			strcpy_s(text, 128, "");
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
	case ServerMessage::ClientConnected:
	case ServerMessage::ClientDisconnected:
	case ServerMessage::Chat:
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
