#include "server.h"

int start_server()
{
	SOCKET server_sock;
	WSADATA server_wsa_data;

	if (WSAStartup(MAKEWORD(2, 2), &server_wsa_data) != 0)
	{
		printf("WSAStartup Failure: %08X\n", WSAGetLastError());
		return 1;
	}

	if ((server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		printf("ServerSocket creation failed: %08X\n", WSAGetLastError());
		return 2;
	}

	// Host could be a network path so to be safe we'll do MAX_PATH
	char server_hostname[MAX_PATH];
	struct hostent* host;
	
	if (gethostname(server_hostname, MAX_PATH) != 0)
	{
		printf("GetHostName failed: %08X\n", GetLastError());
		return 3;
	}

	if ((host = gethostbyname(server_hostname)) == NULL)
	{
		printf("GetHostByName failed: %08X\n", GetLastError());
		return 4;
	}

	struct sockaddr_in serversock_in;
	// Get a random open port
	USHORT port = 0;

	ZeroMemory(&serversock_in, sizeof(struct sockaddr_in));
	serversock_in.sin_family = AF_INET;
	serversock_in.sin_port = htons(port);
	CopyMemory(&serversock_in.sin_addr, host->h_addr_list[0], host->h_length);

	if ((bind(server_sock, (const struct sockaddr*)&serversock_in, sizeof(serversock_in))) == SOCKET_ERROR)
	{
		printf("Failed to bind: %08X\n", WSAGetLastError());
		return 5;
	}

	printf("###########################################################\n");
	printf("	Server started on host '%s' and port '%i'\n", server_hostname, ntohs(serversock_in.sin_port));
	printf("###########################################################\n");

	printf("Listening for client...\n");
	// TODO: Listen here until we find the client
	
	return 0;
}