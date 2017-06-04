// Kainoa Seto
// TalkClientServer
// 6-3-17
// A simple win32 TCP client to connect and talk to a talk server, this is the main include
// file shared between both the client and server and contains the bulk of the processing
// include.c

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

int start_chat(SOCKET rmt_sock)
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
	// Get our thread data from the arguments
	struct thread_data rmt_data = *(struct thread_data*)args;
	// The current character read in from stdin
	char in_ch = ' ';
	// If we are ready to send the message to the other client/server
	BOOL send_input = FALSE;
	// Current characters typed in the message
	int char_count = 0;

	// Setup the initial key
	printf("> ");
	
	for (;;)
	{
		// If the keyboard is pressed then we want to do some processing
		if (_kbhit())
		{
			// get the key that was pressed
			in_ch = _getch();
			// If it was enter then we are ready to send
			if (in_ch == '\r')
				send_input = TRUE;
			// If it is a backspace then simulate the backspace command on screen and in our buffer
			else if (in_ch == '\b')
			{
				// only backspace the characters we typed
				if (char_count > 0)
				{
					// Remove character from screen, message buffer, and char count
					printf("\b \b");
					trump(rmt_data.msg_buf);
					char_count--;
				}
			}
			else
			{
				// We have a normal character added into the message
				// so display on screen and add to buffer
				printf("%c", in_ch);
				append(rmt_data.msg_buf, in_ch);
				char_count++;
			}
		}
		
		if (send_input)
		{
			// Make sure nothing else is going on with the console on the receive thread
			EnterCriticalSection(rmt_data.cr);
			// Send our message buffer
			send(rmt_data.sock, rmt_data.msg_buf, strlen(rmt_data.msg_buf) + 1, 0);
			// If exit is typed then mark that we exited to the listener thread and exit
			if (strcmp(rmt_data.msg_buf, "exit") == 0)
			{
				*rmt_data.exit = TRUE;
				return 0;
			}

			// Setup key for next message
			printf("\n> ");
			// We are done outputting to the console and socket
			LeaveCriticalSection(rmt_data.cr);
			// Reset buffer
			memset(rmt_data.msg_buf, 0, MAX_INPUT);
			// Reset flag
			send_input = FALSE;
			// Reset count
			char_count = 0;
		}
	}
}

DWORD WINAPI lstn(void* args)
{
	// Get our thread data from the arguments
	struct thread_data rmt_data = *(struct thread_data*)args;
	// Buffer that we received
	char read_buf[MAX_INPUT];
	// Bytes read in from recv
	int read_bytes;

	// The console screen buffer information so we know the max line to set
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int columns;

	// Get output handle for getting the screen buffer information
	HANDLE h_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if (h_stdout == INVALID_HANDLE_VALUE)
	{
		printf("Failed to get stdout handle\n");
		return 2;
	}

	// Get the screen buffer information and calculate columns
	GetConsoleScreenBufferInfo(h_stdout, &csbi);
	columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;

	// Set non blocking on our receive
	ULONG on = 1;
	ioctlsocket(rmt_data.sock, FIONBIO, &on);

	// As long as the other thread hasn't signaled exit then we are good
	while (!*rmt_data.exit)
	{
		// Receive data by polling
		read_bytes = recv(rmt_data.sock, read_buf, MAX_INPUT, 0);

		// If we received soemthing then process it
		if (read_bytes > 0)
		{
			// Add null to the end in case we didn't receive it that way
			read_buf[read_bytes-1] = '\0';

			// Enter our critical section since we are interacting with the console
			EnterCriticalSection(rmt_data.cr);
			
			// Clear out the current row in the console in case the user was typging
			printf("\r%*c", columns-1, ' ');
			// Then paste over it with what we received
			printf("\r< %s\n> ", read_buf);

			// If the user had something in the console that they were typging then add it back in at the bottom
			if (strlen(rmt_data.msg_buf) > 0)
				printf("%s", rmt_data.msg_buf);

			// We terminate the other thread on exit of this one so we do not need to signal
			if (strcmp(read_buf, "exit") == 0)
				break;
			// We are done itneracting with the console and socket
			LeaveCriticalSection(rmt_data.cr);

			// Reset our buffer
			memset(read_buf, 0, MAX_INPUT);
		}
	}

	// Close out stdout handle
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
	// As long as the cstring has chars then truncate
	if (len > 0)
		str[len - 1] = '\0';
}