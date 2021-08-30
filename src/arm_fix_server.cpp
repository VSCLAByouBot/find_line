#include <iostream>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
//#include <string>
#include "ros_findline.h"
#include "find_line/ArmFixPoint.h"
using namespace std;

class YBPosCtrl
{
public:

	YBPosCtrl();
	~YBPosCtrl();
	ros::NodeHandle n;
	ros::Publisher armPosPub;
	ros::ServiceServer fixPosSS;
	//ros::ServiceServer armPosSS;


private:

	const double DEG2RAD = 0.01745329;
	map<string, int> mapPosition;
	const string arm_joint_name[5] = {"arm_joint_1", "arm_joint_2", "arm_joint_3", "arm_joint_4", "arm_joint_5"};

	void buildPosMap();
	bool fixPosServer(find_line::ArmFixPoint::Request  &req, find_line::ArmFixPoint::Response &res);
	void toFixPos(int pos_num);
	void goPosition(const double val[5],vector< vector<int> > seq);
};


YBPosCtrl::YBPosCtrl(){
	armPosPub = n.advertise<brics_actuator::JointPositions>("arm_1/arm_controller/position_command", 1);
	fixPosSS = n.advertiseService("arm_fix_position_srv", &YBPosCtrl::fixPosServer, this);
	//armPosSS = n.advertiseService("arm_position_srv", &YBPosCtrl::PosCtrlServer, this);

	this -> buildPosMap();
	ros::spin();
}

YBPosCtrl::~YBPosCtrl(){;}

void YBPosCtrl::buildPosMap(){
	mapPosition["home"] = 1;
	mapPosition["follow_line"] = 2;
}

bool YBPosCtrl::fixPosServer(find_line::ArmFixPoint::Request  &req,
				  			 find_line::ArmFixPoint::Response &res)
{
    cout << "Receive request : " << req.pos << endl;

	if( mapPosition.find(req.pos) == mapPosition.end() ){
		cout << "Not found in the map." << endl;
		res.status = 0;		// wrong command.
	}
	else{
		this -> toFixPos(mapPosition[req.pos]);

		cout << "Done." << endl;
		res.status = 1;		// Done.
	}

	return true;
}

void YBPosCtrl::toFixPos(int pos_num)
{
	cout << "Got the target position : " << pos_num << endl;

	if(pos_num == mapPosition["home"]){
		vector< vector<int> > j_seq = { {2, 3, 4}, {5}, {1} };
		double j_pos[5] = { 0.01006922, 
							0.0100693,
							-0.015708,
							0.0221239,
							0.110620  };
		this -> goPosition(j_pos, j_seq);
	}
	else if(pos_num == mapPosition["follow_line"]){
		vector< vector<int> > j_seq = { {5}, {1}, {2, 3, 4} };
		double j_pos[5] = { 165 * DEG2RAD, 
							95  * DEG2RAD,
							-40 * DEG2RAD,
							145 * DEG2RAD,
							165 * DEG2RAD };
		this -> goPosition(j_pos, j_seq);
	}
	else
		cout << "Non-existing position." << endl;
}

void YBPosCtrl::goPosition(const double j_pos[5],vector< vector<int> > j_seq){
	ros::Rate rate(10);
	for(int j = 0; j < j_seq.size(); j++){
		brics_actuator::JointPositions command;
		vector<brics_actuator::JointValue> setValue;
		setValue.resize(j_seq[j].size());

		for(int i = 0; i < j_seq[j].size(); i++){
			setValue[i].joint_uri = arm_joint_name[j_seq[j][i]-1];
			setValue[i].unit = boost::units::to_string(boost::units::si::radians);
			setValue[i].value = j_pos[j_seq[j][i]-1];
			cout << "setting: " << setValue[i].joint_uri << ", " << setValue[i].value << endl;
		}
		command.positions = setValue;

		for(int n = 0; n < 10; n++)
			rate.sleep();

		cout << "sending command ..." << endl;
		armPosPub.publish(command);

		for(int n = 0; n < 15; n++)
			rate.sleep();
	}
}

// -------------------------------------------------

int main(int argc, char **argv)
{
	ros::init(argc, argv, "youbot_arm_fix_pos");
	
	YBPosCtrl YB;

	return 0;	
}