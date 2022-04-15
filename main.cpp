#include <cstdio>
#include <thread>

#include <zmq.hpp>

#include "CaptureDevice.h"

#define FPS 30

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Invalid # of args\n");
        return -1;
    }

    // create out zmq context
    zmq::context_t zmq_ctx;
    zmq::socket_t sock(zmq_ctx, zmq::socket_type::sub);

    // connect to the socket
    sock.connect("tcp://localhost:42069");
    sock.set(zmq::sockopt::subscribe, "record");
    sock.set(zmq::sockopt::rcvtimeo, 0);

    auto rid = sock.get(zmq::sockopt::subscribe);
    printf("%s", rid.c_str());

    // create our capture devices
//    CaptureDevice capture_device_a(0, 1280, 720, FPS, "/home/homie/Desktop/dev/APPA_record/recordings/output.mkv");
//    CaptureDevice capture_device_b(1, 1280, 720, FPS, "/home/homie/Desktop/dev/APPA_record/recordings/output2.mkv");

    printf("Sleeping until launch signal received\n");

//    auto sleep_for = std::chrono::milliseconds(std::stoi(argv[1]) * 1000);
//    std::this_thread::sleep_for(sleep_for);
    zmq::message_t recv;
    while(true) {
        auto res = sock.recv(recv);
        if (res.has_value()) {
            if (recv.to_string() == "record start")
                break;
        }
    }

    printf("Recording for %s seconds\n", argv[2]);

    // timing vars
    long start_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    long run_time = std::stoi(argv[2]) * 1000;
    auto finished_time = start_time + run_time;

    auto current_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    // start capture
//    capture_device_a.start_capture_thread(run_time);
//    capture_device_b.start_capture_thread(run_time);

    // wait until capture should be done
    while(current_time < finished_time) {
        // print current record time to console
        current_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        printf("\rRecording... %ld seconds", (current_time - start_time) / 1000);

        // check if another process has signaled the record to end
        auto res = sock.recv(recv);
        if (res.has_value()) {
            if (recv.to_string() == "record end")
                break;
        }
    }

    // stop capture
    printf("\nDone capturing\n");
//    capture_device_a.join_capture_thread();
//    capture_device_b.join_capture_thread();

    // release devices
//    capture_device_a.release();
//    capture_device_b.release();

    return 0;
}
