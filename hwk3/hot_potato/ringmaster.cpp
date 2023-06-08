#include <cstdlib>

#include "actions.h"

int main(int args, char * argv[]) {
    if(args != 4) {
        cerr << "Error: the input format of ringmaster must be ( ringmaster <port_num> <num_players> <num_hops> )" << endl;
        return EXIT_FAILURE;
    }
    const char * ringmaster_port = argv[1];
    int player_num = atoi(argv[2]);
    int hops_num = atoi(argv[3]);

    if(player_num <= 1) {
        cerr << "Error: the number of players must be greater than 1" << endl;
        return EXIT_FAILURE;
    }

    if(hops_num < 0 || hops_num > 512) {
        cerr << "Error: the number of hops must be within [0, 512]" << endl;
        return EXIT_FAILURE;
    }

    // init print
    cout << "Potato Ringmaster" << endl;
    cout << "Players = " << player_num << endl;
    cout << "Hops = " << hops_num << endl;

    // init as server
    int ringmaster_sockfd = initServer(ringmaster_port);
    vector<int> rm_connection_player_fds;
    vector<int> rm_connection_player_ports;
    vector<string> rm_connection_player_ipaddrs;

    // connect all players to current ringmaster and get corresponding info
    connectAllPlayers(ringmaster_sockfd, 
                      player_num, 
                      rm_connection_player_fds,
                      rm_connection_player_ports, 
                      rm_connection_player_ipaddrs);
    
    // set the circle of players, let the right neighbor be the server of current player
    // the left neighor be the client of current player  
    setPlayersCycle(player_num, 
                    rm_connection_player_fds, 
                    rm_connection_player_ports, 
                    rm_connection_player_ipaddrs);
 
    // start game
    Potato potato;
    potato.hops_num = hops_num;
    if(hops_num != 0) {
        srand((unsigned int)time(NULL) + player_num);
        int rand_id = rand() % player_num;
        if(int len = send(rm_connection_player_fds[rand_id], &potato, sizeof(potato), 0) == -1) {
            cerr << "Error: errors occur in send" << endl;
            return EXIT_FAILURE;
        }
        cout << "Ready to start the game, sending potato to player " << rand_id << endl;

        // wait until the potato reported from the last player
        keepReceivingPotato(rm_connection_player_fds, potato);
    }
    // either receive potato from the last player
    // or the the original number of hops is 0
    if(potato.hops_num != 0) {
        cerr << "Error: the last potato's num_hops are not 0" << endl;
        return EXIT_FAILURE;
    }
    // // notify players of closing the game
    for(int i = 0; i < rm_connection_player_fds.size(); i++) {
        if(int len = send(rm_connection_player_fds[i], &potato, sizeof(potato), 0) == -1) {
            cerr << "Error: errors occur in send" << endl;
            return EXIT_FAILURE;
        }
    }

    if(potato.path_length != 0) {
        cout << "Trace of potato:" << endl;
        for(int i = 0; i < potato.path_length - 1; i++) {
            cout << potato.path[i] << ',';
        }
        cout << potato.path[potato.path_length - 1] << endl;
    }

    for(int i = 0; i < player_num; i++) {
        close(rm_connection_player_fds[i]);
    }
    close(ringmaster_sockfd);
    return EXIT_SUCCESS;
}