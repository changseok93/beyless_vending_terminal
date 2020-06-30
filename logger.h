//
// Created by root on 20. 6. 19..
//

#ifndef DAEMON_PROCESS_LOGGER_H
#include <iostream>
#include <string>
#define DAEMON_PROCESS_LOGGER_H


class logger {
private:
    std::string log_host; // log host
public:
    logger(std::string _log_host); // default initializer with log host

    // Get current date/time, format is YYYY-MM-DD.HH:mm:ss
    const std::string currentDateTime();

   	// print log with [log host] and [current time]
    void print_log(std::string log_body);

};


#endif //DAEMON_PROCESS_LOGGER_H
