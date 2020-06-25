//
// Created by changseok on 20. 6. 12..
//

#include "camera.h"

// default camera class initializer, only for debuggin, deprecated
camera::camera(){
    //default Video_device is 0
    video_device.push_back(0);

    cv::VideoCapture _cap;
    _cap.open("/dev/video0");

    caps.push_back(_cap);
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

    // grab images from predefined capture device
    try{
        std::vector<cv::VideoCapture>::iterator iter;
        for(iter = caps.begin(); iter != caps.end(); iter++){
            iter->set(CV_CAP_PROP_FRAME_WIDTH, 1920);
            iter->set(CV_CAP_PROP_FRAME_HEIGHT, 1080);

            // for stable output quality, get 5 frames and use last one
            for(int i = 0; i<5; i++)
                *iter >> image;
            images.push_back(image);
        }
    } catch (cv::Exception& e) {
        log.print_log(("EXCEPTION CAUGHT : " + std::string(e.what())));
        return false;
    }

    log.print_log("GET FRAME");
    return true;
}

std::vector<cv::Mat> camera::get_frame(){
    return images;
}


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
    log.print_log("SAVE IMAGE TO " + std::string(PATH));
    return true;
}
