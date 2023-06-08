#include "actions.h"

int initServer(const char *port)
{
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    const char *hostname = NULL;

    memset(&host_info, 0, sizeof(host_info));

    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags = AI_PASSIVE;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0)
    {
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } // if

    socket_fd = socket(host_info_list->ai_family,
                       host_info_list->ai_socktype,
                       host_info_list->ai_protocol);
    if (socket_fd == -1)
    {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } // if

    int yes = 1;
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1)
    {
        cerr << "Error: cannot bind socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } // if

    status = listen(socket_fd, 100);
    if (status == -1)
    {
        cerr << "Error: cannot listen on socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } // if

    freeaddrinfo(host_info_list);
    return socket_fd;
}

int initClient(const char *hostname, const char *port)
{
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;

    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0)
    {
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } // if

    socket_fd = socket(host_info_list->ai_family,
                       host_info_list->ai_socktype,
                       host_info_list->ai_protocol);
    if (socket_fd == -1)
    {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } // if

    // cout << "Connecting to " << hostname << " on port " << port << "..." << endl;

    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1)
    {
        cerr << "Error: cannot connect to socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } // if

    freeaddrinfo(host_info_list);
    return socket_fd;
}

int serverAccept(int socket_fd, string *ipaddr)
{
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int client_connection_fd;
    client_connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (client_connection_fd == -1)
    {
        cerr << "Error: cannot accept connection on socket" << endl;
        return -1;
    }
    struct sockaddr_in * skaddr = (struct sockaddr_in *)&socket_addr;
    *ipaddr = inet_ntoa(skaddr->sin_addr);
    return client_connection_fd;
}

int getPortNum(int socket_fd) {
    struct sockaddr_in skaddr;
    socklen_t skaddr_len = sizeof(skaddr);
    if(getsockname(socket_fd, (struct sockaddr *)&skaddr, &skaddr_len) == -1) {
        cerr << "Error: cannot get corresponding sockname" << endl;
    }
    // convert NBO to HBO
    // sin_port is uint16_t, so ntohs is enough
    return ntohs(skaddr.sin_port);
}

void keepReceivingPotato(vector<int> socket_fds, Potato & potato) {
    fd_set readfds;
    FD_ZERO(&readfds);
    int max_fd = *max_element(socket_fds.begin(), socket_fds.end());
    for(int i = 0; i < socket_fds.size(); i++) {
        FD_SET(socket_fds[i], &readfds);
    }
    select(max_fd + 1, &readfds, NULL, NULL, NULL);
    for(int i = 0; i < socket_fds.size(); i++) {
        if(FD_ISSET(socket_fds[i], &readfds)) {
            int len = recv(socket_fds[i], &potato, sizeof(potato), MSG_WAITALL);
            if(len == -1) {
                exit(EXIT_FAILURE);
            }
            break;
        } 
    }
}

void connectAllPlayers(int ringmaster_fd, int player_num, vector<int> &player_fds, vector<int> &player_ports, vector<string> &addrs) {
    for(int i = 0; i < player_num; i++) {
        string player_ipaddr;
        // ringmaster(rm) accept player connection
        int rm_connection_player_fd = serverAccept(ringmaster_fd, &player_ipaddr);
        //notify player of player number and its id
        int id = i;
        if(int len = send(rm_connection_player_fd, &id, sizeof(id), 0) == -1) {
            cerr << "Error: errors occur in snen" << endl;
            exit(EXIT_FAILURE);
        }
        if(int len = send(rm_connection_player_fd, &player_num, sizeof(player_num), 0) == -1) {
            cerr << "Error: errors occur in send" << endl;
            exit(EXIT_FAILURE);
        }
        // receive port from connected player
        // prepare for other players or ringmaster to communicate with it
        int player_port;
        if(int len = recv(rm_connection_player_fd, &player_port, sizeof(player_port), 0) == -1) {
            cerr << "Error: errors occur in recv" << endl;
            exit(EXIT_FAILURE);
        }
        player_fds.push_back(rm_connection_player_fd);
        player_ports.push_back(player_port);
        addrs.push_back(player_ipaddr);
        cout<<"Player "<< id <<" is ready to play"<<endl;
    }
}

void setPlayersCycle(int player_num, vector<int> &player_fds, vector<int> &player_ports, vector<string> &addrs) {
    // let the current player's right neighbor be its server
    for(int i = 0; i < player_num; i++) {
        int neighbor_id = (i + 1) % player_num;
        int neighbor_port = player_ports[neighbor_id];
        string neighbor_addr_str = addrs[neighbor_id];
        char neighbor_addr[100];
        memset(neighbor_addr, 0, sizeof(neighbor_addr));
        strcpy(neighbor_addr, neighbor_addr_str.c_str());
        // send ip and port to the current players
        if(int len = send(player_fds[i], &neighbor_addr, sizeof(neighbor_addr), 0) == -1) {
            cerr << "Error: errors occur in send" << endl;
            exit(EXIT_FAILURE);
        }
        if(int len = send(player_fds[i], &neighbor_port, sizeof(neighbor_port), 0) == -1) {
            cerr << "Error: errors occur in send" << endl;
            exit(EXIT_FAILURE);
        }
    } 
}