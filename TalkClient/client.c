// Kainoa Seto
// Talk Client
// 6-3-17
// A simple win32 TCP client to connect and talk to a talk server
// client.c

#include "client.h"

int start_client(char* hostname, USHORT port)
{
	SOCKET client_sock;
	WSADATA client_wsa_data;
	struct hostent* host;
	struct sockaddr_in client_sock_in;

	// Setup the WSA data
	if (WSAStartup(MAKEWORD(2, 2), &client_wsa_data) != 0)
	{
		printf("WSAStartup Failure: %08X\n", WSAGetLastError());
		return 1;
	}

	// Setup client socket
	if ((client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		printf("ServerSocket Creation failed: %08X\n", WSAGetLastError());
		return 2;
	}

	// If hostname is empty then get the local hostname
	if (hostname == "")
	{
		if (gethostname(hostname, MAX_PATH) != 0)
		{
			printf("GetHostName failed: %08X\n", GetLastError());
			return 3;
		}
	}

	// get host information by the hostname
	if ((host = gethostbyname(hostname)) == NULL)
	{
		printf("GetHostByName failed: %08X\n", GetLastError());
		return 4;
	}

	ZeroMemory(&client_sock_in, sizeof(struct sockaddr_in));
	client_sock_in.sin_family = AF_INET;
	client_sock_in.sin_port = htons(port);
	CopyMemory(&client_sock_in.sin_addr, host->h_addr_list[0], host->h_length);

	// Attempt to connect to the server
	if (connect(client_sock, (const struct sockaddr*)&client_sock_in, sizeof(client_sock_in)) != 0)
	{
		printf("Failed to connect to the server: %08X\n", GetLastError());
		return 5;
	}
	printf("###########################################################\n");
	printf("Connected to '%s' on port '%i'!\n", host->h_name, htons(client_sock_in.sin_port));
	printf("###########################################################\n");

	// Start the talk chat
	return start_chat(client_sock);
}