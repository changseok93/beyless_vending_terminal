#include <iostream>
#include "terminal.h"


/*
 * argument vector lists : sudo ./daemon_process [cam count][cam device numbers ....][ip address]
 */
int main(int argc, char** argv) {
    int* cam_list = new int [atoi(argv[1])];
    for(int i = 0; i < atoi(argv[1]); i++)
        cam_list[i] = atoi(argv[2+i]);
    terminal T(argv[argc-1], 0, "20001", "20001", cam_list, 1, 42, 39, 12);
    T.initialize_mqtt_client();
    T.initialize_MySQL_database();

    std::thread t2(&terminal::initialize_mqtt_client, &T);

    t2.join();
    while(std::tolower(std::cin.get()) != 'q')
        ;

    return 0;
}