/* Listner for yB base */

#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "ros/ros.h"
#include "geometry_msgs/Twist.h"
#include "nav_msgs/Odometry.h"

using namespace std;

void baseVelCB(const geometry_msgs::Twist &msg)
{	
	/*	
	cout << "------------------------------------------\n" <<	
		"Base Velocity Command Received !!!\n" << endl;
	
	cout << "[Linear]" <<
		"\n  x: " << msg.linear.x << "; " <<
		"\n  y: " << msg.linear.y << "; " <<
		"\n  z: " << msg.linear.z << endl;
	cout << "[Angular]" <<
		"\n  x: " << msg.angular.x << "; " <<
		"\n  y: " << msg.angular.y << "; " <<
		"\n  z: " << msg.angular.z << endl;

	cout << "\nDone." << endl;
	*/
	if(msg.linear.x != 0 || msg.linear.y != 0 || msg.linear.z != 0)
		cout << "Base Command !!!" << endl;
	else
		cout << "Stop !!" << endl;

}

int main(int argc, char **argv)
{
	ros::init(argc, argv, "yB_base_listener");
	ros::NodeHandle n;
	ros::Subscriber baseVelSubscriber;	

	baseVelSubscriber = n.subscribe("cmd_vel", 1, &baseVelCB);
	ros::spin();
	return 0;
}
