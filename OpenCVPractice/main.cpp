#include <iostream>
#include <math.h>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

Mat img;
// 0 for original image , 1 for greyScaled , 2 for halftoned , 3 for crossHatching
Mat adjustedImgs[4];
Mat currentImg;

int currentSelectedIndex = 0;

void onMouse(int Event, int x, int y, int flags, void* param);

float distance(Point a, Point b)
{
	return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));
}

Mat greyScale(Mat originalImage)
{
	Mat greyScaleImg = originalImage.clone();

	const int width = originalImage.cols;
	const int height = originalImage.rows;

	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			Vec3b color = greyScaleImg.at<Vec3b>(Point(x, y));
			//r * 0.299 + g * 0.587 + b * 0.114
			float intensity = (color[2] * 0.299f + color[1] * 0.587f + color[0] * 0.114f);

			Vec3b greyColor = Vec3b(intensity*1.0f, intensity*1.0f, intensity*1.0f);
			greyScaleImg.at<Vec3b>(Point(x, y)) = greyColor;
		}
	}

	return greyScaleImg;
}

Mat halftone(Mat originalImage)
{
	Mat halftonedImg = originalImage.clone();

	const int width = originalImage.cols;
	const int height = originalImage.rows;

	const int xDotNumber = 150;
	const int yDotNumber = 150;

	const int xQuadSize = (width / xDotNumber);
	const int yQuadSize = (height / yDotNumber);

	const float minDotRadius = 0.0f;
	const float maxDotRadius = min(xQuadSize, yQuadSize);

	for (int x = xQuadSize; x < width; x += 2 * xQuadSize)
	{
		for (int y = yQuadSize; y < height; y += 2 * yQuadSize)
		{
			float rSum = 0; // 0 ~ 255 * sampleCount
			float gSum = 0;
			float bSum = 0;
			int sampleCount = 0;

			// iterate all pixels in a quad as sample area for a dot
			for (int xInQuad = x - xQuadSize; xInQuad < x + xQuadSize; xInQuad++)
			{
				if (xInQuad < 0 || xInQuad >= width) continue;
				for (int yInQuad = y - yQuadSize; yInQuad < y + yQuadSize; yInQuad++)
				{
					if (yInQuad < 0 || yInQuad >= height) continue;

					Vec3b color = originalImage.at<Vec3b>(Point(xInQuad, yInQuad));

					rSum += color[2] / 255.0f;
					gSum += color[1] / 255.0f;
					bSum += color[0] / 255.0f;

					sampleCount++;
				}
			}
			// Calculate dot data and draw on texture
			float averageIntensity = 255.0f * (rSum * 0.299f + gSum * 0.587f + bSum * 0.114f) / (sampleCount);

			Vec3b dotColor = Vec3b(averageIntensity, averageIntensity, averageIntensity);
			float dotRadius = minDotRadius + averageIntensity * (maxDotRadius - minDotRadius) / 255.0f;

			for (int xInQuad = x - xQuadSize; xInQuad < x + xQuadSize; xInQuad++)
			{
				if (xInQuad < 0 || xInQuad >= width) continue;
				for (int yInQuad = y - yQuadSize; yInQuad < y + yQuadSize; yInQuad++)
				{
					if (yInQuad < 0 || yInQuad >= height) continue;

					Point currentPixelPosition(xInQuad, yInQuad);

					if (distance(currentPixelPosition, Point(x, y)) <= dotRadius)
					{
						halftonedImg.at<Vec3b>(Point(xInQuad, yInQuad)) = dotColor;
					}
					else
					{
						halftonedImg.at<Vec3b>(Point(xInQuad, yInQuad)) = Vec3b(0, 0, 0);
					}
				}
			}
		}
	}
	return halftonedImg;
}

Mat crossHatching(Mat originalImage)
{
	Mat crossHatchingImg = originalImage.clone();

	const int width = originalImage.cols;
	const int height = originalImage.rows;

	int equal = 0;
	int notEqual = 0;

	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			Vec3b color = crossHatchingImg.at<Vec3b>(Point(x, y));
			//r * 0.299 + g * 0.587 + b * 0.114 , from 0 to 255
			float intensity = color[1] / 255.0f;
			//(color[2] * 0.299f + color[1] * 0.587f + color[0] * 0.114f) / 255.0f;

			Vec3b lineColor = Vec3b(intensity * 255.0f, intensity * 255.0f, intensity * 255.0f);
			//Vec3b lineColor = Vec3b(255, 255, 255);
			Vec3b pixelColor = Vec3b(0, 0, 0);

			if (intensity < 0.8f)
			{
				float modResult = fmod((2.0f*x - y), 15.0f);
				if (modResult <= 2.0f && modResult >= 0)
					pixelColor = lineColor;
			}
			if (intensity < 0.6f)
			{
				float modResult = fmod((2.0f*x + y), 15.0f);
				if (modResult <= 2.0f && modResult >= 0)
					pixelColor = lineColor;
			}
			if (intensity < 0.4f)
			{
				float modResult = fmod((2.0f*x - y + 15.0f), 15.0f);
				if (modResult <= 2.0f && modResult >= 0)
					pixelColor = lineColor;
			}
			if (intensity < 0.2f)
			{
				float modResult = fmod((2.0f*x + y + 15.0f), 15.0f);
				if (modResult <= 2.0f && modResult >= 0)
					pixelColor = lineColor;
			}

			/*if (pixelColor != lineColor)
				notEqual++;
			else
				equal++;*/

			crossHatchingImg.at<Vec3b>(Point(x, y)) = pixelColor;
		}
	}

	//cout << "equal : " << equal << " , notEqual : " << notEqual << endl;
	return crossHatchingImg;
}

int main()
{
	string inputImageFile = "lena.png";

	img = imread(inputImageFile);
	//namedWindow("image", WINDOW_NORMAL);
	//imshow("image", img);

	adjustedImgs[0] = img.clone();
	adjustedImgs[1] = greyScale(img);
	adjustedImgs[2] = halftone(img);
	adjustedImgs[3] = crossHatching(img);

	currentImg = img.clone();

	namedWindow("currentImg", WINDOW_NORMAL);
	resizeWindow("currentImg", 512, 512);
	imshow("currentImg", currentImg);

	//cv::imwrite(inputImageFile + "Result.jpg", currentImg);

	setMouseCallback("currentImg", onMouse, &currentImg);

	cout << "No error after initializing." << endl;
	cout << "Usage : press 0~3 to select different processed images , press left mouse button and drag to draw." << endl;
	cout << "0 for origin image , 1 for greyScaled , 2 for halftoned , 3 for crossHatched" << endl;
	cout << "Press esc to exit." << endl;

	while (true)
	{
		int keyCode = waitKey(0);

		if (keyCode == 27)
		{
			return 0;
		}

		if (keyCode >= '0' && keyCode < '4')
		{
			currentSelectedIndex = keyCode - '0';
			cout << "currentSelectedIndex = " << currentSelectedIndex << endl;
		}
	}
}

void onMouse(int Event, int x, int y, int flags, void* param)
{
	//cout << "Event = " << Event << endl; cout << "flags = " << flags << endl;

	if (flags == CV_EVENT_FLAG_LBUTTON)
	{
		for (int xInRadius = x - 15; xInRadius < x + 15; xInRadius++)
		{
			if (xInRadius < 0 || xInRadius > img.rows) continue;
			for (int yInRadius = y - 15; yInRadius < y + 15; yInRadius++)
			{
				if (yInRadius < 0 || yInRadius > img.cols) continue;

				currentImg.at<Vec3b>(yInRadius, xInRadius) = adjustedImgs[currentSelectedIndex].at<Vec3b>(yInRadius, xInRadius);
			}
		}
	}
	
	imshow("currentImg", currentImg);
}