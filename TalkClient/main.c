#include "client.h"

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("Usage: Talkclient port [hostname]\n");
		return 0;
	}

	char hostname[MAX_PATH] = "";
	USHORT port = (USHORT)atoi(argv[1]);
	if (argc == 3)
		strcpy_s(hostname, MAX_PATH, argv[2]);

	return start_client(hostname, port);
}