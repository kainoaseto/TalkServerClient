// Kainoa Seto
// Talk Server
// 6-3-17
// A simple win32 TCP socket server to facilitate chat between the server and one client server implementation
// server.c

#include "server.h"

int start_server()
{
	SOCKET server_sock;
	WSADATA server_wsa_data;

	// Setup the WSA information
	if (WSAStartup(MAKEWORD(2, 2), &server_wsa_data) != 0)
	{
		printf("WSAStartup Failure: %08X\n", WSAGetLastError());
		return 1;
	}

	// Create our server socket
	if ((server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		printf("ServerSocket creation failed: %08X\n", WSAGetLastError());
		return 2;
	}

	// Host could be a network path so to be safe we'll do MAX_PATH
	char server_hostname[MAX_PATH];
	struct hostent* host;
	
	// Get hostname
	if (gethostname(server_hostname, MAX_PATH) != 0)
	{
		printf("GetHostName failed: %08X\n", GetLastError());
		return 3;
	}

	// Get host information from hostname
	if ((host = gethostbyname(server_hostname)) == NULL)
	{
		printf("GetHostByName failed: %08X\n", GetLastError());
		return 4;
	}

	struct sockaddr_in server_sock_in;
	// Get a random open port
	USHORT port = 0;

	ZeroMemory(&server_sock_in, sizeof(struct sockaddr_in));
	server_sock_in.sin_family = AF_INET;
	server_sock_in.sin_port = htons(port);
	CopyMemory(&server_sock_in.sin_addr, host->h_addr_list[0], host->h_length);

	// Bind our socket to the information above
	if ((bind(server_sock, (const struct sockaddr*)&server_sock_in, sizeof(server_sock_in))) == SOCKET_ERROR)
	{
		printf("Failed to bind: %08X\n", WSAGetLastError());
		return 5;
	}

	// Listen for a client
	if (listen(server_sock, SOMAXCONN) != 0)
	{
		printf("Failed to start listener: %08X\n", WSAGetLastError());
		return 6;
	}

	// Gets updated information after bind
	int server_sock_in_len = sizeof(struct sockaddr_in);
	if (getsockname(server_sock, (struct sockaddr*)&server_sock_in, &server_sock_in_len) != 0)
	{
		printf("Failed to retrieve populated socket data: %08X\n", GetLastError());
		return 7;
	}

	printf("###########################################################\n");
	printf("	Server started on host '%s' and port '%i'\n", server_hostname, htons(server_sock_in.sin_port));
	printf("###########################################################\n");

	printf("Listening for client...\n");
	
	struct sockaddr_in client_sock_in;
	int client_sock_in_len = sizeof(struct sockaddr_in);
	SOCKET client_sock = accept(server_sock, (struct sockaddr*)&client_sock_in, &client_sock_in_len);
	
	printf("Connected!\n");

	// We found a client, lets start the chat
	start_chat(client_sock);

	// Close our listener socket since we are done with it
	closesocket(server_sock);

	return 0;
}