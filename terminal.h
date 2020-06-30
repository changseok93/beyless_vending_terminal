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
    // mqtt broker address with port number
    const std::string SERVER_ADDRESS;
    // mqtt client optional parameter
    const bool NO_LOCAL = true;

    // MQTT client QOS (will be modified)
    int QOS;
    // MQTT client options...
    mqtt::connect_options connOpts;
    std::string user_id;
    std::string topic;
    
    // MQTT client object
    mqtt::async_client cli;
    // MQTT client publish, subscribe objects
    mqtt::topic sub1;
    mqtt::topic sub2;
    mqtt::topic pub1;

    // logger for SYSTEM terminal
    logger log = logger("SYS");

    // MYSQL connector object and return handler
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    // MYSQL options 
    char *server = "192.168.10.69";
    char *user = "root";
    char *password = "return123";
    char *database = "test";


public:
    // terminal class initiaizer, also initialize all inherited classes.
    terminal(std::string _SERVER_ADDRESS, int _QOS, std::string _user_id, std::string _topic, int* camera_index, int num, int lock, int door, int trigger);
    // initialize MQTT client, all publisher & subscriber
    void initialize_mqtt_client();
    //initialize MYSQL database
    void initialize_MySQL_database();

    // mqsql publish given string type payload (will be modified)
    void mqtt_publish(std::string payload);

    // doorLock_daemon, deprecated
    void doorLock_daemon();

    // post image using HTTP protocol
    bool post_image(std::string json);

    // create json type response form
    std::string create_response_form(std::string json, char* type, std::string stage, std::string msg, bool result);

    // upload image to database
    int64_t database_upload(cv::Mat iter, std::string env_id, std::string type);

};

#endif //DAEMON_PROCESS_TERMINAL_H


