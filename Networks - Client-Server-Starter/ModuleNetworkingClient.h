#pragma once

#include "ModuleNetworking.h"

#include <list>

class ModuleNetworkingClient : public ModuleNetworking {
public:
    //////////////////////////////////////////////////////////////////////
    // ModuleNetworkingClient public methods
    //////////////////////////////////////////////////////////////////////

    bool start(const char* serverAddress, int serverPort, const char* playerName);

    bool isRunning() const;

private:
    //////////////////////////////////////////////////////////////////////
    // Module virtual methods
    //////////////////////////////////////////////////////////////////////

    bool update() override;

    bool gui() override;

    //////////////////////////////////////////////////////////////////////
    // ModuleNetworking virtual methods
    //////////////////////////////////////////////////////////////////////

    void onPacketReceived(SOCKET socket, const InputMemoryStream& packet) override;

    void onSocketDisconnected(SOCKET socket) override;

    //////////////////////////////////////////////////////////////////////
    // Client state
    //////////////////////////////////////////////////////////////////////

    enum class ClientState {
        Stopped,
        Start,
        Logging
    };

	struct Message {
		Message() {}
		Message(std::string message, ImVec4 color) : m_message(message), m_color(color) {}
		Message(const Message& message)
		{
			m_message = message.m_message;
			m_color = message.m_color;
		}
		~Message() {}

		std::string m_message;
		ImVec4 m_color;
	};

    ClientState state = ClientState::Stopped;

    sockaddr_in serverAddress = {};
    SOCKET s = INVALID_SOCKET;

    std::string playerName;
	ImVec4 m_playerColor;

	std::list<Message> m_messages;
};
