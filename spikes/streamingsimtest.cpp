/*
 * The first version of the ImageStream reading experiment
 * using StreamingSimulation.
 *
 * The new version is avtstreamingtest
 */

#include "StreamingSimulation.h"
#include "BlockingWait.h"
#include "ImageStream.h"
#include "TimeMeasure.h"
#include "TimingExperiments.h"
#include <vector>
#include <thread>
#include <opencv2/opencv.hpp>

int main() {

    srand(time(nullptr));

    cv::Mat im1 = cv::imread("../data/robotmac_1.png");
    cv::Mat im2 = cv::imread("../data/robotmac_2.png");

    std::vector<cv::Mat> images1;
    images1.push_back(im1);

    std::vector<cv::Mat> images2;
    images2.push_back(im2);

    ImageStream im_stream_1(80);
    im_stream_1.init(im1.cols, im1.rows, 1);

    ImageStream im_stream_2(80);
    im_stream_2.init(im2.cols, im2.rows, 1);

    BlockingWait bw1;
    BlockingWait bw2;

    StreamingSimulation streaming_sim_1(images1, 20, bw1, im_stream_1);
    StreamingSimulation streaming_sim_2(images2, 20, bw2, im_stream_2);

    std::thread t1(streaming_sim_1);
    std::thread t2(streaming_sim_2);

    ImageResponse resp_1;
    ImageResponse resp_2;

    std::chrono::milliseconds sleep_interval{100};

    TimestampsMatrix image1_query_spans;
    TimestampsMatrix image2_query_spans;

    for (int i = 0; i < 100; i++) {

        std::chrono::milliseconds random_flutuation{rand() % 160 - 80};
        std::chrono::milliseconds total_sleep = sleep_interval + random_flutuation;
        std::this_thread::sleep_for(total_sleep);

        TimePoint t_now = currentTime();
        TimePoint t_im_1;
        TimePoint t_im_2;

        std::vector<TimePoint> getim1_span;
        std::vector<TimePoint> getim2_span;

        TimePoint git0 = currentTime();
        im_stream_1.getImage(t_now, resp_1);
        TimePoint git1 = currentTime();
        std::cout << durationAsString(git1 - git0) << " ";

        im_stream_2.getImage(t_now, resp_2);

        image1_query_spans.push_back(getim1_span);
        image2_query_spans.push_back(getim2_span);

        t_im_1 = resp_1.timestamps_snapshot[resp_1.target_index][0];
        t_im_2 = resp_2.timestamps_snapshot[resp_2.target_index][0];

        std::cout << durationAsString(
                resp_1.timestamps_snapshot[resp_1.target_index][1] - resp_1.timestamps_snapshot[resp_1.target_index][0]
        ) << " ";

        std::cout << "Got image " << i << ": ";
        std::cout << durationAsString(absDuration(t_now, t_im_1)) << ", ";
        std::cout << durationAsString(absDuration(t_now, t_im_2)) << ", ";
        std::cout << durationAsString(absDuration(t_im_1, t_im_2)) << std::endl;

    }

    std::string csv_timestamps;
    csvStringFromTimestampsMatrix(resp_1.timestamps_snapshot, csv_timestamps);
    std::cout << csv_timestamps << std::endl;

    std::cout<< "Press <enter> to stop all the streaming threads...\n" ;
    getchar();
    bw1.notify();
    bw2.notify();

    t1.join();
    t2.join();

    return 0;

}