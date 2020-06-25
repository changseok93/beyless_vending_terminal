//
// Created by changseok on 20. 6. 15..
//
#ifndef DAEMON_PROCESS_TERMINAL_H

#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <cstring>
#include <cctype>
#include <thread>
#include <chrono>
#include "mqtt/async_client.h"
#include "mqtt/topic.h"

#include "camera.h"
#include "doorLock.h"
#include "logger.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include <curl/curl.h>
#include </usr/include/mysql/mysql.h>

#define DAEMON_PROCESS_TERMINAL_H

class terminal : public camera, public doorLock
{
private:
    const std::string SERVER_ADDRESS;
    const bool NO_LOCAL = true;

    int QOS;
    std::string user_id;
    std::string topic;

    mqtt::connect_options connOpts;
    mqtt::async_client cli;

    mqtt::topic sub1;
    mqtt::topic sub2;
    mqtt::topic pub1;

    logger log = logger("SYS");

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    char *server = "192.168.10.69";
    char *user = "root";
    char *password = "return123";
    char *database = "test";


public:
    terminal(std::string _SERVER_ADDRESS, int _QOS, std::string _user_id, std::string _topic, int* camera_index, int num, int lock, int door, int trigger);
    void initialize_mqtt_client();
    void initialize_MySQL_database();

    void mqtt_publish(std::string payload);

    void doorLock_daemon();

    bool post_image(std::string json);

    std::string create_response_form(std::string json, char* type, std::string stage, std::string msg, bool result);

    bool database_upload(cv::Mat iter, std::string env_id, std::string type);

};

#endif //DAEMON_PROCESS_TERMINAL_H


