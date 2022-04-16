#include <cstdio>
#include <thread>

#ifdef ZMQ_ENABLED
#include <zmq.hpp>
#endif

#include "CaptureDevice.h"

#define FPS 24

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Invalid # of args\n");
        return -1;
    }

#ifdef ZMQ_ENABLED
    // create out zmq context
    zmq::context_t zmq_ctx;
    zmq::socket_t sock(zmq_ctx, zmq::socket_type::sub);

    // connect to the socket
    sock.connect("tcp://localhost:42069");
    sock.set(zmq::sockopt::subscribe, "record");
    sock.set(zmq::sockopt::rcvtimeo, 0);
    sock.set(zmq::sockopt::connect_timeout, 0);
    // create our capture devices
    CaptureDevice capture_device_a(0, 1280, 720, FPS, "../recordings/output.mkv");

    printf("Sleeping until launch signal received\n");
#else
    CaptureDevice capture_device_b(1, 1280, 720, FPS, "../recordings/output2.mkv");
#endif

#ifdef ZMQ_ENABLED
    zmq::message_t recv;
    while(true) {
        auto res = sock.recv(recv);
        if (res.has_value()) {
            if (recv.to_string() == "record start")
                break;
        }
    }
#else
    printf("Sleeping for %d seconds until recording\n", std::stoi(argv[1]));
    auto sleep_for = std::chrono::milliseconds(std::stoi(argv[1]) * 1000);
    std::this_thread::sleep_for(sleep_for);
#endif

    printf("Recording for %s seconds\n", argv[2]);

    // timing vars
    long start_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    long run_time = std::stoi(argv[2]) * 1000;
    auto finished_time = start_time + run_time;

    auto current_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    // start capture
#ifdef ZMQ_ENABLED
    capture_device_a.start_capture_thread(run_time);
#else
    capture_device_b.start_capture_thread(run_time);
#endif

    // wait until capture should be done
    while(current_time < finished_time) {
        // print current record time to console
        current_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        if (current_time - start_time % 1000 == 0)
            printf("\rRecording... %ld seconds", (current_time - start_time) / 1000);

#ifdef ZMQ_ENABLED
        // check if another process has signaled the record to end
        auto res = sock.recv(recv);
        if (res.has_value()) {
            if (recv.to_string() == "record end") {
                printf("Received stop signal.\n");
                break;
            }
        }
#endif
    }

    // stop capture
    printf("\nDone capturing\n");
#ifdef ZMQ_ENABLED
    capture_device_a.join_capture_thread();
#else
    capture_device_b.join_capture_thread();
#endif

    // release devices
#ifdef ZMQ_ENABLED
    capture_device_a.release();
    sock.close();
#else
    capture_device_b.release();
#endif

    return 0;
}
