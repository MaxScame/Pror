#include <iostream>
#include <ctime>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>

#include "Request.h"

#define MAX_CORNERS 505
#define STATUS_OK 1
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


namespace Active
{
	vector<float> Detect(Point2f& delta)
	{
		vector<float> temp;
		// Значения для 30 fps
		Point2f StandUp30 = { 4.5f, 6.9f };
		Point2f HeadRot30 = { 0.3f, 0.2f };
		Point2f Stable30 = { 0.1f, 0.1f };
		Point2f SlowDown30 = { 0.2f, 0.2f };
		Point2f HandShake30 = { 1.9f, 2.0f };
		// Сравнение на совпадение
		temp.push_back(((delta.x / StandUp30.x) + (delta.y / StandUp30.y)) / 2);
		temp.push_back(((delta.x / HeadRot30.x) + (delta.y / HeadRot30.y)) / 2);
		temp.push_back(((delta.x / Stable30.x) + (delta.y / Stable30.y)) / 2);
		temp.push_back(((delta.x / SlowDown30.x) + (delta.y / SlowDown30.y)) / 2);
		temp.push_back(((delta.x / HandShake30.x) + (delta.y / HandShake30.y)) / 2);
		return temp;
	}
}


int main(int argc, char *argv[])
{
	VideoCapture capture(0);
	capture.set(15, -5.5);

	string Status;
	string CountMove;
	string Location;
	string Pulse;
	if (argc > 1 && argc == 4)
	{
		Status = argv[1];
		CountMove = argv[2];
		Pulse = argv[3];
		Location = argv[4];
	}
	else
	{
		Status = "START";
		CountMove = "42";
		Pulse = "70";
		Location = "0812";
	}
	
	POST::SEND_POST(Status, CountMove, Pulse, Location);

	if (!capture.isOpened())
	{
		cerr << "Camera is not conected!" << endl;
		return 0;
	}
	// Создание цветов для "углов"
	vector<Scalar> colors;
	RNG rng(16020);
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
	Point2f MaxAvgDelta2sec = { 0.0, 0.0 }; // Максимальное значение дельты за время измерений
	//ofstream of("res.txt");
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
			if (status[i] == STATUS_OK)
			{
				NewValidPoints.push_back(vNewPoints[i]);
				// Отрисовка треков
				line(mask, vNewPoints[i], vOldPoints[i], colors[i], 2);
				circle(frame, vNewPoints[i], 3, colors[i], -1);
				DeltaPerFrame += cvvv::abs((vNewPoints[i] - vOldPoints[i]));
			}
		}
		DeltaPerFrame = cvvv::div(DeltaPerFrame, vOldPoints.size());
		vector<float> temp;
		temp = Active::Detect(DeltaPerFrame);
		Mat img;
		// Данные коэффициенты должны стремиться к нулю, что означало бы полное соответствие прогнозам
		// Подбор можно осуществлять нейронками
		putText(frame, format("StandUp: %.1f; HeadRot: %.1f; Stable: %.1f; SlowDown: %.1f; HandShake: %.1f", temp[0], temp[1], temp[2], temp[3], temp[4]), Point(20, 55), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(32, 255, 32), 2.0);
		if (DeltaPerFrame.x > 3.5f || DeltaPerFrame.y > 3.5f)
		{
			MaxAvgDelta2sec = DeltaPerFrame;
			putText(frame, format("d(%.1f;%.1f)", DeltaPerFrame.x, DeltaPerFrame.y), Point(20, 25), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255, 25, 10), 2.0);
			putText(frame, format("| MAX: d(%.1f;%.1f)", MaxAvgDelta2sec.x, MaxAvgDelta2sec.y), Point(105, 25), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255, 25, 10), 2.0);
			//of << MaxAvgDelta2sec.x << "\t" << MaxAvgDelta2sec.y << endl;
		}
		else
		{
			putText(frame, format("d(%.1f;%.1f)", DeltaPerFrame.x, DeltaPerFrame.y), Point(20, 25), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0, 255, 128), 2.0);
			putText(frame, format("| MAX: d(%.1f;%.1f)", MaxAvgDelta2sec.x, MaxAvgDelta2sec.y), Point(105, 25), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255, 25, 10), 2.0);
		}
		add(frame, mask, img);
		imshow("RealTime Tracking", img);
		imwrite("frame.jpg", img);
		int keyboard = waitKey(FPS(30));
		// Обновление предыдущего состояния
		old_gray = frame_gray.clone();
		vOldPoints = NewValidPoints;
	}
	//of.close();
}