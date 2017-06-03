// Kainoa Seto
// Talk Client
// 6-3-17
// A simple win32 TCP client to connect and talk to a talk server
// main.c

#include "client.h"

int main(int argc, char* argv[])
{
	// Optional hostname but must have port
	if (argc < 2)
	{
		printf("Usage: TalkClient port [hostname]\n");
		return 0;
	}

	char hostname[MAX_PATH] = "";
	USHORT port = (USHORT)atoi(argv[1]);

	// If the hostname is specified use that instead
	if (argc == 3)
		strcpy_s(hostname, MAX_PATH, argv[2]);

	// Start the client
	return start_client(hostname, port);
}