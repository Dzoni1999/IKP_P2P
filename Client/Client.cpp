#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN
//#define no_init_all deprecated

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"
#include <time.h>

#include "Structures.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define TRUE 1
#define FALSE 0

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 21000
#define BUFFER_SIZE 1024

int sendInitMessage(SOCKET socket, int userId,const char* name, unsigned short listenPort) {
	// message type
	char buffer[BUFFER_SIZE];
	int messageSize = 0;
	MessageType type;
	type.message_type = htonl(INIT_MESSAGE);
	memcpy(buffer, &type, sizeof(type));
	messageSize += sizeof(type);

	UserInit user;
	user.id = htonl(userId);
	user.listen_port = htons(listenPort);
	memcpy(user.name, name, strlen(name)+1);
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
	char buffer[BUFFER_SIZE];
	int messageSize = 0;
	MessageType type;
	type.message_type = htonl(SEND_MESSAGE);
	memcpy(buffer, &type, sizeof(type));
	messageSize += sizeof(type);

	Message msg;
	memcpy(msg.message, message, strlen(message));
	msg.messageSize = htonl(strlen(message));
	msg.id = htonl(id);
	memcpy(buffer + sizeof(type), &msg, sizeof(msg.messageSize)*2 + strlen(message));
	messageSize += sizeof(msg.messageSize)*2 + strlen(message);
	int result = send(socket, buffer, messageSize, 0);
	if (result == messageSize) {
		printf("Message successfully sent.\n");
	}
}

void getP2PConnectionData(SOCKET socket, int p2pUserId) {
	// message type
	char buffer[BUFFER_SIZE];
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
	char buffer[BUFFER_SIZE];
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

int main()
{
	// Socket used to communicate with server
	SOCKET connectSocket = INVALID_SOCKET;

	// Variable used to store function return value
	int iResult;

	// Buffer we will use to store message
	char dataBuffer[BUFFER_SIZE];

	// WSADATA data structure that is to receive details of the Windows Sockets implementation
	WSADATA wsaData;

	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	// create a socket
	connectSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Create and initialize address structure
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;								// IPv4 protocol
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// ip address of server
	serverAddress.sin_port = htons(SERVER_PORT);					// server port

	// Connect to server specified in serverAddress and socket connectSocket
	iResult = connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
	if (iResult == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	printf("Successfully connected to server.\n");

	int userId = 0;
	char name[100];
	unsigned short listenPort = 0;
	do {
		// send init message
		printf("Enter user id: ");
		scanf("%d", &userId);

		printf("Enter username:");
		scanf("%s", name);
		
		printf("Enter listening port:");
		scanf("%d", &listenPort);
		
	} while (sendInitMessage(connectSocket, userId, name, listenPort));

	// set of socket descriptors
	fd_set readfds;

	// timeout for select function
	timeval timeVal;
	timeVal.tv_sec = 0;
	timeVal.tv_usec = 300;


	int option = 0;
	do {
		printf("1. Check incoming messages.\n");
		printf("2. Send message to user.\n");
		printf("3. Connect p2p with client.\n");
		printf("Enter option:\n");
		scanf("%d", &option);
		switch (option)
		{
		case 1:
		{
			// initialize socket set
			FD_ZERO(&readfds);

			FD_SET(connectSocket, &readfds);
			//FD_SET(listenSocket, &readfds);

			// wait for events on set
			int selectResult = select(0, &readfds, NULL, NULL, &timeVal);

			if (selectResult == SOCKET_ERROR)
			{
				printf("Select failed with error: %d\n", WSAGetLastError());
				//closesocket(listenSocket);
				option = -1;
				break;
			}
			else if (selectResult == 0) // timeout expired
			{
				printf("No new messages");
				break;
			}
			//else if (FD_ISSET(listenSocket, &readfds))
			//{
			//	// Struct for information about connected client
			//	sockaddr_in clientAddr;
			//	int clientAddrSize = sizeof(struct sockaddr_in);

			//	// New connection request is received. Add new socket in array on first free position.
			//	SOCKET newClient = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

			//	if (newClient == INVALID_SOCKET)
			//	{
			//		if (WSAGetLastError() == WSAECONNRESET)
			//		{
			//			printf("accept failed, because timeout for client request has expired.\n");
			//		}
			//		else
			//		{
			//			printf("accept failed with error: %d\n", WSAGetLastError());
			//		}
			//	}
			//	else
			//	{

			//	}
			//}
			else
			{
				if (FD_ISSET(connectSocket, &readfds)) {
					Message message = {};
					int result = recv(connectSocket, (char*)&message, sizeof(message.messageSize)*2, 0);
					message.messageSize = ntohl(message.messageSize);
					message.id = ntohl(message.id);

					// receive message
					result = recv(connectSocket, message.message, message.messageSize, 0);

					printf("Message arrived from id: %d.\n", message.id);
					printf("Message: %s\n", message.message);
				}
			}
			break;
		}
		case 2:
		{
			printf("Enter user id: ");
			int id = 0;
			scanf("%d", &id);
			char buffer[BUFFER_SIZE];
			printf("Enter message: ");
			scanf("%s", buffer);
			sendMessage(connectSocket, buffer, id);
			break;
		}
		case 3:
		{

			break;
		}
		default:
			break;
		}

	} while (option >= 1 && option <= 3);

	// Shutdown the connection since we're done
	iResult = shutdown(connectSocket, SD_BOTH);

	// Check if connection is succesfully shut down.
	if (iResult == SOCKET_ERROR)
	{
		printf("Shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	Sleep(1000);

	// Close connected socket
	closesocket(connectSocket);

	// Deinitialize WSA library
	WSACleanup();

	return 0;
}
