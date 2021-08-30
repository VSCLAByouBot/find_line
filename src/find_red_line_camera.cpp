#include "ros/ros.h"
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <find_line/SlopeAndPixel.h>
#include <stdio.h>
#include <iostream>
using namespace std;
using namespace cv;

//#define ImagePub
//#define FindLinePub
//#define ImageSave
//#define DEBUG_N

class FindLine
{
public:

	FindLine();
	~FindLine();
		
	ros:: NodeHandle nh_;
	image_transport::ImageTransport it_;
	image_transport::Subscriber image_sub_;
	image_transport::Publisher drawing_pub;
	ros::Publisher line_pub;

	bool _image_pub, _image_save, _print_info = false;

private:

	void StartLoop();
	void FindLineCB(const sensor_msgs::ImageConstPtr& msg);
	Mat  FindRedMask(Mat src);
	void CalSlopeAndPixel();
	void SlopeOutput();
	void SaveDrawing();

	Mat image, drawing;
	int count_W_down, count_W_up;
	int img_index;
	double m_line;
	bool has_m_line = true;
};

FindLine::FindLine():it_(nh_)
{
	nh_.getParam("/image_pub", _image_pub);
	nh_.getParam("/image_save", _image_save);
	nh_.getParam("/print_red_info", _print_info);

	if(_print_info){
		cout << "get param : [ " << boolalpha << _image_pub << " , " 
								<< boolalpha << _image_save << " , "
								<< boolalpha << _print_info << " ]" << endl;
	}

	image_sub_ = it_.subscribe("camera/image_raw", 1, &FindLine::FindLineCB, this);
	drawing_pub = it_.advertise("camera/find_red_line", 1);
	line_pub = nh_.advertise<find_line::SlopeAndPixel>("line_slope_and_px",1);

	img_index = 0;

	FindLine::StartLoop();
}


FindLine::~FindLine(){ 
	destroyAllWindows();
	cout << "end class FindLine" << endl;
}


void FindLine::StartLoop()
{
#ifdef DEBUG_N
	cout << "Start Looping ...\n\n" << endl;
#endif

	ros::Rate rate(5); //5Hz

	while(nh_.ok())
	{
		ros::spinOnce();
		rate.sleep();				
	}
}

void FindLine::FindLineCB(const sensor_msgs::ImageConstPtr& msg)
{

#ifdef DEBUG_N
	cout << "get picture from camera." << endl;
#endif

	cv_bridge::CvImagePtr cv_ptr;
	try
	{
		cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
		this -> image = cv_ptr->image;
	}
	catch(cv_bridge::Exception& err)
	{
		ROS_ERROR("cv_bridge exception: %s", err.what());
		return;
	}
	resize(image, image, Size(image.cols/2, image.rows/2));
	FindLine::CalSlopeAndPixel();
}

Mat FindLine::FindRedMask(Mat src){

	Mat hsv, r1, r2;;
	cvtColor(src, hsv, COLOR_BGR2HSV); 
	inRange(hsv, Scalar(0, 20, 30), Scalar(10, 255, 255), r1);
	inRange(hsv, Scalar(170, 20, 30), Scalar(180,255, 255), r2);

	return r1|r2;
}

void FindLine::CalSlopeAndPixel()
{
	Mat roi = image(Rect(0, image.rows/2, image.cols, image.rows/2));
	
	int pt2_st = roi.rows * 3/4;  
	int sec_y = roi.rows * 1/4;

	Rect roi_Rect[2];
	roi_Rect[0] = Rect(0, 0, roi.cols, sec_y);
	roi_Rect[1] = Rect(0, pt2_st, roi.cols, sec_y);

	Mat check_up = roi(roi_Rect[0]);
	Mat check_down = roi(roi_Rect[1]);	
	
	count_W_up = countNonZero(this -> FindRedMask(check_up));
	count_W_down = countNonZero(this -> FindRedMask(check_down));

#ifdef DEBUG_N
	cout << "[ White px ] : up = " << count_W_up << " ; down = " << count_W_down << endl;
#endif

	vector<vector<Point> > cnt;
	Point2i centers[2];

	has_m_line = true;

	for(int r = 0; r < 2; r++)
	{
		Mat mask = FindRedMask(roi(roi_Rect[r]));

	#ifdef DEBUG_N
		cout << "find contour & center : " << r << endl;
	#endif

		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;
		findContours(mask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

		if(contours.size() > 0){
			int largest_area = 0;
			int largest_area_index = 0;
			for (size_t i = 0; i < contours.size(); i++)
			{
				double area = contourArea(contours[i]);
				if (area > largest_area){
					largest_area = area;
					largest_area_index = i;
				}
			}
			cnt.push_back(contours[largest_area_index]);

			Moments mu = moments(contours[largest_area_index]);
			int cx = (int)(mu.m10/mu.m00);
			int cy = (int)(mu.m01/mu.m00);

			centers[r] = Point(cx, cy);
		}// end if (there is any allowable contour)
		else{
			//cout << "Not enough center points." << endl;
			has_m_line = false;
		}
	}// end for (find contour & center)

	drawing = roi.clone();

	if(has_m_line){	//with two centers
		#ifdef DEBUG_N
			cout << "draw the result." << endl;
		#endif

			for(int c = 0; c < cnt[1].size(); c++){
				cnt[1][c].y += pt2_st;
			}
			drawContours(drawing, cnt, 0, Scalar(0, 0, 255), 2);
			drawContours(drawing, cnt, 1, Scalar(0, 0, 255), 2);
			
			cnt.clear();

			line(drawing, centers[0], Point(centers[1].x, centers[1].y + pt2_st), Scalar(0, 255, 0), 2);

			m_line = (double)(centers[0].x - centers[1].x) / (double)(centers[1]. y + pt2_st - centers[0].y);	
			this -> SlopeOutput();	
	}//end if (there is exactly  moments found)

	if(_image_pub){

	#ifdef DEBUG_N
		cout << "publish the image." << endl;
	#endif
		sensor_msgs::ImagePtr msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", drawing).toImageMsg();
		drawing_pub.publish(msg);
		waitKey(100);
	}
	else{
		// cout << "show the image" << endl;
		imshow("Find line", drawing);
		waitKey(100);
	} // endif

	if(_image_save){
		this -> SaveDrawing();
	} //endif

} // end CalSlopeAndPixel

void FindLine::SlopeOutput(){

	if(_print_info){
		cout << "> [ slope = " << m_line << 
		         " ; up_px = " << count_W_up << 
				 " ; dn_px = " << count_W_down << " ]" << endl;
	}
	find_line::SlopeAndPixel line_msg;
	line_msg.up_px = count_W_up;
	line_msg.down_px = count_W_down;
	line_msg.hasSlope = has_m_line;
	if(has_m_line)
		line_msg.slope = m_line;
	else
		line_msg.slope = 0;

#ifdef DEBUG_N
	cout << "publish the info." << endl;
#endif
	
	line_pub.publish(line_msg);
}

void FindLine::SaveDrawing(){

#ifdef DEBUG_N
	cout << "save the images." << endl;
#endif

/*
	img_index ++;

	char str[8];
	sprintf(str,"%d.jpg",img_index);

	imwrite(str,drawing);
*/
}

int main(int argc, char** argv){

	ros::init(argc, argv, "find_red_line");
	FindLine fl;
	
	return 0;
}
