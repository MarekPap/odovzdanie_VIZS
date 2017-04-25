#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include "Object.h"

//defaltne nastavena sirka a vyska obrazu
const int FRAME_WIDTH =  640;
const int FRAME_HEIGHT =  480;
//max pocet detekovanych objektov vo frame
const int MAX_NUM_OBJECTS = 4;  
//min a max plocha objektu 
const int MIN_OBJECT_AREA = 50 * 50;
const int MAX_OBJECT_AREA = 200 * 200;

const string windowName = "Original Image";
const string windowName1 = "HSV Image";
const string windowName_red = "RED";
const string windowName_yellow = "YELLOW";
const string windowName_blue = "BLUE";
const string windowName_green = "GREEN";

Mat src, src_gray;

string intToString(int number) 
{
	std::stringstream ss;
	ss << number;
	return ss.str();
}

void drawObject(vector<Object> theObjects, Mat &frame, Mat &temp, vector< vector<Point> > contours, vector<Vec4i> hierarchy) {

	float x_suradnica;
	float radius;
	float real_distance, real_posun_X;
	Point2f center;

	for (int i = 0; i < theObjects.size(); i++) 
	{
		//vykreslenie polohy, farby, atd. do frame
		cv::drawContours(frame, contours, i, theObjects.at(i).getColor(), 3, 8, hierarchy);
		cv::circle(frame, cv::Point(theObjects.at(i).getXPos() + 320, theObjects.at(i).getYPos() + 240), 5, theObjects.at(i).getColor());
		cv::putText(frame, intToString(theObjects.at(i).getXPos()) + " , " + intToString(theObjects.at(i).getYPos()), cv::Point(theObjects.at(i).getXPos() + 320, theObjects.at(i).getYPos() + 260), 1, 1, theObjects.at(i).getColor());
		cv::putText(frame, theObjects.at(i).getType(), cv::Point(theObjects.at(i).getXPos() + 320, theObjects.at(i).getYPos() + 220), 1, 2, theObjects.at(i).getColor());

		//zapiseme x poziciu do x_suradnice
		x_suradnica = theObjects.at(i).getXPos();
				
		for (int i = 0; i < contours.size(); ++i)
		{
			char buffer[64] = { 0 };
			//zistuje polomer kruhu
			minEnclosingCircle(contours[i], center, radius);
			
			// vypocita realnu vzdialenost od kamery
			real_distance = -0.0001341*radius*radius*radius + 0.0477 *radius*radius - 5.713 *radius + 264.1;

			real_posun_X = -0.3187 + 0.004767 * real_distance + 0.009693 *x_suradnica + 0.001466 * real_distance * x_suradnica - 0.00007483 *x_suradnica;
			
			sprintf(buffer, "hlbka[cm]: %.2lf", real_distance);
			putText(frame, buffer, Point(center.x - 70, center.y - 80), FONT_HERSHEY_SIMPLEX, .5, Scalar(0, 0, 0), 2);

			sprintf(buffer, "posun[cm]: %.2lf", real_posun_X);
			putText(frame, buffer, Point(center.x - 70, center.y - 100), FONT_HERSHEY_SIMPLEX, .5, Scalar(0, 0, 0), 2);
		}
	}
}

void morphOps(Mat &thresh) 
{
	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle
	Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));

	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);

	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);
}

void trackFilteredObject(Object theObject, Mat threshold, Mat HSV, Mat &cameraFeed) 
{
	vector <Object> objects;
	Mat temp;
	threshold.copyTo(temp);

	//2 vektory potrebne pre funkciu findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;

	//najde obrysy filtrovaneho obrazu pomocou funkcie OpenCV findContours	
	findContours(temp, contours, hierarchy, CV_HOUGH_STANDARD, CV_CHAIN_APPROX_SIMPLE);

    //pre najdenie objektu sa pouziva moment
	double refArea = 0;
	bool objectFound = true;
	if (hierarchy.size() > 0) 
	{
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if (numObjects<MAX_NUM_OBJECTS) 
		{
			for (int index = 0; index >= 0; index = hierarchy[index][0]) 
			{

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				// v prípade, že plocha je menšia ako 30 px od 30px potom je to asi len šum
				// chceme len objekt s najväèšiu plochu
				if (area < MAX_OBJECT_AREA)
				{
					if (area > MIN_OBJECT_AREA)
					{

						Object object;
						object.setXPos((moment.m10 / area) - 320);
						object.setYPos((moment.m01 / area) - 240);
						object.setType(theObject.getType());
						object.setColor(theObject.getColor());
						objects.push_back(object);
						
						objectFound = true;
					}
					else {
						objectFound = false;
					}
				}
				else
				{
					objectFound = false;
				}
			}
			//ak je najdeny objekt tak vykresli
			if (objectFound == true) 
			{
				//vykresli umiestnenie objektu na obrazku
				drawObject(objects, cameraFeed, temp, contours, hierarchy);
			}
		}
	}
}

int main(int argc, char* argv[])
{
	//if we would like to calibrate our filter values, set to true.
	bool calibrationMode = true;

	Mat cameraFeed;
	Mat threshold_red;
	Mat threshold_blue;
	Mat threshold_green;
	Mat threshold_yellow;
	Mat HSV;
	Mat mask;

	VideoCapture capture;
	capture.open(0);
	//nastavi vysku a sirku  capture frame
	capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
	waitKey(1000);
	while (1) 
	{
		capture.read(cameraFeed);
		src = cameraFeed;

		if (!src.data)
		{
			return -1;
		}

		//vytvorenie objektov pre farby		
		Object blue("blue"), yellow("yellow"), red("red"), green("green");
	
		//modra farba
		cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
		inRange(HSV, blue.setHSVmin(Scalar(110, 50, 50)), blue.setHSVmax(Scalar(130, 255, 255)), threshold_blue);
		morphOps(threshold_blue);
		trackFilteredObject(blue, threshold_blue, HSV, cameraFeed);
			
		//zlta farba
		cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
		inRange(HSV, yellow.setHSVmin(Scalar(20, 124, 123)), yellow.setHSVmax(Scalar(30, 256, 256)), threshold_yellow);
		morphOps(threshold_yellow);
		trackFilteredObject(yellow, threshold_yellow, HSV, cameraFeed);
				
		//cervena farba
		cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
		inRange(HSV, red.setHSVmin(Scalar(0, 160, 186)), red.setHSVmax(Scalar(47,256,256)), threshold_red);
		morphOps(threshold_red);
		trackFilteredObject(red, threshold_red, HSV, cameraFeed);
		
		//zelena farba
		cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
		inRange(HSV, green.setHSVmin(Scalar(37, 86, 67)), green.setHSVmax(Scalar(77, 256,256)), threshold_green);
		morphOps(threshold_green);
		trackFilteredObject(green, threshold_green, HSV, cameraFeed);
	
		//  imshow(windowName_blue,threshold_blue);
		imshow(windowName_green, threshold_green);
		//imshow(windowName_red, threshold_red);
		//imshow(windowName_yellow, threshold_yellow);
		imshow(windowName, cameraFeed);
		imshow(windowName1,HSV);

		waitKey(30);
	}
	return 0;
}