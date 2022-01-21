#pragma once

typedef enum enum_message_type {
	INIT_MESSAGE = 5,
	SEND_MESSAGE = 1,
	P2P_CONNECTION_REQUEST = 2,
	P2P_CONNECTION_RESPONSE = 3,
	INIT_MESSAGE_RESPONSE = 4
}EMessageType;

typedef struct message_type {
	int message_type;
}MessageType;

#pragma region Client structures

typedef struct user_init {
	int id;
	char name[25];
	unsigned short listen_port;
}UserInit;

typedef struct message {
	int messageSize;
	int id;
	char message[512];
}Message;

typedef struct p2p_connection_request {
	int id;
}P2PConnectionRequest;

#pragma endregion

#pragma region Server structures

typedef struct user_init_response {
	int result;
}UserInitResponse;

typedef struct p2p_connection_response {
	unsigned short listen_port;
	int ip_address;
}P2PConnectionResponse;

#pragma endregion



