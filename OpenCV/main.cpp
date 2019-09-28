#include <iostream>
#include <condition_variable>
#include <mutex>
#include <ctime>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>

#include "Request.h"

#define POST_TIME_IN_MS 2000
#define MAX_CORNERS 505
#define STATUS_OK 1
#define FPS(x) int(1000/int(x))
#define TIME_TO_SEND_IN_THREAD 60 // FPSx2

using namespace cv;
using namespace std;

/// ���������� ��� ������������� ������, � ������� ������� �����
condition_variable PostRequestThreadStatus;
bool post_request_thread_status = 0;
mutex post_request_thread_mutex;
bool ThreadIsOpen = true;
unsigned gCountMove = 0;


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
		// �������� ��� 30 fps
		Point2f StandUp30 = { 4.5f, 6.9f };
		Point2f HeadRot30 = { 0.3f, 0.2f };
		Point2f Stable30 = { 0.1f, 0.1f };
		Point2f SlowDown30 = { 0.2f, 0.2f };
		Point2f HandShake30 = { 1.9f, 2.0f };
		// ��������� �� ����������
		temp.push_back(((delta.x / StandUp30.x) + (delta.y / StandUp30.y)) / 2);
		temp.push_back(((delta.x / HeadRot30.x) + (delta.y / HeadRot30.y)) / 2);
		temp.push_back(((delta.x / Stable30.x) + (delta.y / Stable30.y)) / 2);
		temp.push_back(((delta.x / SlowDown30.x) + (delta.y / SlowDown30.y)) / 2);
		temp.push_back(((delta.x / HandShake30.x) + (delta.y / HandShake30.y)) / 2);
		return temp;
	}
}

namespace SendThread
{
	void _PowerOff()
	{
		ThreadIsOpen = false;
	}

	void _Send(RNG& rng, string& Status, string& Location)
	{
		while (ThreadIsOpen)
		{
			string st;
			rng.uniform(0, 1) == 1 ? st += "START" : st += "STOP";
			string cm = to_string(gCountMove);
			string pl = to_string(rng.uniform(70, 85));
			POST::SEND_POST(Status, cm, pl, Location);
			Sleep(POST_TIME_IN_MS);
		}

		post_request_thread_status = 1;
		PostRequestThreadStatus.notify_one();
	}

	thread SendPostThread(RNG& rng, string& Status, string& Location)
	{
		thread th(_Send, ref(rng), ref(Status), ref(Location));
		th.detach();
		return th;
	}
}

int main(int argc, char *argv[])
{
	VideoCapture capture(0);
	capture.set(15, -5.5);
	//capture.set(15, -8.5); // ��� ���������� ���

	string Status;
	string CountMove;
	string Location;
	string Pulse;
	if (argc > 1 && argc == 4)
	{
		Status = argv[1]; // START, STOP
		CountMove = argv[2]; // �����
		Pulse = argv[3]; // �����
		Location = argv[4]; // ������
	}
	else
	{
		Status = "START";
		CountMove = "42";
		Pulse = "70";
		Location = "0812";
	}
	
	
	if (!capture.isOpened())
	{
		cerr << "Camera is not conected!" << endl;
		return 0;
	}
	// �������� ������ ��� "�����"
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
	// �������� ������ ���� � ���� �� �� ����
	capture >> old_frame;
	cvtColor(old_frame, old_gray, COLOR_BGR2GRAY);
	goodFeaturesToTrack(old_gray, vOldPoints, MAX_CORNERS, 0.01, 20, Mat(), 20, false, 0.04);
	// �������� ����� �������
	Mat mask = Mat::zeros(old_frame.size(), old_frame.type());
	Point2f MaxAvgDelta2sec = { 0.0, 0.0 }; // ������������ �������� ������ �� ����� ���������
	//ofstream of("res.txt");
	thread post_th = SendThread::SendPostThread(rng, Status, Location);
	unsigned counter = 0;
	while (true)
	{
		counter++;
		Mat frame, frame_gray;
		capture >> frame;
		if (frame.empty())
			break;
		cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
		// ������� ����������� ��������
		vector<unsigned char> status;
		vector<float> err;
		TermCriteria criteria = TermCriteria((TermCriteria::COUNT) | (TermCriteria::EPS), 10, 0.05);
		if (vOldPoints.size() <= 10 && vNewPoints.size() <= 10)
		{
			// �������� ������ ���� � ���� �� �� ����
			capture >> old_frame;
			cvtColor(old_frame, old_gray, COLOR_BGR2GRAY);
			goodFeaturesToTrack(old_gray, vOldPoints, MAX_CORNERS, 0.01, 20, Mat(), 20, false, 0.04);
			// �������� ����� �������
			mask = Mat::zeros(old_frame.size(), old_frame.type());
		}
		calcOpticalFlowPyrLK(old_gray, frame_gray, vOldPoints, vNewPoints, status, err, Size(50, 50), 1, criteria);
		vector<Point2f> NewValidPoints;
		Point2f DeltaPerFrame = { 0.0, 0.0 }; // ������ ����������� �� ������ ��� � ��������
		Point2f AvgDelta2sec = { 0.0, 0.0 }; // ������� �������� ������ �� 2 �������
		for (unsigned i = 0; i < vOldPoints.size(); i++)
		{
			// ����� ������� �����
			if (status[i] == STATUS_OK)
			{
				NewValidPoints.push_back(vNewPoints[i]);
				// ��������� ������
				line(mask, vNewPoints[i], vOldPoints[i], colors[i], 1);
				circle(frame, vNewPoints[i], 3, colors[i], -1);
				DeltaPerFrame += cvvv::abs((vNewPoints[i] - vOldPoints[i]));
			}
		}
		DeltaPerFrame = cvvv::div(DeltaPerFrame, vOldPoints.size());
		vector<float> temp;
		temp = Active::Detect(DeltaPerFrame);
		Mat img;
		// ������ ������������ ������ ���������� � ����, ��� �������� �� ������ ������������ ���������
		// ������ ����� ������������ ����������
		putText(frame, format("StandUp: %.1f; HeadRot: %.1f; Stable: %.1f; SlowDown: %.1f; HandShake: %.1f", temp[0], temp[1], temp[2], temp[3], temp[4]), Point(20, 55), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(32, 255, 32), 2.0);
		putText(frame, format("Move/2sec %d", gCountMove), Point(20, 75), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(25, 255, 10), 2.0);
		if (DeltaPerFrame.x > 0.7f || DeltaPerFrame.y > 0.7f)
		{
			MaxAvgDelta2sec = DeltaPerFrame;
			putText(frame, format("d(%.1f;%.1f)", DeltaPerFrame.x, DeltaPerFrame.y), Point(20, 25), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255, 25, 10), 2.0);
			putText(frame, format("| MAX: d(%.1f;%.1f)", MaxAvgDelta2sec.x, MaxAvgDelta2sec.y), Point(105, 25), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255, 25, 10), 2.0);
			//of << MaxAvgDelta2sec.x << "\t" << MaxAvgDelta2sec.y << endl;
			if (counter < TIME_TO_SEND_IN_THREAD)
			{
				gCountMove++;
			}
		}
		else
		{
			putText(frame, format("d(%.1f;%.1f)", DeltaPerFrame.x, DeltaPerFrame.y), Point(20, 25), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0, 255, 128), 2.0);
			putText(frame, format("| MAX: d(%.1f;%.1f)", MaxAvgDelta2sec.x, MaxAvgDelta2sec.y), Point(105, 25), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255, 25, 10), 2.0);
		}

		if (counter == TIME_TO_SEND_IN_THREAD)
		{
			counter = 0;
			gCountMove = 0;
		}
		add(frame, mask, img);
		imshow("RealTime Tracking", img);
		imwrite("frame.jpg", frame);
		int keyboard = waitKey(FPS(30));
		// ���������� ����������� ���������
		old_gray = frame_gray.clone();
		vOldPoints = NewValidPoints;
		char c = (char)waitKey(10);
		if (c == 27 || c == 'q' || c == 'Q')
			break;	
	}
	string st = "STOP";
	string cm = to_string(gCountMove);
	string pl = to_string(rng.uniform(70, 85));
	POST::SEND_POST(Status, cm, pl, Location);
	SendThread::_PowerOff();
	unique_lock<mutex> sock_lock(post_request_thread_mutex);
	PostRequestThreadStatus.wait(sock_lock, []() { return post_request_thread_status == 1; });
	cout << "POST thread closed." << endl;

	//of.close();
}