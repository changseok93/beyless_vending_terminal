//
// Created by root on 20. 6. 19..
//

#include "logger.h"

logger::logger(std::string _log_host) {
    this->log_host = _log_host;
}

const std::string logger::currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);

    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

void logger::print_log(std::string log_body) {
    char out_log[1000];
    sprintf(out_log, "[%s][%s] %s", currentDateTime().c_str(), this->log_host.c_str(), log_body.c_str());
    std::cout << out_log << std::endl;
}

