#include <cstdlib>
#include <assert.h>

#include "actions.h"

int main(int args, char * argv[]) {
    if(args != 3) {
        cerr << "Error: the input format of player must be ( player <machine_name> <port_num> )" << endl;
        return EXIT_FAILURE;
    }

    const char * hostname = argv[1];
    const char * port = argv[2];
    // init as client receiving information from ringmaster
    int connect_rm_sockfd = initClient(hostname, port);

    // get id and the number of players from ringmaster
    int id, player_num;
    if(int len = recv(connect_rm_sockfd, &id, sizeof(id), MSG_WAITALL) == -1) {
        cerr << "Error: errors occur in recv" << endl;
        return EXIT_FAILURE;
    }
    if(int len = recv(connect_rm_sockfd, &player_num, sizeof(player_num), MSG_WAITALL) == -1) {
        cerr << "Error: errors occur in recv" << endl;
        return EXIT_FAILURE;
    }
    cout<<"Connected as player " << id << " out of " << player_num << " total players"<<endl;

    // init as server to do the server job for ringmaster and other players
    int as_server_sockfd = initServer("");
    int as_server_port = getPortNum(as_server_sockfd);
    if(int len = send(connect_rm_sockfd, &as_server_port, sizeof(as_server_port), 0) == -1) {
        cerr << "Error: errors occur in send" << endl;
        return EXIT_FAILURE;       
    }

    // init as client to its right neighbor
    char rneighbor_addr[100];
    int rneighbor_port_int;
    if(int len = recv(connect_rm_sockfd, &rneighbor_addr, sizeof(rneighbor_addr), MSG_WAITALL) == -1) {
        cerr << "Error: errors occur in recv" << endl;
        return EXIT_FAILURE;
    }
    if(int len = recv(connect_rm_sockfd, &rneighbor_port_int, sizeof(rneighbor_port_int), MSG_WAITALL) == -1) {
        cerr << "Error: errors occur in recv" << endl;
        return EXIT_FAILURE;
    }
    char rneighbor_port[9];
    memset(rneighbor_port, 0, sizeof(rneighbor_port));
    string temp = to_string(rneighbor_port_int);
    strcpy(rneighbor_port, temp.c_str());
    int connect_rneighbor_sockfd = initClient(rneighbor_addr, rneighbor_port);
    int right_neighbor_id = (id + 1) % player_num;

    // init as server to its left neighbor
    string left_neighbor_addr;
    int connect_lneighbor_sockfd = serverAccept(as_server_sockfd, &left_neighbor_addr);
    int left_neighbor_id = (id + player_num - 1) % player_num;


    vector<int> all_connection_sockfds;
    all_connection_sockfds.push_back(connect_rm_sockfd);
    all_connection_sockfds.push_back(connect_rneighbor_sockfd);
    all_connection_sockfds.push_back(connect_lneighbor_sockfd);

    Potato potato;
    while(true) {
        keepReceivingPotato(all_connection_sockfds, potato);
        // get the notification from ringmaster to close the game
        if(potato.hops_num == 0) {
            break;
        }
        potato.path[potato.path_length++] = id;
        potato.hops_num--;
        // no hops, I'm it
        if(potato.hops_num == 0) {
            cout << "I'm it" << endl;
            if(int len = send(connect_rm_sockfd, &potato, sizeof(potato), 0) == -1) {
                cerr << "Error: errors occur in send" << endl;
                return EXIT_FAILURE;
            }
        }
        else {
            srand((unsigned int)time(NULL) + potato.path_length);
            int rand_id = rand() % 2;
            // send to left neighbor
            if(rand_id == 0) {
                cout << "Sending potato to " << left_neighbor_id << endl;
                if(int len = send(connect_lneighbor_sockfd, &potato, sizeof(potato), 0) == -1) {
                    cerr << "Error: errors occur in send" << endl;
                    return EXIT_FAILURE;
                }
            }
            // send to right neighbor
            else {
                cout << "Sending potato to " << right_neighbor_id << endl;
                if(int len = send(connect_rneighbor_sockfd, &potato, sizeof(potato), 0) == -1) {
                    cerr << "Error: errors occur in send" << endl;
                    return EXIT_FAILURE;
                }
            }
        }
    }

    for(int i = 0; i < all_connection_sockfds.size(); i++) {
        close(all_connection_sockfds[i]);
    }
    close(as_server_sockfd);
    return EXIT_SUCCESS;
}