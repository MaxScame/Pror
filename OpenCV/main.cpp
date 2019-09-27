#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>

#include "Request.h"

#define MAX_CORNERS 505
#define FPS(x) int(1000/int(x))

using namespace cv;
using namespace std;



namespace cvvv
{
	Point2f abs(Point2f point)
	{
		Point2f temp;
		temp.x = fabs(float(point.x));
		temp.y = fabs(float(point.y));
		return temp;
	}

	Point2f div(Point2f dividend, size_t divider)
	{
		Point2f temp;
		temp.x = float(dividend.x) / divider;
		temp.y = float(dividend.y) / divider;
		return temp;
	}
}

int main(int argc, char *argv[])
{
	VideoCapture capture(0);
	capture.set(15, -5.5);


	string Name = UserName();
	if (POST::POST(Name, "Start", "42", "240b", "2019-09-09 13:37:42"))
		cout << "Cool" << endl;

	if (!capture.isOpened())
	{
		cerr << "Camera is not conected!" << endl;
		return 0;
	}
	// Создание цветов для "углов"
	vector<Scalar> colors;
	RNG rng(42);
	for (unsigned i = 0; i < MAX_CORNERS; i++)
	{
		int r = rng.uniform(0, 256);
		int g = rng.uniform(0, 256);
		int b = rng.uniform(0, 256);
		colors.push_back(Scalar(r, g, b));
	}
	Mat old_frame, old_gray;
	vector<Point2f> vOldPoints, vNewPoints;
	// Получаем первый кадр и ищем на нём углы
	capture >> old_frame;
	cvtColor(old_frame, old_gray, COLOR_BGR2GRAY);
	goodFeaturesToTrack(old_gray, vOldPoints, MAX_CORNERS, 0.01, 20, Mat(), 20, false, 0.04);
	// Создание маски разницы
	Mat mask = Mat::zeros(old_frame.size(), old_frame.type());
	while (true)
	{
		Mat frame, frame_gray;
		capture >> frame;
		if (frame.empty())
			break;
		cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
		// Подсчёт оптического свечения
		vector<unsigned char> status;
		vector<float> err;
		TermCriteria criteria = TermCriteria((TermCriteria::COUNT) | (TermCriteria::EPS), 10, 0.05);
		calcOpticalFlowPyrLK(old_gray, frame_gray, vOldPoints, vNewPoints, status, err, Size(50, 50), 1, criteria);
		vector<Point2f> NewValidPoints;
		Point2f DeltaPerFrame = { 0.0, 0.0 }; // Дельта перемещения по точкам что в движении
		Point2f AvgDelta2sec = { 0.0, 0.0 }; // Среднее значение дельты за 2 секунды
		for (unsigned i = 0; i < vOldPoints.size(); i++)
		{
			// Выбор хороших точек
			if (status[i] == 1)
			{
				NewValidPoints.push_back(vNewPoints[i]);
				// Отрисовка треков
				line(mask, vNewPoints[i], vOldPoints[i], colors[i], 2);
				circle(frame, vNewPoints[i], 5, colors[i], -1);
				DeltaPerFrame += cvvv::abs((vNewPoints[i] - vOldPoints[i]));
			}
		}
		DeltaPerFrame = cvvv::div(DeltaPerFrame, vOldPoints.size());
		Mat img;
		putText(frame, format("(%.2f,%.2f)", DeltaPerFrame.x, DeltaPerFrame.y), Point(10, 25), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0, 255, 128), 2.0);
		add(frame, mask, img);
		imshow("RealTime Tracking", img);
		int keyboard = waitKey(FPS(10));
		// Обновление предыдущего состояния
		old_gray = frame_gray.clone();
		vOldPoints = NewValidPoints;
	}
}