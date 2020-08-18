#include <iostream>
#include "terminal.h"

// Door lock controll pin numbers.. (up to device setting)
#define LOCK_SENSOR 42
#define DOOR_SENSOR 39
#define LOCK_TRIGGER 12

/*
 * argument vector lists : sudo ./daemon_process [device_id] [cam count] [cam device numbers ....] [ip address]
 */
int main(int argc, char** argv) {
    // user argument parsing
    if (argc < 3){
        std::cout << "not enough user argument." << std::endl <<
        "argument vector lists : sudo ./daemon_process [device_id] [cam count] [cam device numbers ....] [ip address]" << std::endl;
        exit(0);
    }
    std::string device_id = argv[1];
    std::string broker_ip = argv[argc-1];

    //create terminal object and initialize mqtt, MySQl
    terminal T(broker_ip, 0, device_id, device_id, LOCK_SENSOR, DOOR_SENSOR, LOCK_TRIGGER);
    T.initialize_mqtt_client();
    T.initialize_MySQL_connector();
    T.start_daemon();

    //start vending terminal until q pressed
    std::cout << "||||||||||||[START BEYLESS VENDING TERMINAL]||||||||||||" << std::endl;
    std::cout << "press 'q' to exit" << std::endl;
    while(std::tolower(std::cin.get()) != 'q')
        ;

    return 0;
}