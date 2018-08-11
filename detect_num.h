#ifndef DETECT_NUM_H
#define DETECT_NUM_H

#include <iostream>  
#include <string>  
#include <list>  
#include <vector>  
#include <map>  
#include <stack>  
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc/imgproc.hpp>  


using namespace std;
using namespace cv;

void FindMinMax(int &x_min, int &x_max, int &y_min, int &y_max, Mat image)
{
	x_max = 0;
	y_max = 0;
	x_min = image.rows;
	y_min = image.cols;

}

void ImageThreshold(cv::Mat imgOriginal, cv::Mat &imgThresholded)
{

        int iLowH = 26;
        int iHighH = 34;

        int iLowS = 43;
        int iHighS = 255;

        int iLowV = 46;
	int iHighV = 255;


	bool found = false;

        cv::Mat imgHSV;
	std::vector<Mat> hsvSplit;
        cv::cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);
        cv::split(imgHSV, hsvSplit);
        cv::equalizeHist(hsvSplit[2], hsvSplit[2]);
	merge(hsvSplit, imgHSV);

        cv::inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded);

        cv::Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
        cv::morphologyEx(imgThresholded, imgThresholded, MORPH_OPEN, element);


        cv::morphologyEx(imgThresholded, imgThresholded, MORPH_CLOSE, element);
        cv::Mat img_canny;
        cv::Canny(imgThresholded, img_canny, 10, 80);
	std::vector<std::vector<Point> > contours, good_contours;
        cv::findContours(img_canny, contours, CV_RETR_EXTERNAL, CHAIN_APPROX_NONE);


	std::vector<Point2f> imagePoints_vertex;
	std::vector<Point> approxPoly;
	if (contours.size() == 0)
	{
		std::cout << "no contour is found" << endl;
	}
	else
	{
		for (int k = 0; k < contours.size(); k++)
		{
			if (contourArea(contours[k]) > 10000)
				approxPolyDP(contours[k], approxPoly, 1, 1);  //��ͼ����������ж�������
			if (approxPoly.size() == 4)
				;
		}
	}

	//imwrite("Thresholded.jpg", imgThresholded);
}

void  Two_PassNew(const Mat &bwImg, Mat &labImg)
{
	assert(bwImg.type() == CV_8UC1);
	labImg.create(bwImg.size(), CV_32SC1);
	labImg = Scalar(0);
	labImg.setTo(Scalar(1), bwImg);
	assert(labImg.isContinuous());
	const int Rows = bwImg.rows - 1, Cols = bwImg.cols - 1;
	int label = 1;
	std::vector<int> labelSet;
	labelSet.push_back(0);
	labelSet.push_back(1);
	//the first pass  
	int *data_prev = (int*)labImg.data;
	int *data_cur = (int*)(labImg.data + labImg.step);
	for (int i = 1; i < Rows; i++)
	{
		data_cur++;
		data_prev++;
		for (int j = 1; j<Cols; j++, data_cur++, data_prev++)
		{
			if (*data_cur != 1)
				continue;
			int left = *(data_cur - 1);
			int up = *data_prev;
			int neighborLabels[2];
			int cnt = 0;
			if (left>1)
				neighborLabels[cnt++] = left;
			if (up > 1)
				neighborLabels[cnt++] = up;
			if (!cnt)
			{
				labelSet.push_back(++label);
				labelSet[label] = label;
				*data_cur = label;
				continue;
			}
			int smallestLabel = neighborLabels[0];
			if (cnt == 2 && neighborLabels[1]<smallestLabel)
				smallestLabel = neighborLabels[1];
			*data_cur = smallestLabel;
			
			for (int k = 0; k<cnt; k++)
			{
				int tempLabel = neighborLabels[k];
				int& oldSmallestLabel = labelSet[tempLabel];
				if (oldSmallestLabel > smallestLabel)
				{
					labelSet[oldSmallestLabel] = smallestLabel;
					oldSmallestLabel = smallestLabel;
				}
				else if (oldSmallestLabel<smallestLabel)
					labelSet[smallestLabel] = oldSmallestLabel;
			}
		}
		data_cur++;
		data_prev++;
	}

	for (size_t i = 2; i < labelSet.size(); i++)
	{
		int curLabel = labelSet[i];
		int prelabel = labelSet[curLabel];
		while (prelabel != curLabel)
		{
			curLabel = prelabel;
			prelabel = labelSet[prelabel];
		}
		labelSet[i] = curLabel;
	}
	//second pass  
	data_cur = (int*)labImg.data;
	for (int i = 0; i < Rows; i++)
	{
		for (int j = 0; j < bwImg.cols - 1; j++, data_cur++)
			*data_cur = labelSet[*data_cur];
		data_cur++;
	}
}
cv::Scalar GetRandomColor()
{
	uchar r = 255 * (rand() / (1.0 + RAND_MAX));
	uchar g = 255 * (rand() / (1.0 + RAND_MAX));
	uchar b = 255 * (rand() / (1.0 + RAND_MAX));
	return cv::Scalar(b, g, r);
}

std::vector<Vec2i> detect_num(cv::Mat oriImage, std::vector<cv::Mat> &all_image, float height)
{
	std::vector<Vec2i> coordination;
	cv::Mat one_image;
	cv::Mat binImage;
	ImageThreshold(oriImage, binImage);
	cv::threshold(binImage, binImage, 50, 1, CV_THRESH_BINARY);
	cv::Mat labelImg;
	double time;
	time = getTickCount();
	
	Two_PassNew(binImage, labelImg);
	
	int *data_cur = (int*)labelImg.data;
	int t1;
	int t2 = 0;
	int x_min = labelImg.rows;
	int y_min = labelImg.cols;
	int x_max = 0;
	int y_max = 0;
	Vec2i zuobiao;
	int Min_pixel_num,Max_pixel_num;
	while (1)
	{
		int t = 0;
		for (int i = 0; i < labelImg.rows; i++)
		{
			for (int j = 0; j < labelImg.cols - 1; j++, data_cur++)
			{
				if (*data_cur == 0)
					continue;
				t1 = *data_cur;
				if ((t1 != t2) && (t2 > 0))
					continue;
				if (i < x_min)
					x_min = i;
				if (i > x_max)
					x_max = i;
				if (j < y_min)
					y_min = j;
				if (j > y_max)
					y_max = j;
				t2 = t1;
				*data_cur = 0;
				t = 1;
			}
			data_cur++;
		}
		if (!t)
			break;
		if (height > 7) {
			Min_pixel_num = 25; Max_pixel_num = 300;}
		else if (height < 7 && height > 4) {
			Min_pixel_num = 50; Max_pixel_num = 350;}
		else {
			Min_pixel_num = 70; Max_pixel_num = 400;}
		if ((x_max - x_min) < Min_pixel_num || (y_max - y_min) < Min_pixel_num || ((y_max - y_min)/ (x_max - x_min))>2 || 
			((y_max - y_min)/ (x_max - x_min))<0.5
			|| (x_max - x_min) >Max_pixel_num || (y_max - y_min) >Max_pixel_num)
		{
			x_min = labelImg.rows;
			y_min = labelImg.cols;
			x_max = 0;
			y_max = 0;
			t2 = 0;
			data_cur = (int*)labelImg.data;
			continue;
		}
		cv::Rect rect(y_min, x_min, y_max - y_min, x_max - x_min);
		cv::Mat temp(oriImage, rect);
		all_image.push_back(temp);
		//all_image.push_back(one_image);
		zuobiao[0] = (x_max + x_min) / 2;
		zuobiao[1] = (y_min + y_max) / 2;
		coordination.push_back(zuobiao);

		x_min = labelImg.rows;
		y_min = labelImg.cols;
		x_max = 0;
		y_max = 0;
		t2 = 0;
		data_cur = (int*)labelImg.data;
	}
	
	return coordination;
}

#endif
