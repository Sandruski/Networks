#pragma once

// Add as many messages as you need depending on the
// functionalities that you decide to implement.

enum class ClientMessage
{
	Hello,
	Chat
};

enum class ServerMessage
{
	Welcome,
	NonWelcome,
	ClientConnected,
	ClientDisconnected,
	Chat,
	Help,
	List,
	Disconnect,
	Whisper,
	ChangeName
};

