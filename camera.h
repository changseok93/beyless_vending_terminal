//
// Created by changseok on 20. 6. 12..
//

#ifndef DAEMON_PROCESS_CAMERA_H
#include <iostream>
//#include <opencv2/opencv.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include <iterator>
#include <chrono>
#include <ctime>
#include <experimental/filesystem>
#include <regex>


#include "logger.h"
#define DAEMON_PROCESS_CAMERA_H

class camera {
private:
    std::vector <cv::VideoCapture> caps; // list of activated video capture devices
    std::vector <int> video_device; //list of video capture device index
    std::vector <cv::Mat> images; // list of latest images from camera
    cv::Mat image; // temper single image

    logger log = logger("CAMERA"); //logger object for print log
public:
    camera(); //defualt initializer, deprecated
    camera(std::string mode, char* prefix_path, std::string regex_grammer);
    camera(int* camera_index, int num); // initializer with number of camera device and it's index. registrate all camera devices

    bool grab_frame(); // grab image from camera devices
    std::vector<cv::Mat> get_frame(); // return latest taken images from camera
    int get_image_count();

    bool save_frame(char* PATH); //save image in PATH
};


#endif //DAEMON_PROCESS_CAMERA_H
