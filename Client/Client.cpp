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
