//
// Created by changseok on 20. 6. 12..
//

#include "camera.h"

// default camera class initializer, only for debuggin, deprecated
camera::camera(){
    //default Video_device is 0
    video_device.push_back(0);

    cv::VideoCapture _cap;
    _cap.open("/dev/video0", CV_CAP_V4L);

    caps.push_back(_cap);
}

camera::camera(std::string mode, char* prefix_path, std::string regex_grammer) {

    char* string_buffer;
    // if mode is not in auto detect, call default initializer
    std::cout << mode  << std::endl;
    if (mode != "auto_detect"){
        //default Video_device is 0
        video_device.push_back(0);

        cv::VideoCapture _cap;
        _cap.open("/dev/video0", CV_CAP_V4L);


        caps.push_back(_cap);
    }
    else {
        std::regex re (regex_grammer.c_str());
        std::smatch match;

        for (auto & entry : std::experimental::filesystem::directory_iterator(prefix_path)){
            std::string str = entry.path().string();
            if (std::regex_search(str, match, re, std::regex_constants::match_default)){
                string_buffer = new char[50];
                sprintf(string_buffer, "camera device found on port number : %s", match[1].str().c_str());
                log.print_log(string_buffer);
                delete string_buffer;

                video_device.push_back(std::stoi(match[1].str()));
                //cv::VideoCapture _cap(prefix_path + match.str(), CV_CAP_V4L);
//                _cap.open(());
                caps.emplace_back(cv::VideoCapture(prefix_path + match.str(), CV_CAP_V4L));

            }
        }
    }
}

// camera class initializer, gets number of camera devices and list of video indexs
camera::camera(int* camera_index, int num){
    //hard copy
    for(int i = 0; i < num; i++){
        video_device.push_back(camera_index[i]);
        cv::VideoCapture _cap;
        _cap.open(camera_index[i]);
        caps.push_back(_cap);
    }
}

// grab frame from camera devices.
bool camera::grab_frame() {
    // flush images container
    images.clear();

    // grab images from predefined capture devices
    try{
        std::vector<cv::VideoCapture>::iterator iter;
        for(iter = caps.begin(); iter != caps.end(); iter++){
            iter->set(CV_CAP_PROP_FRAME_WIDTH, 1920);
            iter->set(CV_CAP_PROP_FRAME_HEIGHT, 1080);

            // for stable output quality, get 5 frames and use last one
            cv::Mat img;
            for(int i = 0; i<5; i++)
                *iter >> img;
            cv::Rect bounds(0,0,img.cols,img.rows);
            cv::Rect r(330,0,1290,1080); // partly outside
            cv::Mat roi = img( r & bounds ); // cropped to fit image

            images.push_back(roi);
            roi.release();
        }
    } catch (cv::Exception& e) {
        log.print_log(("EXCEPTION CAUGHT : " + std::string(e.what())));
        return false;
    }

    log.print_log("GET FRAME");
    return true;
}

// return images
std::vector<cv::Mat> camera::get_frame(){
    return images;
}

// save images in _PATH/image#.jpeg
bool camera::save_frame(char* _PATH){
    int image_counter = 0;
    std::vector<cv::Mat>::iterator iter;
    char PATH[100];
    for (iter = images.begin(); iter!= images.end(); iter++){
        sprintf(PATH, "%simage%d.jpeg", _PATH, image_counter++);

        try{
            cv::imwrite(PATH, *iter);
        } catch (cv::Exception& e){
            log.print_log(("EXCEPTION CAUGHT : " + std::string(e.what())));
            return false;
        }
    }
    log.print_log("SAVE IMAGE TO " + std::string(_PATH));
    return true;
}

int camera::get_image_count() {

    return video_device.size();

}
