#ifndef __INCLUDE_H__
#define __INCLUDE_H__

#include <stdio.h>
#include <stdlib.h>


#include <Windows.h>
#include <winsock.h>

// We want to be able to send and receive at the same time, so we
// must have our receive and send on seperate threads. The resource
// these both will be sharing is the terminal. So we will need to have
// some locking in place to make sure we only write after we have finished receiving.
// This means that lets say we have a 'send' action and a 'receive' action
// at the same time. We will check if we have a receive and if we do not then send.
// If we have a receive, then heyo! copy our buffer, output the receive, then send
int talk(SOCKET rmt_sock);
int grab(SOCKET rmt_sock);

#endif // __INCLUDE_H__