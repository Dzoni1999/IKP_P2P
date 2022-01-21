#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"
#include "Structures.h"
#include "List.cpp"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_PORT 21000
#define BUFFER_SIZE 1024


CRITICAL_SECTION cs;
List userList;

typedef struct process_thread_data {
	SOCKET socket;
	unsigned long ip_adress;
}ProcessThreadData;

DWORD WINAPI processMessageThread(LPVOID lpParam) {
	ProcessThreadData data = *(ProcessThreadData*)lpParam;
	SOCKET socket = data.socket;
	char buffer[BUFFER_SIZE];
	int result = 0;
	MessageType type;
	
	while (true) {

		int result = recv(data.socket, (char*)&type, sizeof(MessageType), 0);
		type.message_type = ntohl(type.message_type);

		if (result == sizeof(MessageType)) {
			// message type
			char buffer[BUFFER_SIZE];
			int messageSize = 0;

			switch (type.message_type)
			{
			case INIT_MESSAGE:
			{
				// Adding client
				UserInit init = {};
				result = recv(socket, (char*)&init, sizeof(init), 0);
				init.id = ntohl(init.id);
				init.listen_port = ntohs(init.listen_port);

				// Check if able to add client
				EnterCriticalSection(&cs);
				UserData userData = getitembyid(userList, init.id);
				int responseRetVal = 0;
				if (userData.id == init.id) {
					responseRetVal = -1;
				}
				else {
					userData.id = init.id;
					userData.ip_address = data.ip_adress;
					userData.listen_port = init.listen_port;
					//memcpy(userData.name, init.name, strlen(init.name));
					strcpy(userData.name, init.name);
					userData.socket = socket;
					insertback(&userList, userData);
					printf("Client with id: %d username: %s successfully initializes.\n", userData.id, userData.name);
				}
				LeaveCriticalSection(&cs);


				UserInitResponse response;
				response.result = htonl(responseRetVal);
				result = send(socket, (char*)&response, sizeof(response), 0);

				break;
			}
			case SEND_MESSAGE:
			{
				// forward message to client
				Message message = {};
				result = recv(socket, (char*)&message, sizeof(message.messageSize) * 2, 0);
				message.messageSize = ntohl(message.messageSize);
				message.id = ntohl(message.id);

				// receive message
				result = recv(socket, message.message, message.messageSize, 0);
				if (result > 0) {
					// send message to destination
					EnterCriticalSection(&cs);
					UserData userData = getitembyid(userList, message.id);
					LeaveCriticalSection(&cs);
					if (userData.id != -1) {
						int msgSize = sizeof(int) * 2 + message.messageSize;
						message.id = htonl(message.id);
						message.messageSize = htonl(message.messageSize);
						result = send(userData.socket, (char*)&message, msgSize, 0);
						if (result == msgSize) {
							//return 0;
						}
						else {
							return -1;
						}
					}
				}

				break;
			}
			case P2P_CONNECTION_REQUEST:
				// send client data back
				break;
			default:
				break;
			}
		}
		
	}
	return 0;
}

int main()
{
#pragma region Connection
	initlist(&userList);
    // Socket used for listening for new clients 
    SOCKET listenSocket = INVALID_SOCKET;

    // Socket used for communication with client
    SOCKET acceptedSocket = INVALID_SOCKET;

    // Variable used to store function return value
    int iResult;

    // Buffer used for storing incoming data
    char dataBuffer[BUFFER_SIZE];

    // WSADATA data structure that is to receive details of the Windows Sockets implementation
    WSADATA wsaData;

    // Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return 1;
    }

	InitializeCriticalSection(&cs);


    // Initialize serverAddress structure used by bind
    sockaddr_in serverAddress;
    memset((char*)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;				// IPv4 address family
    serverAddress.sin_addr.s_addr = INADDR_ANY;		// Use all available addresses
    serverAddress.sin_port = htons(SERVER_PORT);	// Use specific port


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
#pragma endregion
	
	while (true)
	{
		// set of socket descriptors
		fd_set readfds;

		// timeout for select function
		timeval timeVal;
		timeVal.tv_sec = 1;
		timeVal.tv_usec = 0;
		// initialize socket set
		FD_ZERO(&readfds);

		FD_SET(listenSocket, &readfds);
		int lenght = 0;
		EnterCriticalSection(&cs);
		lenght = length(userList);
		for (int i = 0; i < lenght; i++)
		{
			UserData data = getitem(userList, i);
			FD_SET(data.socket, &readfds);
		}
		LeaveCriticalSection(&cs);
		// wait for events on set
		int selectResult = select(0, &readfds, NULL, NULL, &timeVal);

		if (selectResult == SOCKET_ERROR)
		{
			printf("Select failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}
		else if (selectResult == 0) // timeout expired
		{
			Sleep(1);
			continue;
		}
		else if (FD_ISSET(listenSocket, &readfds))
		{
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
				/*if (ioctlsocket(clientSockets[lastIndex], FIONBIO, &mode) != 0)
				{
					printf("ioctlsocket failed with error.");
					continue;
				}
				lastIndex++;*/
				printf("New client request accepted. Client address: %s : %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

				ProcessThreadData threadData = {};
				threadData.socket = newClient;
				threadData.ip_adress = clientAddr.sin_addr.S_un.S_addr;
				DWORD retVal;
				HANDLE threadHandle = CreateThread(NULL, 0, &processMessageThread, &threadData, 0, &retVal);
			}
		}
		else
		{
			//int lenght = 0;
			//EnterCriticalSection(&cs);
			//lenght = length(userList);
			//LeaveCriticalSection(&cs);

			//for (int i = 0; i < lenght; i++)
			//{
			//	SOCKET clientSocket;

			//	EnterCriticalSection(&cs);
			//	UserData data = getitem(userList, i);
			//	LeaveCriticalSection(&cs);
			//	clientSocket = data.socket;

			//	// Check if new message is received from client on position "i"
			//	if (FD_ISSET(clientSocket, &readfds))
			//	{

			//		MessageType type;
			//		int result = recv(data.socket, (char*)&type, sizeof(MessageType), 0);
			//		type.message_type = ntohl(type.message_type);

			//		ProcessThreadData threadData = {};
			//		threadData.socket = clientSocket;
			//		threadData.ip_adress = data.ip_address;
			//		threadData.type = type.message_type;
			//		DWORD retVal;
			//		HANDLE threadHandle = CreateThread(NULL, 0, &processMessageThread, &threadData, 0, &retVal);
			//	}
			//}
		}
		FD_ZERO(&readfds);
	}

#pragma region Shutdown
    // Shutdown the connection since we're done
    iResult = shutdown(acceptedSocket, SD_BOTH);

    // Check if connection is succesfully shut down.
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(acceptedSocket);
        WSACleanup();
        return 1;
    }

    //Close listen and accepted sockets
    closesocket(listenSocket);
    closesocket(acceptedSocket);

    // Deinitialize WSA library
    WSACleanup();


#pragma endregion
}
