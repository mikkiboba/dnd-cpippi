#include "Kitchen.h"


Kitchen::~Kitchen() {}


void Kitchen::defineImgs(bool quack) {

	#if defined(_WIN32) || defined(_WIN64)
        bgColorImage = cv::imread("../../../dnd-cpippi/imgs/background.jpg");
		//frame2 = cv::imread("../../../dnd-cpippi/imgs/backgroundGridPiedini.png");
		frameImage = cv::imread("../../../dnd-cpippi/imgs/backgroundGridPiedini2.png");
		bgGridImage = cv::imread("../../../dnd-cpippi/imgs/backgroundGrid.png");
	#endif

	#if defined(__APPLE__) || defined(__MACH__)
        bgColorImage = cv::imread("../imgs/background.jpg");
		//frame2 = cv::imread("../imgs/backgroundGridPiedini.png");
		frameImage = cv::imread("../imgs/backgroundGridPiedini2.png");
		bgGridImage = cv::imread("../imgs/backgroundGrid.png");
	#endif

	if (bgColorImage.empty() || frameImage.empty() || bgGridImage.empty()) {
		std::cerr << "Could not load images!" << std::endl;
	}

}

void Kitchen::defineImgsFromVideo(int n) {
	if (n == 0) { 
		video.read(bgColorImage); 
	}
	if (n == 1) { 
		cv::VideoCapture v("../../../dnd-cpippi/imgs/gridtette.mp4");
		v.read(bgGridImage); 
	}
}


// * for debug
void Kitchen::showImg(cv::Mat& img) {
	cv::imshow("Image", img);
	cv::waitKey(0);
}


void Kitchen::letHimCook() {
	// * 1. frame del piano di gioco
	// -> proietta griglia
	// * 2. frame del piano di gioco + griglia
	cv::Mat frame;
	int frameCount = 0;
	//cv::Mat state = bgColorImage; // volevo usarlo per la roba di isChanged ma poi niente
	for (int i = 0; i < 2; i++) {
		defineImgsFromVideo(i);
	}
	cv::VideoCapture v("../../../dnd-cpippi/imgs/tuttotette.mp4");
	video = v;
	v.read(frameImage);
	double thresholdSmallChanges = 0.05; 
	grid.initPreprocess(bgGridImage, bgColorImage);
	while (true) {
		
		video >> frame;
		if (frame.empty()) { video.release(); break; }
		
		if (grid.getPreprocess().isChanged(frameImage, frame, thresholdSmallChanges)) { 
			std::cout << "is changed" << std::endl;
			grid.preprocessEntities(frame, bgGridImage); //frameImage
			grid.findElementsInLine(frame);
		}
		
		//
		//// Show frame number
		std::string label = "Frame: " + std::to_string(frameCount++);
		cv::putText(frame, label, { 10, 30 }, cv::FONT_HERSHEY_SIMPLEX, 1.0, { 0, 255, 0 }, 2);
		cv::imshow("Frame-by-Frame Viewer", frame);
		cv::waitKey(0);

		char key = (char)cv::waitKey(30);
		if (key == 27 || key == 'q') break; // ESC or 'q' to quit
		
		//char key = cv::waitKey(0); // 0 = wait forever
	}

	video.release();
	cv::destroyAllWindows();
	//grid.initPreprocess(bgGridImage, bgColorImage);
	//// ...
	//// ...
	//// -> proietta resto (opzionale)
	//showImg(grid.getPreprocess().getFgMask());
	//bool isJoever = false;
	//// * 3. loop di frame
	//while (!isJoever) {
	//	isJoever = cooking();
	//}
	//// -> analizziamo i frame per frame
	//// cooking()
	//grid.printEntities();
}


bool Kitchen::cooking() {
	cv::Mat frame;
	if (!video.read(frame)) { return true; }
	showImg(frame);
	grid.preprocessEntities(frame, bgGridImage); //frameImage
	grid.findElementsInLine(frame);
	return false;
}