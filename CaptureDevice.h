//
// Created by grsan on 3/22/22.
//

#include <thread>
#include <opencv4/opencv2/opencv.hpp>
#include <atomic>

#include "utils.h"

#ifndef APPA_RECORD_CAPTURE_DEVICE_H
#define APPA_RECORD_CAPTURE_DEVICE_H


class CaptureDevice {
public:
    CaptureDevice(int device_id, int width, int height, unsigned int framerate, const std::string& output_name);
    void start_capture_thread(unsigned int run_time);
    void join_capture_thread();
    void kill();
    void release();

private:
    unsigned int fps;
    std::thread capture_thread;
    std::thread write_thread;

    cv::VideoCapture capture;
    cv::VideoWriter writer;

    std::atomic<bool> finished;
    std::atomic<bool> killed;
    std::queue<cv::Mat> frame_queue;

    std::string image_dir;

    void start_write_thread();
};


#endif //APPA_RECORD_CAPTURE_DEVICE_H
