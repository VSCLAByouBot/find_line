#include<ros/ros.h>
#include<image_transport/image_transport.h>
#include<cv_bridge/cv_bridge.h>
#include<sensor_msgs/image_encodings.h>
#include "std_msgs/Char.h"
#include<opencv-3.3.1-dev/opencv2/opencv.hpp>
#include<opencv-3.3.1-dev/opencv/highgui.h>
#include<opencv-3.3.1-dev/opencv2/core.hpp>
#include<queue>
#include<stdio.h>
#include<iostream>
#include<algorithm>
#define get_data_rate 15 //5
#define pub_data_rate 3 //1
//#define CalSlopeAndPixelOnly
#define theshold 1000 //the theshold of the camera pixel
using namespace cv;
using namespace std;
//FILE *fout;
class FindLine_V
{
	int count;
	ros:: NodeHandle nh_;
	image_transport::ImageTransport it_;
	image_transport::Subscriber image_sub_;
	ros::Publisher chatter_pub;
	cv_bridge::CvImagePtr cv_ptr;
	public:
	// Initial Class
	FindLine_V()
		:it_(nh_)
	{
		
		FILE *fout;
		fout = fopen("/home/huang/slope.txt","a");
		fprintf(fout,"%s","----------------");		
		image_sub_ = it_.subscribe("/camera/image_rect_color", 1, &FindLine_V::FindLineCB, this);
		chatter_pub = nh_.advertise<std_msgs::Char>("chatter",10);//
		namedWindow("Find line");
		dilate_ = getStructuringElement(MORPH_RECT, Size(9, 9), Point(4, 4));
		count_W_up = theshold;count_W_down = theshold;
		FindLine_V::StartLoop();
		
		
	}
	~FindLine_V()
	{
		destroyWindow("Find line");
		//fclose(fout);
	}
	
	void StartLoop()
	{
		ros::Rate rate(get_data_rate); //5Hz
		//cout<<"start loop  ";
		//static int index = 0;//to avoid to go to case3 right after finishing case2
		for(int i=0;i<get_data_rate;i++)		
			rate.sleep();
		while(nh_.ok())
		{
			ros::spinOnce();
			#ifdef CalSlopeAndPixelOnly		
			FindLine_V::CalSlopeAndPixel();
			#else
			
			if((count_W_down < theshold)||(count_W_up < theshold))
			{
				if(count_W_up < theshold)
				{
					ROS_INFO("cannot detect down range pixel");
				}
				else
				{
					ROS_INFO("cannot detect up range pixel");
				}
				//publish stop char
				std_msgs::Char msg_to_car;
				msg_to_car.data = 'k';
				ROS_INFO("%c", msg_to_car.data);
				chatter_pub.publish(msg_to_car);
				break;
			}
			else
			{
				rate.sleep();
				static int index = 0;//to avoid to go to case3 right after finishing case2
				if(!pub_que.empty()&&!FindLine_V::PublishEqualChar())//publish immediately
					{cout<<"case 1";FindLine_V::pub_to_car(1);}
				else if(pub_que.size()>=(get_data_rate/pub_data_rate))//publish every fixed time
					{cout<<"case 2";FindLine_V::pub_to_car(0);index = 1;}
				else
					{
						cout<<"case 3";FindLine_V::CalSlopeAndPixel();
						if(pub_que.size()==1&&index==0)//publish at the first time
						{	
							FindLine_V::pub_to_car(2);
						}
						index = 0;
					}
				
				
			}
			#endif
		}//end while
	}
	void pub_to_car(int now)///
	{
		
		std_msgs::Char msg_to_car;
		static int image_index = 0;
		char str[2]={'f','i'};
		image_index++;
		sprintf(str,"%d.jpg",image_index);		
		if(now==1)
		{
			
			msg_to_car.data = pub_que.back();
			ROS_INFO("%c", msg_to_car.data);
			chatter_pub.publish(msg_to_car);
			imwrite(str,drawing);
			while (!pub_que.empty()) //clear the queue 
			{
        			pub_que.pop();
    			}
		}
		else if(now==2)	
		{
			msg_to_car.data = pub_que.front();
			ROS_INFO("%c", msg_to_car.data);
			chatter_pub.publish(msg_to_car);
			imwrite(str,drawing);
		}	
		else
		{		
			msg_to_car.data = pub_que.front();
			while (!pub_que.empty()) //clear the queue 
			{
        			pub_que.pop();
    			}
			chatter_pub.publish(msg_to_car);		
			ROS_INFO("%c", msg_to_car.data);
			imwrite(str,drawing);
		}	
	}
	bool PublishEqualChar()
	{
		if(pub_que.front()==pub_que.back())
			return true;
		return false;
	}
	// Callback Function
	void FindLineCB(const sensor_msgs::ImageConstPtr& msg)
	{
		cv_bridge::CvImagePtr cv_ptr;
		try
		{
			cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
			image = cv_ptr->image;
		}
		catch(cv_bridge::Exception& err)
		{
			ROS_ERROR("cv_bridge exception: %s", err.what());
			return;
		}
		resize(image, image, Size(image.cols/2, image.rows/2));
	}// end Callback Function
	static bool cmp1(const vector<Point> &a, const vector<Point> &b)
	{
		return a[0].y > b[0].y;
	}
	void CalSlopeAndPixel()
	{
		//resize(image, image, Size(image.cols/2, image.rows/2));
		roi = image(Rect(0, image.rows/2, image.cols, image.rows/2));
		//roi = image(Rect(0, 0, image.cols, image.rows));
		
		/*pt2_st = roi.rows * 3 / 4;  
		sec_y = roi.rows * 1 / 4;  

		roi_Rect[0] = Rect(0, 0, roi.cols, sec_y);
		roi_Rect[1] = Rect(0, pt2_st, roi.cols, sec_y);
		check_up = roi(roi_Rect[0]);*/
		check_up = roi;
		//check_up = image(Rect(0, image.rows/2, image.cols, image.rows/8));//roi up
		cvtColor(check_up, check_up, COLOR_BGR2GRAY);
		GaussianBlur(check_up, check_up, Size(19, 19), 0);
		threshold(check_up, check_up, 15, 255, THRESH_BINARY_INV);
		
		/*check_down = roi(roi_Rect[1]);
		cvtColor(check_down, check_down, COLOR_BGR2GRAY);
		GaussianBlur(check_down, check_down, Size(19, 19), 0);
		threshold(check_down, check_down, 15, 255, THRESH_BINARY_INV);*/
		count_W_up = countNonZero(check_up);
		//count_W_down = countNonZero(check_down);
		//cout<<"count_W_up"<<count_W_up<<endl;
		//cout<<"count_W_down"<<count_W_down<<endl;
		if((count_W_up < theshold))
		{
			if(count_W_up < theshold)
				ROS_INFO("Upper pixel can't detect.");

			imshow("Find line", roi);
			waitKey(100);
		}
		else
		{	
			
			Mat src = roi;
		
			cvtColor(src, src, COLOR_BGR2GRAY);
			GaussianBlur(src, src, Size(19, 19), 0);
			threshold(src, src, 15, 255, THRESH_BINARY_INV);
			dilate(src, src, dilate_);
			
			vector<vector<Point> > contours;
			vector<Vec4i> hierarchy;
			findContours(src, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
			//sort contours of y from the biggest to the smallest
			std::sort(contours.begin(), contours.end(), FindLine_V::cmp1);
			for(int i = 0;i < contours.size();i++)
			{
				for(int j = 0;j < contours[i].size();j++)
				{	
					cout<<contours[i][j]<<"  ";
				}
				cout<<endl;
			}
			//the biggest y goes up some scale, 
			//and use find contours to get the contour of down roi
			//the smallest y goes down some scale, 
			//and use find contours to get the contour of upper roi
			//stop criteria: When upper roi and down roi is overlap
			
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

			centers[0] = Point(cx, cy);
			
		drawing = roi.clone();

		//rectangle(drawing, roi_Rect[0], Scalar(255, 0, 0), 2.5);
		//rectangle(drawing, roi_Rect[1], Scalar(255, 0, 0), 2.5);
		
		for(int c = 0; c < cnt[1].size(); c++){
			//cout << cnt[1].size() << endl;
			cnt[1][c].y += pt2_st;
		}
		drawContours(drawing, cnt, 0, Scalar(0, 0, 255), 2);
		drawContours(drawing, cnt, 1, Scalar(0, 0, 255), 2);
		
		cnt.clear();
	
		line(drawing, centers[0], Point(centers[1].x, centers[1].y + pt2_st), Scalar(0, 255, 0), 2);

		m_line = (double)(centers[0].x - centers[1].x) / (double)(centers[1]. y + pt2_st - centers[0].y);
		ROS_INFO("Slope: %lf ; White Pixels: %d", m_line, count_W_up);
		FindLine_V::slope_output();	
		imshow("Find line", drawing);
		waitKey(100);
		}//end 180 line else
		//todo: Calculate the slope and pixel and Put the char you want to publish to car here 
		FindLine_V::TransSlopeToChar();		
		/* fixed value
		static int count = 0;		
		if(count<14)
		{			
			pub_que.push('i');
			//cout<<"send i";
			count++;
		}			
		else if(count==14)
		{
			//std_msgs::Char msg_to_car2;
			//msg_to_car2.data='k';
			pub_que.push('k');
			//cout<<"send k";
			//chatter_pub.publish(msg_to_car2);		
			//ROS_INFO("%c", msg_to_car2.data);
			//count = 0;	
			count++;
		}
		*/		
	}// end CalSlopeAndPixel
	void TransSlopeToChar()
	{
		pub_que.push('i');
		if(-0.3<=m_line&&m_line<0.3)//forwards
			pub_que.push('i');
		else if(0.3<=m_line&&m_line<1.1)//right front 30 degree
			pub_que.push('t');
		else if(1.1<=m_line&&m_line<5)//right front 60 degree
			pub_que.push('y');
		else if(-1.1<=m_line&&m_line<-0.3)//left front 30 degree
			pub_que.push('r');
		else if(-5<=m_line&&m_line<-1.1)//left front 60 degree
			pub_que.push('e');
		else
			pub_que.push('k');
		
		
	}
	void slope_output()
	{	
		FILE *fout;
		fout = fopen("/home/huang/slope.txt","a");
		if(fout==NULL) 
		{
   			printf("Fail To Open File slope.txt!!");
   			fclose(fout);
   			return;
 		}		
		fprintf(fout,"%lf\n",m_line);
	}
	private:
		Mat roi, check_up,check_down, dilate_, drawing,image;
		Rect roi_Rect[2];
		vector<vector<Point> > cnt;
		int pt2_st, sec_y; 
		Point2i centers[2]; 
		int count_W_up,count_W_down;double m_line;
		queue<char> pub_que;
		//FILE *fout;
	
};

int main(int argc, char** argv){
	ros::init(argc, argv, "find_line");
	FindLine_V fl;
	
	return 0;
}
