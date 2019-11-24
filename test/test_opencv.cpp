#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>

#include "orwell/support/GlobalLogger.hpp"
#include "orwell/game/Game.hpp"

#include "Common.hpp"

TEST(OpenCV, Simple)
{
	cv::Mat aImage = cv::imread("circles.jpg", 0);
	if (aImage.empty())
	{
		FAIL() << "Could not read image";
	}

	cv::Mat aGrayScaleImage;
	cv::medianBlur(aImage, aImage, 5);
	cv::cvtColor(aImage, aGrayScaleImage, cv::COLOR_GRAY2BGR);

	std::vector< cv::Vec3f > aCircles;
	cv::HoughCircles(
			aImage, aCircles, CV_HOUGH_GRADIENT,
			1, aImage.rows/8,  // dp, minDist
			200, 30, 0, 0   // param1, param2, minRadius, maxRadius
			);
	ORWELL_LOG_INFO("Circles detected: " << aCircles.size());
	EXPECT_EQ(size_t{1}, aCircles.size())
		<< "Only one circlue should be found";
	for (auto && aCircle : aCircles)
	{
		ORWELL_LOG_DEBUG(
				"circle of radius " << aCircle[2] <<
				" at (" << aCircle[0] << ", " << aCircle[1] << ")");
		EXPECT_TRUE(abs(197.5 - aCircle[0]) < 1e-5)
			<< "X coordinate";
		EXPECT_TRUE(abs(199.5 - aCircle[1]) < 1e-5)
			<< "Y coordinate";
		EXPECT_TRUE(abs(94.3319 - aCircle[2]) < 1e-4)
			<< "radius " << aCircle[2];
		circle(aGrayScaleImage, cv::Point(aCircle[0], aCircle[1]), aCircle[2], cv::Scalar(0, 0, 255), 3);
		circle(aGrayScaleImage, cv::Point(aCircle[0], aCircle[1]), 2, cv::Scalar(0, 255, 0), 3);
	}

	cv::imwrite("temp.jpg", aImage);
	cv::imwrite("output.jpg", aGrayScaleImage);
}

int main(int argc, char **argv)
{
	return RunTest(argc, argv, "test_opencv");
}
