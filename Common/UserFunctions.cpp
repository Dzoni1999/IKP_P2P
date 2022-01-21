#include "Structures.h"
#include "UserFunctions.h"

int sendInitMessage(SOCKET socket, int userId, const char* name, unsigned short listenPort) {
	// message type
	char buffer[1024];
	int messageSize = 0;
	MessageType type;
	type.message_type = htonl(INIT_MESSAGE);
	memcpy(buffer, &type, sizeof(type));
	messageSize += sizeof(type);

	UserInit user;
	user.id = htonl(userId);
	user.listen_port = htons(listenPort);
	memcpy(user.name, name, strlen(name) + 1);
	messageSize += sizeof(user);
	memcpy(buffer + sizeof(type), &user, sizeof(user));

	int result = send(socket, buffer, messageSize, 0);
	if (result == messageSize) {
		printf("User init message successfully sent.\n");
		printf("Waiting for server response...\n");
	}

	UserInitResponse response;
	result = recv(socket, buffer, sizeof(response), 0);
	if (result == sizeof(response)) {
		memcpy(&response, buffer, sizeof(response));
		response.result = ntohl(response.result);
		if (response.result == 0) {
			printf("User successfully initialized.\n");
		}
		return response.result;
	}
	return -1;
}

void sendMessage(SOCKET socket, char* message, int id) {
	// message type
	char buffer[1024];
	int messageSize = 0;
	MessageType type;
	type.message_type = htonl(SEND_MESSAGE);
	memcpy(buffer, &type, sizeof(type));
	messageSize += sizeof(type);

	Message msg;
	memcpy(msg.message, message, strlen(message));
	msg.messageSize = htonl(strlen(message));
	msg.id = htonl(id);
	memcpy(buffer + sizeof(type), &msg, sizeof(msg.messageSize) * 2 + strlen(message));
	messageSize += sizeof(msg.messageSize) * 2 + strlen(message);
	int result = send(socket, buffer, messageSize, 0);
	if (result == messageSize) {
		printf("Message successfully sent.\n");
	}
}

void getP2PConnectionData(SOCKET socket, int p2pUserId) {
	// message type
	char buffer[1024];
	int messageSize = 0;
	MessageType type;
	type.message_type = htonl(P2P_CONNECTION_REQUEST);
	memcpy(buffer, &type, sizeof(type));
	messageSize += sizeof(type);

	P2PConnectionRequest p2pRequest;
	p2pRequest.id = htonl(p2pUserId);
	messageSize += sizeof(p2pRequest);
	memcpy(buffer + sizeof(type), &p2pRequest, sizeof(p2pRequest));

	int result = send(socket, buffer, messageSize, 0);
	if (result == messageSize) {
		printf("P2P connection request successfully sent.\n");
	}
}

void sendMessageP2P(SOCKET socket, char* message) {
	char buffer[1024];
	int messageSize = 0;
	Message msg;
	memcpy(msg.message, message, strlen(message));
	msg.messageSize = htonl(strlen(message));
	memcpy(buffer, &msg, sizeof(msg.messageSize) + strlen(message));
	messageSize += sizeof(msg.messageSize) + strlen(message);
	int result = send(socket, buffer, messageSize, 0);
	if (result == messageSize) {
		printf("Message successfully sent.\n");
	}
}