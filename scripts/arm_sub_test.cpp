/* STEP 1 : Reading Joint State */
/* STEP 2 : Sending Position Command (Single)*/
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

const int NumOfJoint = 5;
#define DEG2RAD 0.01745329
#define RAD2DEG 57.29578049

class ArmSubTest
{
public:
	ArmSubTest();
	~ArmSubTest();
private:
	ros::NodeHandle n;
	ros::Publisher armPositionsPublisher;
	ros::Subscriber jointStateSubscriber;

	const string arm_joint_name[NumOfJoint] = {"arm_joint_1", "arm_joint_2", "arm_joint_3", "arm_joint_4", "arm_joint_5"};
	double arm_joint_state[NumOfJoint] = { 0.0, 0.0, 0.0, 0.0, 0.0 }; //radian
	//double arm_joint_max[NumOfJoint] = {  169,  90,  146,  102.5,  167.5 };	//degree
	//double arm_joint_min[NumOfJoint] = { -169, -65, -151, -102.5, -167.5 };	//degree
	double arm_joint_max[NumOfJoint] = { 334.615, 149.999, -0.90001, 196.478, 323.239 };	//degree
	double arm_joint_min[NumOfJoint] = { 0.577  , 0.577  , -288    , 1.268  , 6.33801 };	//degree

	int mode = 0;

	void startLoop();
	void showJointState();
	void jointCallback(const sensor_msgs::JointState &jointStates);
	void setSingleJoint(const int joint_num, const double joint_value);
	void setJoints(const double joint_value[NumOfJoint]);
};

//================================/
ArmSubTest::ArmSubTest(){
	armPositionsPublisher = n.advertise<brics_actuator::JointPositions>("arm_1/arm_controller/position_command", 1, this);
	jointStateSubscriber = n.subscribe("/joint_states", 1, &ArmSubTest::jointCallback, this);
	this -> startLoop();
}
ArmSubTest::~ArmSubTest(){}

//=================================/
void ArmSubTest::jointCallback(const sensor_msgs::JointState &jointStates){
	cout << "===== Subscriber Callback =====" << endl;
	for (int i = 0; i < jointStates.position.size(); i++){
		for (int j = 0; j < NumOfJoint; j++){
			if (jointStates.name[i] == arm_joint_name[j]){
				arm_joint_state[j] = jointStates.position[i];
				//cout << "Joint #" << j << " : " << jointStates.name[j] << jointStates.position[i] << " => " << arm_joint_state[j] << endl; 
				break;
			}
		}
	}
	        

}
void ArmSubTest::showJointState(){

	cout << "===== Joint State =====" << endl;
	cout <<   "Joint #1: " << arm_joint_state[0]*RAD2DEG << 
		"\nJoint #2: " << arm_joint_state[1]*RAD2DEG <<
		"\nJoint #3: " << arm_joint_state[2]*RAD2DEG <<
		"\nJoint #4: " << arm_joint_state[3]*RAD2DEG <<
		"\nJoint #5: " << arm_joint_state[4]*RAD2DEG << endl;

}
void ArmSubTest::setSingleJoint(const int joint_num, const double joint_value){
	brics_actuator::JointPositions command;
	vector<brics_actuator::JointValue> armJointPositions;
	
	/*	
	armJointPositions.resize(NumOfJoint);
	for(int i = 0; i < NumOfJoint; i++){
		armJointPositions[i].joint_uri = arm_joint_name[i];
		armJointPositions[i].unit = boost::units::to_string(boost::units::si::radians);
		if(i == joint_num-1)
			armJointPositions[i].value = joint_value;
		else
			armJointPositions[i].value = arm_joint_state[i];
	}
	*/
	
	armJointPositions.resize(1);	
	armJointPositions.joint_uri = arm_joint_name[joint_num - 1];
	armJointPositions.unit = boost::units::to_string(boost::units::si::radians);
	armJointPositions.value = joint_value;

	cout << "Single Joint Setting: " << armJointPositions.joint_uri << ", " << armJointPositions.value << endl;
	cout << "sending command ..." << endl;
	
	command.positions = armJointPositions;
	//armPositionsPublisher.publish(command);
}

void ArmSubTest::setJoints(const double joint_value[NumOfJoint]){
	/*brics_actuator::JointPositions command;
	vector<brics_actuator::JointValue> armJointPositions;
	armJointPositions.resize(NumOfJoint);
	
	for(int i = 0; i < NumOfJoint; i++){
		armJointPositions[i].joint_uri = arm_joint_name[i];
		armJointPositions[i].unit = boost::units::to_string(boost::units::si::radians);
		armJointPositions[i].value = joint_value[i];
	}

	cout << "sending command ..." << endl;
	
	command.positions = armJointPositions;
	armPositionsPublisher.publish(command);*/
}

//======================================/

void ArmSubTest::startLoop(){

	//ros::Rate rate(5); //Hz
	
	while(n.ok()){
		ros::spinOnce();

		cout << "\n===== Choose mode =====" <<
			"\n>  0: Exit" <<
			"\n>  1: Show Joint State" <<
			"\n>  2: Set Joint Position (Single)" <<
			"\n>  3: Set Joint Position (All)" << endl;
		
		cin >> mode;
		while(mode < 0 || mode > 3){
			cout << "[WRONG] The mode number is out of range." <<
				"\n===== Choose mode =====" <<
				"\n>  0: Exit" <<
				"\n>  1: Show Joint State" <<
				"\n>  2: Set Joint Position (Single)" <<
				"\n>  3: Set Joint Position (All)" << endl;
			cin >> mode;
		}
		
		if(mode == 0){
			cout << "exit..." << endl;
			break;		
		}
		/* Mode 1 Show Joint State(s) */
		else if(mode == 1){
			ros::spinOnce();
			this -> showJointState();
		}
		/* Mode 2 Set Single Joint Value */
		else if(mode == 2){
			ros::spinOnce();
			int joint_num = 0;
			double joint_val;
			cout << "[Set Joint Position (Single)] \nChoose Joint: (1~5)" << endl;
			cin >> joint_num;
			if(joint_num < 1 || joint_num > 5){
				cout << "Joint number out of range (#1~#5)" << endl;
				continue;
			}
			else{
				cout << "Desired joint value (deg.):" << endl;
				cin >> joint_val;
				while(joint_val >= arm_joint_max[joint_num] || joint_val <= arm_joint_min[joint_num]){
					cout << "Joint#" << joint_num << " : " << arm_joint_max[joint_num] << " ~ " << arm_joint_min[joint_num] << " (deg.)" << "\nDesired joint value:" << endl;
					cin >> joint_val;
				}
			
				this -> setSingleJoint(joint_num, joint_val * DEG2RAD);  // radian
			}
		}
		/* Mode 3 Set Five Joint Values */
		else if(mode == 3){
			ros::spinOnce();
			double joint_val[NumOfJoint];
			cout << "[Set Joint Position (Five All)]" << endl;
			for(int i = 0; i < NumOfJoint; i++){
				cout << "Desired value of Joint#" << i+1 <<"(deg.) : "<< endl;
				cin >> joint_val[i];
				while(joint_val[i] >= arm_joint_max[i] || joint_val[i] <= arm_joint_min[i]){
					cout << "Range: " << arm_joint_max[i] << " ~ " << arm_joint_min[i] << " (deg.)" << "\nDesired joint value:" << endl;
					cin >> joint_val[i];
				}
				joint_val[i] = joint_val[i] * DEG2RAD;	// radian
			}
			this -> setJoints(joint_val);
		}
		/* Else : do nth. */
		else
			continue;
		
		
		//rate.sleep();

	}// End of While

}// End of StartLoop()

int main(int argc, char **argv)
{
	ros::init(argc, argv, "yB_pos_control_test");
	ArmSubTest Arm_Position_SubTest;
	//ros::spin();
	
	return 0;	
}
