//
// Created by changseok on 20. 6. 12..
//

#ifndef DAEMON_PROCESS_CAMERA_H
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <iterator>
#include <chrono>
#include <ctime>

#include "logger.h"
#define DAEMON_PROCESS_CAMERA_H

class camera {
private:
    std::vector <cv::VideoCapture> caps;
    std::vector <int> video_device;
    std::vector <cv::Mat> images;
    cv::Mat image;

    logger log = logger("CAMERA");
public:
    camera();
    camera(int* camera_index, int num);

    bool grab_frame();
    std::vector<cv::Mat> get_frame();

    bool save_frame(char* PATH);
};


#endif //DAEMON_PROCESS_CAMERA_H
