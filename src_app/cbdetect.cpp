#include "DriverAVT/AVTExperiments.h"
#include "ProcessingTask.h"
#include "VisionAlg/CBCalib.h"
#include <vector>
#include <functional>

using CBCResults = std::vector<cv::Point2f>;
using ProcessingFunction = std::function<bool(cv::Mat, ExtendedImageStream<CBCResults>&, CBCResults&)>;

ProcessingFunction get_cbc_func(int width, int height, const std::string& window_name) {

    cv::Size pattern_size_wh{width, height};

    return [pattern_size_wh, window_name](cv::Mat image, ExtendedImageStream<CBCResults>& image_stream, CBCResults& res) {
        bool found = findCBC(image, pattern_size_wh, res);
        if (found) {

            cv::imshow(window_name, image);
            cv::waitKey(1);

        }

        return found;
    };

}

int main() {

    const int N_FRAMES = 3;
    const int STREAM_SIZE = 10;

    VimbaSystem& sys = VimbaSystem::GetInstance();

    VmbErrorType err;

    err = sys.Startup();
    if (err != VmbErrorSuccess) {
        std::cout << "Failed to start up. Shutting down\n";
        sys.Shutdown();
        return -1;
    }

    BlockingWait bw1;
    BlockingWait bw2;

    ExtendedImageStream<CBCResults> image_stream_1(STREAM_SIZE);
    ExtendedImageStream<CBCResults> image_stream_2(STREAM_SIZE);

    ProcessingFunction f1 = get_cbc_func(15, 9, "Camera 1");
    ProcessingFunction f2 = get_cbc_func(15, 9, "Camera 2");
    cv::namedWindow("Camera 1", cv::WINDOW_NORMAL);
    cv::namedWindow("Camera 2", cv::WINDOW_NORMAL);
    cv::resizeWindow("Camera 1", 640, 480);
    cv::resizeWindow("Camera 2", 640, 480);

    TypedProcessingTask<CBCResults> task_1(image_stream_1, f1);
    TypedProcessingTask<CBCResults> task_2(image_stream_2, f2);

    AVTStreaming cam1_streaming(0, N_FRAMES, image_stream_1, task_1, bw1);
    AVTStreaming cam2_streaming(1, N_FRAMES, image_stream_2, task_2, bw2);

    std::thread t1(cam1_streaming);
    std::thread t2(cam2_streaming);

    std::cout<< "Press <enter> to stop all the streaming threads...\n" ;
    getchar();
    bw1.notify();
    bw2.notify();

    t1.join();
    t2.join();

    sys.Shutdown();
    std::cout << "Shutting down normally\n";

    return 0;
}