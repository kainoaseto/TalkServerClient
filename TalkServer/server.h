// Kainoa Seto
// Talk Server
// 6-3-17
// A simple win32 TCP socket server to facilitate chat between the server and one client
// server.h

#ifndef __SERVER_H__
#define __SERVER_H__

#include "../include.h"

// Starts the win32 socket server and then attempts to connect to a client on a random port
// Takes nothing in but will output the hostname and port chosen
// It will return a int != 0 if an error occurs
int start_server();

#endif // __SERVER_H__