#pragma comment (lib, "Ws2_32.lib")

#include "Structures.h"
#include "UserFunctions.cpp"

#define TRUE 1
#define FALSE 0

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 21000
#define BUFFER_SIZE 1024

int main()
{
#pragma region Connection

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
#pragma endregion

	int userId = 0;
	char name[100];
	unsigned short listenPort = 0;
	do {
		// Sending init message to server
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

	unsigned long  mode = 1;
	if (ioctlsocket(connectSocket, FIONBIO, &mode) != 0)
		printf("ioctlsocket failed with error.");


	int option = 0;
	do {
		printf("1. Send messages over server.\n");
		printf("2. Connect p2p with client.\n");
		printf("0. Exit.\n");
		printf("Enter option:\n");
		scanf("%d", &option);

	} while (option < 0 || option > 2);
	if (option == 1) {
		{
			printf("Enter any key to send message.\n");
			while (1) {
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
					break;
				}
				else if (selectResult == 0) // timeout expired
				{
					if (_kbhit()) {

						printf("Enter user id: ");
						int id = 0;
						scanf("%d", &id);
						char buffer[BUFFER_SIZE];
						printf("Enter message: ");
						scanf("%s", buffer);
						sendMessage(connectSocket, buffer, id);
					}
					continue;
				}
				else
				{
					if (FD_ISSET(connectSocket, &readfds)) {
						Message message = {};
						int result = recv(connectSocket, (char*)&message, sizeof(message.messageSize) * 2, 0);
						message.messageSize = ntohl(message.messageSize);
						message.id = ntohl(message.id);

						// receive message
						result = recv(connectSocket, message.message, message.messageSize, 0);

						printf("Message arrived from id: %d.\n", message.id);
						printf("Message: %s\n", message.message);
					}
				}
			}
		}
	}

	if (option == 2) {
		SOCKET listenSocket = INVALID_SOCKET;

		SOCKET p2pSocket = INVALID_SOCKET;

		// Initialize serverAddress structure used by bind
		sockaddr_in serverAddress;
		memset((char*)&serverAddress, 0, sizeof(serverAddress));
		serverAddress.sin_family = AF_INET;				// IPv4 address family
		serverAddress.sin_addr.s_addr = INADDR_ANY;		// Use all available addresses
		serverAddress.sin_port = htons(listenPort);	// Use specific port


		// Create a SOCKET for connecting to server
		listenSocket = socket(AF_INET,      // IPv4 address family
			SOCK_STREAM,  // Stream socket
			IPPROTO_TCP); // TCP protocol

		// Check if socket is successfully created
		if (listenSocket == INVALID_SOCKET)
		{
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Setup the TCP listening socket - bind port number and local address to socket
		iResult = bind(listenSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

		// Check if socket is successfully binded to address and port from sockaddr_in structure
		if (iResult == SOCKET_ERROR)
		{
			printf("bind failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}

		// Set listenSocket in listening mode
		iResult = listen(listenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR)
		{
			printf("listen failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}

		printf("Server socket is set to listening mode. Waiting for new connection requests.\n");

		unsigned long  mode = 1;
		if (ioctlsocket(listenSocket, FIONBIO, &mode) != 0)
			printf("ioctlsocket failed with error.");

		printf("Enter any key to connect to p2p.\n");

		while (1) {
			// initialize socket set
			FD_ZERO(&readfds);

			if (p2pSocket != INVALID_SOCKET) {
				FD_SET(p2pSocket, &readfds);
			}
			
			FD_SET(listenSocket, &readfds);
			FD_SET(connectSocket, &readfds);

			// wait for events on set
			int selectResult = select(0, &readfds, NULL, NULL, &timeVal);

			if (selectResult == SOCKET_ERROR)
			{
				printf("Select failed with error: %d\n", WSAGetLastError());
				//closesocket(listenSocket);
				break;
			}
			else if (selectResult == 0) // timeout expired
			{
				if (_kbhit()) {
					if (p2pSocket == INVALID_SOCKET) {
						printf("Enter p2p user id: ");
						int id = 0;
						scanf("%d", &id);
						if (id != userId) {
							getP2PConnectionData(connectSocket, id);
						}
						else {
							printf("Unable to send same id as you have.\n");
						}
					}
					else {
						int id = 0;
						char buffer[BUFFER_SIZE];
						printf("Enter message: ");
						scanf("%s", buffer);
						sendMessage(p2pSocket, buffer, id);
					}
				}
				continue;
			}
			else
			{
				if (FD_ISSET(listenSocket, &readfds)) {
					// Struct for information about connected client
					sockaddr_in clientAddr;
					int clientAddrSize = sizeof(struct sockaddr_in);
					// New connection request is received. Add new socket in array on first free position.
					SOCKET newClient = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

					if (newClient == INVALID_SOCKET)
					{
						if (WSAGetLastError() == WSAECONNRESET)
						{
							printf("accept failed, because timeout for client request has expired.\n");
						}
						else
						{
							printf("accept failed with error: %d\n", WSAGetLastError());
						}
					}
					else
					{
						printf("New client request accepted. Client address: %s : %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

						unsigned long  mode = 1;
						if (ioctlsocket(newClient, FIONBIO, &mode) != 0)
							printf("ioctlsocket failed with error.");

						p2pSocket = newClient;

					}
				}
				if (FD_ISSET(connectSocket, &readfds)) {
					MessageType type = {};
					int result = recv(connectSocket, (char*)&type, sizeof(MessageType), 0);
					type.message_type = ntohl(type.message_type);

					if (result == sizeof(MessageType)) {
						// message type
						char buffer[BUFFER_SIZE];
						int messageSize = 0;

						switch (type.message_type)
						{
						case SEND_MESSAGE:
						{
							// forward message to client
							Message message = {};
							result = recv(connectSocket, (char*)&message, sizeof(message.messageSize) * 2, 0);
							message.messageSize = ntohl(message.messageSize);
							message.id = ntohl(message.id);

							// receive message
							result = recv(connectSocket, message.message, message.messageSize, 0);
							if (result > 0) {
								printf("Message arrived from id: %d.\n", message.id);
								printf("Message: %s\n", message.message);
							}

							break;
						}
						case P2P_CONNECTION_RESPONSE:
						{
							P2PConnectionResponse response = {};
							result = recv(connectSocket, (char*)&response, sizeof(response), 0);
							response.ip_address = ntohl(response.ip_address);
							response.listen_port = ntohs(response.listen_port);
							if (response.listen_port == 0) {
								printf("There is no user with given id.\n");
							}
							else {
								printf("User ip adress and listen port found.\n");
								p2pSocket = socket(AF_INET,
									SOCK_STREAM,
									IPPROTO_TCP);

								if (p2pSocket == INVALID_SOCKET)
								{
									printf("socket failed with error: %ld\n", WSAGetLastError());
									WSACleanup();
									return 1;
								}

								// Create and initialize address structure
								sockaddr_in serverAdd;
								serverAdd.sin_family = AF_INET;								// IPv4 protocol
								serverAdd.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// ip address of server
								serverAdd.sin_port = htons(response.listen_port);					// server port

								// Connect to server specified in serverAddress and socket p2pSocket
								iResult = connect(p2pSocket, (SOCKADDR*)&serverAdd, sizeof(serverAdd));
								if (iResult == SOCKET_ERROR)
								{
									printf("Unable to connect to p2p server.\n");
								}
								else {
									printf("Successfully connected to p2p server.\n");
									unsigned long  mode = 1;
									if (ioctlsocket(p2pSocket, FIONBIO, &mode) != 0)
										printf("ioctlsocket failed with error.");
								}
							}
							break;
						}
						default:
							break;
						}
					}


				}
				if (FD_ISSET(p2pSocket, &readfds)) {
					MessageType type = {};
					int result = recv(p2pSocket, (char*)&type, sizeof(MessageType), 0);
					type.message_type = ntohl(type.message_type);

					if (result == sizeof(MessageType)) {
						// message type
						char buffer[BUFFER_SIZE];
						int messageSize = 0;

						switch (type.message_type)
						{
						case SEND_MESSAGE:
						{
							// forward message to client
							Message message = {};
							result = recv(p2pSocket, (char*)&message, sizeof(message.messageSize) * 2, 0);
							message.messageSize = ntohl(message.messageSize);
							message.id = ntohl(message.id);

							// receive message
							result = recv(p2pSocket, message.message, message.messageSize, 0);
							if (result > 0) {
								printf("Message: %s\n", message.message);
							}

							break;
						}
						default:
							break;
						}
					}


				}
			}
		}

	}

	//else if (FD_ISSET(listenSocket, &readfds))
	//{
	
	//}

#pragma region Shutdown
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
#pragma endregion
}
