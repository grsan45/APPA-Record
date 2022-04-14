//
// Created by grsan on 3/22/22.
//

#include "CaptureDevice.h"
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

CaptureDevice::CaptureDevice(int device_id, int width, int height, unsigned int framerate, const std::string& output_name) {
    //create our pipeline string
    std::string gstreamer_pipeline = format(std::string("nvarguscamerasrc sensor-id=%d !"
                                                        "video/x-raw(memory:NVMM), width=(int)%d, height=(int)%d, fps=(fraction)%d/1 ! "
                                                        "nvvidconv flip-method=%d ! "
                                                        "video/x-raw, width=(int)%d, height=(int)%d, format=(string)BGRx ! "
                                                        "videoconvert ! "
                                                        "video/x-raw, format=(string)BGR ! appsink"), device_id, width, height, framerate, 0, width, height);

    capture = cv::VideoCapture(gstreamer_pipeline);
    writer = cv::VideoWriter(output_name, cv::VideoWriter::fourcc('H','2','6','4'), framerate, cv::Size2i(width, height));

    fps = framerate;
    finished = false;
    killed = false;

    if (!capture.isOpened())
        throw std::runtime_error(format("Capture device %d failed to open", device_id));

    if (!writer.isOpened())
        throw std::runtime_error(format("Writer %d failed to open", device_id));

    image_dir = format(std::string("../recordings/%d/"), device_id);
    if (!fs::exists(image_dir)) {
        fs::create_directory(image_dir);
    }

    for (auto& file : fs::directory_iterator(image_dir))
        fs::remove_all(file.path());
}

void CaptureDevice::start_capture_thread(unsigned int run_time) {
    capture_thread = std::thread([run_time, this]()
                {
                    auto start_time = std::chrono::steady_clock::now();
                    auto currentTime= std::chrono::steady_clock::now();

                    unsigned int interval = (int)(1/(float)this->fps*1000);

                    while(currentTime < start_time + std::chrono::milliseconds(run_time)) {
                        // if an error occurs, go ahead and stop this thread
//                        if (!capture.isOpened()) {
//                            this->join_capture_thread();
//                            break;
//                        }

                        if (this->finished)
                            break;

                        currentTime = std::chrono::steady_clock::now();

                        auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);

                        cv::Mat frame;
                        this->capture.read(frame);
//                        this->writer.write(frame);
//                        cv::imwrite(format(std::string("../recordings/frame-%d.jpg"), frameNum), frame);
                        this->frame_queue.push(frame);

                        std::this_thread::sleep_until(x);
                    }
                });
    printf("Staring writer thread\n");
    start_write_thread();
}

void CaptureDevice::start_write_thread() {
    write_thread = std::thread([this]()
               {
                    int frame_num = 0;
                    while(!frame_queue.empty() || !finished) {
                        if (killed)
                            break;

                        if (frame_queue.empty())
                            continue;

                        cv::Mat frame = frame_queue.front();

                        if (frame.empty())
                            continue;

                        cv::imwrite(format(image_dir + "/frame-%d.png", frame_num), frame);

                        frame.release();
                        frame_queue.pop();
                        frame_num++;
                    }
                    for (int i = 0; i < frame_num; i++) {
                        cv::Mat frame = cv::imread(format(image_dir + "/frame-%d.png", i));
                        writer.write(frame);
                        frame.release();
                    }
                    printf("Done writing.\n");
               });
}

void CaptureDevice::join_capture_thread() {
    printf("Stopping capture thread\n");
    finished = true;
    capture_thread.join();
    write_thread.join();
}

void CaptureDevice::kill() {
    killed = true;
    capture_thread.join();
    write_thread.join();
}

void CaptureDevice::release() {
    printf("Releasing capture & writer\n");
    capture.release();
    writer.release();
}
