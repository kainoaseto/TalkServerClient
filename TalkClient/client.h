// Kainoa Seto
// Talk Client
// 6-3-17
// A simple win32 TCP client to connect and talk to a talk server
// client.h

#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "../include.h"

// Start tcp client with hostname and port. After connection is established will begin talking
// Hostname is optional by setting to "" while port is required
// returns a int != 0 if there is a failure or error
int start_client(char* hostname, USHORT port);

#endif //__CLIENT_H__
