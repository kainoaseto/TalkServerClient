#include "include.h"

// Thread data to be passed between the talk and listener threads
// so that they can communicate with each other
struct thread_data
{
	SOCKET sock;
	CRITICAL_SECTION* cr;
	BOOL* exit;
	char* msg_buf;
};

int start_chat(SOCKET rmt_sock, struct sockaddr_in* rmt_sock_in)
{
	// On the chance that a send/recv happens simultaneously this will prevent characters
	// in the console from overlapping and from read/writes to the socket giving undesired
	// behaivor including but not limited to: data corruption, crashing, failure to send/recv
	CRITICAL_SECTION console_mutex;
	// Determine when we should exit if we receive a exit command from one of the threads
	BOOL exit = FALSE; 
	// Message buffer to pass between threads so that the in-progress message is saved
	// when a new message from the server/client comes in
	char msg_buf[MAX_INPUT];

	// Clear our buffer
	memset(msg_buf, 0, MAX_INPUT);

	// Initailize the critical section we will use to lock writing/reading from the console
	InitializeCriticalSection(&console_mutex);

	// Setup remote information to pass through to threads so that they
	// are able to communicate on exiting, and I/O to the remote server
	struct thread_data rmt_info;
	rmt_info.sock = rmt_sock;
	rmt_info.cr = &console_mutex;
	rmt_info.exit = &exit;
	rmt_info.msg_buf = &msg_buf;

	// Create our talk and listener threads with:
	// - default security attributes
	// - default stack size
	// - function pointer to either talk or listen function to be ran
	// - remote information used to keep track of I/O between threads
	// - default creation flags
	// - default thread id
	HANDLE talk_handle = CreateThread(NULL, NULL, talk, (void*)&rmt_info, NULL, NULL);
	HANDLE lstn_handle = CreateThread(NULL, NULL, lstn, (void*)&rmt_info, NULL, NULL);

	// Wait until we receive our exit
	WaitForSingleObject(lstn_handle, INFINITE);
	
	// Terminate the talker since it will not be aware of the exit
	TerminateThread(talk_handle, 0);

	// Cleanup Thread handles
	CloseHandle(lstn_handle);
	CloseHandle(talk_handle);

	// Clean up socket information
	closesocket(rmt_sock);
	WSACleanup();

	return 0;
}

DWORD WINAPI talk(void* args)
{
	struct thread_data rmt_data = *(struct thread_data*)args;
	char in_ch = ' ';
	BOOL send_input = FALSE;
	int char_count = 0;

	// Setup the initial key
	printf("> ");
	
	for (;;)
	{
		if (_kbhit())
		{
			in_ch = _getch();
			if (in_ch == '\r')
				send_input = TRUE;
			else if (in_ch == '\b')
			{
				if (char_count > 0)
				{
					printf("\b \b");
					trump(rmt_data.msg_buf);
					char_count--;
				}
			}
			else
			{
				printf("%c", in_ch);
				append(rmt_data.msg_buf, in_ch);
				char_count++;
			}
		}
		
		if (send_input)
		{
			// Make sure nothing else is going on
			EnterCriticalSection(rmt_data.cr);
			send(rmt_data.sock, rmt_data.msg_buf, strlen(rmt_data.msg_buf) + 1, 0);

			if (strcmp(rmt_data.msg_buf, "exit") == 0)
			{
				*rmt_data.exit = TRUE;
				return 0;
			}

			printf("\n> ");

			LeaveCriticalSection(rmt_data.cr);

			memset(rmt_data.msg_buf, 0, MAX_INPUT);

			send_input = FALSE;
		}
	}
}

DWORD WINAPI lstn(void* args)
{
	struct thread_data rmt_data = *(struct thread_data*)args;
	
	char read_buf[MAX_INPUT];
	int read_bytes;

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int columns;

	HANDLE h_stdin = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE h_stdout = GetStdHandle(STD_OUTPUT_HANDLE);

	if (h_stdin == INVALID_HANDLE_VALUE)
	{
		printf("Failed to get stdin handle\n");
		return 1;
	}

	if (h_stdout == INVALID_HANDLE_VALUE)
	{
		printf("Failed to get stdout handle\n");
		return 2;
	}

	GetConsoleScreenBufferInfo(h_stdout, &csbi);
	columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;

	// Set non blocking on our receive
	ULONG on = 1;
	ioctlsocket(rmt_data.sock, FIONBIO, &on);

	while (!*rmt_data.exit)
	{
		read_bytes = recv(rmt_data.sock, read_buf, MAX_INPUT, 0);

		if (read_bytes > 0)
		{
			read_buf[read_bytes-1] = '\0';

			// Enter our critical section since we are interacting with the console
			EnterCriticalSection(rmt_data.cr);
			
			printf("\r%*c", columns-1, ' ');
			printf("\r< %s\n> ", read_buf);

			if (strlen(rmt_data.msg_buf) > 0)
			{
				printf("%s", rmt_data.msg_buf);
			}

			// We terminate the other thread on exit of this one so we do not need to signal
			if (strcmp(read_buf, "exit") == 0)
				break;

			LeaveCriticalSection(rmt_data.cr);

			memset(read_buf, 0, MAX_INPUT);
		}
	}

	CloseHandle(h_stdin);
	CloseHandle(h_stdout);
	return 0;
}

void append(char* str, char c)
{
	size_t len = strlen(str);
	str[len] = c;
	str[len + 1] = '\0';
}

void trump(char* str)
{
	size_t len = strlen(str);
	if (len > 0)
		str[len - 1] = '\0';
}