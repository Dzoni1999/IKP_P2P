#pragma once

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"
#include <time.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

/*
 *	Function: sendInitMessage
 * --------------------
 *	Sending initialize message .
 *
 *	returns: 0 if user us succesfully registered.
 */
int sendInitMessage(SOCKET socket, int userId, const char* name, unsigned short listenPort);

/*
 *	Function: sendMessage
 * --------------------
 *	Sending user message and ID to server.
 *
 *	returns: /
 */
void sendMessage(SOCKET socket, char* message, int id);

/*
 *	Function: getP2PConnectionData
 * --------------------
 *	Sending request from one client to another.
 *
 *	returns: /
 */
void getP2PConnectionData(SOCKET socket, int p2pUserId);

/*
 *	Function: sendMessageP2P
 * --------------------
 *	Sending user message and ID to another client.
 *
 *	returns: /
 */
void sendMessageP2P(SOCKET socket, char* message);
