#include "Kitchen.h"

int main() {

    cv::VideoCapture cap("../../../dnd-cpippi/imgs/bgtette.mp4"); // open the video file
    if (!cap.isOpened())  // check if we succeeded
        std::cerr << "Can not open Video file" << std::endl;
    else std::cout << "Video opened " << std::endl;

    Kitchen videoKitchen = Kitchen(cap);
    //Kitchen k = Kitchen();
    videoKitchen.letHimCook();

    return 0;
}