#include <iostream>
#include <assert.h>

#include "ros/ros.h"
#include "trajectory_msgs/JointTrajectory.h"
#include "brics_actuator/CartesianWrench.h"
#include "brics_actuator/JointPositions.h"
#include "sensor_msgs/JointState.h"

#include <boost/units/io.hpp>

#include <boost/units/systems/angle/degrees.hpp>
#include <boost/units/conversion.hpp>

#include <iostream>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <boost/units/systems/si/length.hpp>
#include <boost/units/systems/si/plane_angle.hpp>
#include <boost/units/io.hpp>

#include <boost/units/systems/angle/degrees.hpp>
#include <boost/units/conversion.hpp>

using namespace std;

#define arrayLength(arr) (sizeof(arr) / sizeof(arr[0]))

#define DEG2RAD 0.01745329

#define home 1
#define find_line 2

const int NumOfJoint = 5;
const string arm_joint_name[NumOfJoint] = {"arm_joint_1", "arm_joint_2", "arm_joint_3", "arm_joint_4", "arm_joint_5"};

//ros::Rate rate(10);
ros::Publisher armPositionsPublisher;

// =========================================/

brics_actuator::JointPositions genCommand(const double joint_value[NumOfJoint], vector<int> joint_num){
	int cmd_num =  joint_num.size();

	brics_actuator::JointPositions command;
	vector<brics_actuator::JointValue> armJointPositions;
	armJointPositions.resize(cmd_num);

	cout << endl;
	
	for(int i = 0; i < cmd_num; i++){
		armJointPositions[i].joint_uri = arm_joint_name[joint_num[i]-1];
		armJointPositions[i].unit = boost::units::to_string(boost::units::si::radians);
		armJointPositions[i].value = joint_value[joint_num[i]-1];
		cout << "setting: " << armJointPositions[i].joint_uri << ", " << armJointPositions[i].value << endl;
	}
	command.positions = armJointPositions;
	return command;
}

brics_actuator::JointPositions getPostions(const int jointSet, vector<int> jointNum){
	double arm_joint_value[NumOfJoint];
	switch(jointSet){
		case home:
			cout << "\ngenerate position set : home ..." << endl;
			arm_joint_value[0] = 0.0100692;
			arm_joint_value[1] = 0.0100692; 
			arm_joint_value[2] = -0.015708;
			arm_joint_value[3] = 0.0221239;
			arm_joint_value[4] = 0.110619;
			break;

		case find_line:	
			cout << "\ngenerate position set : find line ..." << endl;
			arm_joint_value[0] = 165 * DEG2RAD;
			arm_joint_value[1] = 95 * DEG2RAD; 
			arm_joint_value[2] = -40 * DEG2RAD;
			arm_joint_value[3] = 145 * DEG2RAD;
			arm_joint_value[4] = 165 * DEG2RAD;
			break;

		default:
			cout << "\ngenerate position set : home ..." << endl;
			arm_joint_value[0] = 0.0100692;
			arm_joint_value[1] = 0.0100692; 
			arm_joint_value[2] = -0.015708;
			arm_joint_value[3] = 0.0221239;
			arm_joint_value[4] = 0.110619;
			break;
	};

	return genCommand(arm_joint_value, jointNum);
}

int main(int argc, char **argv)
{
	ros::init(argc, argv, "yB_pos_control");
	ros::NodeHandle n;
	ros::Publisher armPositionsPublisher;
	armPositionsPublisher = n.advertise<brics_actuator::JointPositions>("arm_1/arm_controller/position_command", 1);

	ros::Rate rate(10);
	vector< vector<int> > jNum = {{5},
				      {1}, 
	                              {2, 3, 4}, 
				      };
	for(int j = 0; j < jNum.size(); j++){
		brics_actuator::JointPositions command;
		command = getPostions(find_line, jNum[j]);

		for(int n = 0; n < 10; n++)
			rate.sleep();

		cout << "sending command ..." << endl;
		armPositionsPublisher.publish(command);

		for(int n = 0; n < 15; n++)
			rate.sleep();
	}

	return 0;	
}
