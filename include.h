#ifndef __INCLUDE_H__
#define __INCLUDE_H__

#include <stdio.h>
#include <stdlib.h>

#include <Windows.h>
#include <winsock.h>
#include <wincon.h>
#include <conio.h>

// We want to be able to send and receive at the same time, so we
// must have our receive and send on seperate threads. The resource
// these both will be sharing is the terminal. So we will need to have
// some locking in place to make sure we only write after we have finished receiving.
// This means that lets say we have a 'send' action and a 'receive' action
// at the same time. We will check if we have a receive and if we do not then send.
// If we have a receive, then heyo! copy our buffer, output the receive, then send

// Longest message we will send or accept
// It would be good to do a compare of window line length on both client/server
// and then choose the smallest to set for this value so that each message is
// only a line long.
#define MAX_INPUT 512

void append(char* str, char c);
void trump(char* str);

DWORD WINAPI talk(void* args);
DWORD WINAPI lstn(void* args);

int start_chat(SOCKET rmt_sock, struct sockaddr_in* rmt_sock_in);

#endif // __INCLUDE_H__