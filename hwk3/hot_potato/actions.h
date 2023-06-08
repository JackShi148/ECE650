#ifndef __ACTIONS_H__
#define __ACTIONS_H__

#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <string.h>
#include <vector>
#include <algorithm>
#include <cstdlib>

#include "potato.h"

using namespace std;

int initServer(const char * port);

int initClient(const char * hostname, const char * port);

int serverAccept(int socket_fd, string * ipaddr);

int getPortNum(int socket_fd);

void keepReceivingPotato(vector<int> socket_fds, Potato & potato);

void connectAllPlayers(int ringmaster_fd, int player_num, vector<int> &player_fds, vector<int> &player_ports, vector<string> &addrs);

void setPlayersCycle(int player_num, vector<int> &player_fds, vector<int> &player_ports, vector<string> &addrs);
#endif