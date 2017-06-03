// Kainoa Seto
// TalkClientServer
// 6-3-17
// A simple win32 TCP client to connect and talk to a talk server, this is the main include
// file shared between both the client and server and contains the bulk of the processing
// include.h

#ifndef __INCLUDE_H__
#define __INCLUDE_H__

#include <stdio.h>
#include <stdlib.h>

#include <Windows.h>
#include <winsock.h>
#include <wincon.h>
#include <conio.h>

// We want to be able to send and receive at the same time, so we
// must have our receive and send on seperate threads. The main resource
// these both will be sharing is the terminal. So we will need to have
// some locking in place to make sure we only write after we have finished receiving.
// This means that lets say we have a 'send' action and a 'receive' action
// at the same time. We will check if we have a receive and if we do not then send.
// If we have a receive, then copy our buffer, output the receive, then send

// Longest message we will send or accept
// It would be good to do a compare of window line length on both client/server
// and then choose the smallest to set for this value so that each message is
// only a line long.
#define MAX_INPUT 512

// Append helper to append a char to the end of a cstring
// takes in a null terminated cstring and a character
// returns nothing, modified cstring in place
void append(char* str, char c);

// Trumps the last character in a cstring (excluding \0)
// Takes in a null terminated cstring
// returns nothing, modified cstring in place
void trump(char* str);

// Talk thread to handle all input and sending to the other client/server
// args: any pointer to data, in thise case it will be thread_data defined in include.c
// returns a DWORD with the error or success of the thread
DWORD WINAPI talk(void* args);

// Listen thread to handle all receiving and processing from the other client/server
// args: any pointer to data, in thise case it will be thread_data defined in include.c
// returns a DWORD with the error or success of the thread
DWORD WINAPI lstn(void* args);

// Used in both client and server to communicate and mange the talk and lstn threads
// rmt_socket is the socket that the application will be communicating with
// returns an int != 0 if an error occured or 0 if successful
int start_chat(SOCKET rmt_sock);

#endif // __INCLUDE_H__