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
		#if defined(_WIN32) || defined(_WIN64)
			cv::VideoCapture v("../../../dnd-cpippi/imgs/gridtette.mp4");
		#endif

		#if defined(__APPLE__) || defined(__MACH__)
			cv::VideoCapture v("../imgs/gridtette.mp4");
		#endif

		v.read(bgGridImage); 
	}
}


// * for debug
void Kitchen::showImg(cv::Mat& img) {
	cv::imshow("Image", img);
	cv::waitKey(0);
}


void Kitchen::letHimCook() {
	cv::Mat frame;
	int frameCount = 0;
	//cv::Mat state = bgColorImage; // volevo usarlo per la roba di isChanged ma poi niente
	for (int i = 0; i < 2; i++) {
		defineImgsFromVideo(i);
	}
	#if defined(_WIN32) || defined(_WIN64)
		cv::VideoCapture v("../../../dnd-cpippi/imgs/tuttotette.mp4");
	#endif
	#if defined(__APPLE__) || defined(__MACH__)
		cv::VideoCapture v("../imgs/tuttotette.mp4");
	#endif
	video = v;
	//frameImage = bgGridImage;
	frameState = bgGridImage;
	double thresholdSmallChanges = 0.05; 
	grid.initPreprocess(bgGridImage, bgColorImage);

	// * loop over the recording
	// ! this should be the recording of the camera
	bool isChanged;
	while (true) {
		//video >> frame;
		//if (frame.empty()) { video.release(); break; }
		video.read(frame);
		if (!video.read(frame)) { video.release(); break; }
 
		cv::imshow("", frameState);

		isChanged = grid.getPreprocess().isChanged(frameState, frame, thresholdSmallChanges);		
		if (isChanged) { 
			std::cout << "changed ✅" << std::endl;
			frameState = frame.clone();	
			grid.preprocessEntities(frame, bgGridImage); //frameImage
			grid.findElementsInLine(frame);
		}
		
		//
		//// Show frame number
		std::string label = "Frame: " + std::to_string(frameCount++);
		cv::putText(frame, label, { 10, 30 }, cv::FONT_HERSHEY_SIMPLEX, 1.0, { 0, 255, 0 }, 2);
		
		// cv::imshow("Frame-by-Frame Viewer", frame);
		//cv::waitKey(0);

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
	// * 3. loop di frame
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
	grid.preprocessEntities(frame, bgGridImage); //frameImage
	grid.findElementsInLine(frame);
	return false;
}