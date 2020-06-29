#include "terminal.h"

terminal::terminal(std::string _SERVER_ADDRESS, int _QOS,
        std::string _user_id, std::string _topic, int* camera_index,
        int num, int lock, int door, int trigger):
        SERVER_ADDRESS(_SERVER_ADDRESS),
        cli(SERVER_ADDRESS, "DEVICE_"+_user_id),
        sub1(cli, _topic, _QOS),
        sub2(cli, "DEVICE_"+_topic, _QOS),
        pub1(cli, "device_operation_vending_web", _QOS),
        camera(camera_index, num),
        doorLock(lock, door, trigger)
{
    this->QOS = _QOS;
    this->user_id = _user_id;
    this->topic = _topic;
}
size_t noop_cb(void *ptr, size_t size, size_t nmemb, void *data) {
    return size * nmemb;
}
bool terminal::post_image(std::string json) {
    rapidjson::Document d;
    d.Parse(json.c_str());

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.10.19:8082/det_conn/grab/upload_images");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: multipart/form-data");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, noop_cb);
        curl_mime *mime;
        curl_mimepart *part;
        mime = curl_mime_init(curl);
        part = curl_mime_addpart(mime);
        curl_mime_name(part, "0.jpg");
        curl_mime_filedata(part,"/home/changseok/Desktop/image0.jpeg");
        part = curl_mime_addpart(mime);
        curl_mime_name(part, "device_id");
        curl_mime_data(part, std::to_string(d["device_id"].GetInt64()).c_str(), CURL_ZERO_TERMINATED);
        part = curl_mime_addpart(mime);
        curl_mime_name(part, "operation_log_id");
        curl_mime_data(part, std::to_string(d["operation_log_id"].GetInt()).c_str(), CURL_ZERO_TERMINATED);
        part = curl_mime_addpart(mime);
        curl_mime_name(part, "stage");
        curl_mime_data(part, "open_door", CURL_ZERO_TERMINATED);
        part = curl_mime_addpart(mime);
        curl_mime_name(part, "user_id");
        curl_mime_data(part, std::to_string(d["user_id"].GetInt()).c_str(), CURL_ZERO_TERMINATED);
        part = curl_mime_addpart(mime);
        curl_mime_name(part, "server_node_id");
        curl_mime_data(part, d["server_node_id"].GetString(), CURL_ZERO_TERMINATED);
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
        res = curl_easy_perform(curl);
        curl_mime_free(mime);
    }
    curl_easy_cleanup(curl);
    if (res == CURLE_OK)
        return true;
    else
        return false;
}

void terminal::initialize_mqtt_client() {
    connOpts.set_keep_alive_interval(1200);
    connOpts.set_mqtt_version(MQTTVERSION_5);
    connOpts.set_clean_start(true);
    connOpts.set_user_name("DEVICE_"+this->user_id);
    connOpts.set_password("DEVICE_"+this->user_id);


    cli.set_connection_lost_handler([this](const std::string&) {
        log.print_log("connection lost");
        exit(2);
    });

    try {
        auto tok = cli.connect(connOpts);
        tok->wait();

        auto subOpts = mqtt::subscribe_options(NO_LOCAL);
        pub1.subscribe(subOpts)->wait();
        log.print_log("publisher connected");

    } catch(const mqtt::exception& exc){
        std::cerr << exc.what() << std::endl;
    }

    cli.set_message_callback([this](mqtt::const_message_ptr msg) {
        if (msg->get_topic() == topic) {
            // 1. Parse a JSON string into DOM.
            std::string json = std::string(msg->to_string());
            rapidjson::Document d;
            d.Parse(json.c_str());

            std::string msg_group_type = d["msg_group_type"].GetString();
            std::string type = d["type"].GetString();

            if (msg_group_type == "cmd"){
                if (type == "grap_image"){
                    grab_frame();
                    save_frame("/home/changseok/Desktop/");
                    post_image(json);
                }
                else if (type == "collect_dataset"){
                    std::string res_form;
                    int64_t image_id;
                    if (is_ready()){
                        door_open();
                        wait_open();
                        wait_close();
                        door_close();
                        grab_frame();
                        std::vector<cv::Mat> images = get_frame();
                        std::vector<cv::Mat>::iterator iter;

                        for(iter = images.begin(); iter != images.end(); iter++){
                            image_id = database_upload(*iter, d["env_id"].GetString(), d["image_type"].GetString());
                        }
                        res_form = create_response_form(json, "ack", "", std::to_string(image_id), true);
                        mqtt_publish(res_form);
                    } else {
                        res_form = create_response_form(json, "ack", "", "", false);
                        mqtt_publish(res_form);
                    }
                }
                else if (type == "open_door"){
                    if (is_ready()){
                        std::string res_form;
                        grab_frame();
                        save_frame("/home/changseok/Desktop/");
                        if(post_image(json))
                            res_form = create_response_form(json, "image_upload", "open_door", "image_upload",true);
                        else
                            res_form = create_response_form(json, "image_upload", "open_door", "image_upload",false);
                        mqtt_publish(res_form);

                        if (door_open())
                            res_form = create_response_form(json, "door_open_close", "open_door", "open_door",true);
                        else
                            res_form = create_response_form(json, "door_open_close", "open_door", "open_door",false);
                        mqtt_publish(res_form);

                        wait_open();
                        wait_close();

                        if (door_close())
                            res_form = create_response_form(json, "door_open_close", "open_close", "open_close",true);
                        else
                            res_form = create_response_form(json, "door_open_close", "open_close", "open_close",false);
                        mqtt_publish(res_form);
                        grab_frame();
                        save_frame("/home/changseok/Desktop/");
                        if(post_image(json))
                            res_form = create_response_form(json, "image_upload", "open_door", "image_upload",true);
                        else
                            res_form = create_response_form(json, "image_upload", "open_door", "image_upload",false);
                        mqtt_publish(res_form);
                    }
                }

                if (type == "exit"){
                    return ;
                }
            }
        }
    });


    try {
        auto subOpts = mqtt::subscribe_options(NO_LOCAL);
        sub1.subscribe(subOpts)->wait();
        log.print_log("subscriber connected");
        sub2.subscribe(subOpts)->wait();
        log.print_log("subscriber connected");

    } catch(const mqtt::exception& exc){
        std::cerr << exc.what() << std::endl;
    }
}

void terminal::initialize_MySQL_database() {
    if( !(conn = mysql_init((MYSQL*)NULL))){
        log.print_log("init fail");
        exit(1);
    }
    log.print_log("mysql initialize success.");

    bool reconnect = 1;
    mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);

    if(!mysql_real_connect(conn, server, user, password, NULL, 3306, NULL, 0)){
        log.print_log("connect error.");
        exit(1);
    }

    log.print_log("mysql connect success");

    if(mysql_select_db(conn, database) != 0){
        mysql_close(conn);
        log.print_log("select db fail");
        exit(1);
    }
    log.print_log("select db success");
}


void terminal::mqtt_publish(std::string payload) {
    pub1.publish(payload);
}




std::string terminal::create_response_form(std::string json, char* type, std::string stage, std::string msg, bool result){
    rapidjson::Document return_form;
    return_form.SetObject();

    rapidjson::Document::AllocatorType& allocator = return_form.GetAllocator();
    size_t sz = allocator.Size();

    rapidjson::Document input_form;
    input_form.Parse(json.c_str());


    std::vector<std::string> json_members;
    if (type == "image_upload"){
        json_members.assign({"msg", "app_type", "device_id", "msg_group_type",
                             "type", "upload_duration","operation_log_id",
                             "stage", "user_id", "msg_id", "ret_code",
                             "server_node_id", "timestamp"});
    }
    else if (type == "door_open_close"){
        json_members.assign({"msg", "operation_log_id", "app_type", "device_id",
                             "user_id", "msg_group_type", "type", "msg_id",
                             "ret_code", "server_node_id", "timestamp"});
    }
    else if (type == "ack"){
        json_members.assign({"msg", "msg_group_type", "type", "ret_code"});
    }
    else
        return NULL;

    for (auto iter = json_members.begin(); iter!= json_members.end(); iter++){
        if (input_form.HasMember(iter->c_str())){
            rapidjson::Value name;
            name.SetString((iter->c_str()), iter->length());
            return_form.AddMember(name, input_form[iter->c_str()], allocator);
        } else {
            if (*iter == "upload_duration"){
                return_form.AddMember("upload_duration", -0, allocator);
            } else if (*iter == "stage") {
                rapidjson::Value str_value;
                str_value.SetString(stage.c_str(), stage.length());
                return_form.AddMember("stage", str_value, allocator);
            } else if (*iter == "msg") {
                rapidjson::Value str_value;
                str_value.SetString(msg.c_str(), msg.length());
                return_form.AddMember("msg", str_value, allocator);
            } else if (*iter == "ret_code") {
                    if (result)
                        return_form.AddMember("ret_code", "0000", allocator);
                    else
                        return_form.AddMember("ret_code", "0001", allocator);
            }
        }
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    return_form.Accept(writer);

    return buffer.GetString();
}

int64_t terminal::database_upload(cv::Mat iter, std::string env_id, std::string type){
    char* log_string = new char [1000];

    //jpeg compression parameters
    std::vector<uchar> buff;
    std::vector<int> param = std::vector<int>(2);
    param[0]=cv::IMWRITE_JPEG_QUALITY;
    param[1]=95;

    //jpeg compression
    imencode(".jpg",iter,buff,param);
    const char* data = reinterpret_cast<char*>(buff.data());
    int length = buff.size();
    sprintf(log_string, "JPEG encoding, size : %d", length);
    log.print_log(log_string);
    delete log_string;
    
    // -----------------------------------------------------------------------------------------------
    char *query,*end;
    
    query = new char [2*length + 1000];
    end = stpcpy(query,"INSERT INTO Image (env_id, data, type, check_num) VALUES('");
    end = stpcpy(end, env_id.c_str());
    end = stpcpy(end, "','");
    end += mysql_real_escape_string(conn,end,data,length);
    end = stpcpy(end,"','");
    end = stpcpy(end, type.c_str());
    end = stpcpy(end, "','");
    end = stpcpy(end, "1')");

    log_string = new char [1000];
    if (mysql_real_query(conn,query,(unsigned int) (end - query)))
    {
        std::string error = mysql_error(conn);
        sprintf(log_string, "Failed to insert row, Error: %s", error.c_str());
        log.print_log(log_string);
        delete log_string;

    } else {
        log.print_log("save image in mysql_server success");
    }

    delete query;
//    delete data;
    // -----------------------------------------------------------------------------------------------
    return mysql_insert_id(conn);
}