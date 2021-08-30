#include <iostream>
#include "ros/ros.h"
#include <actionlib/client/simple_action_client.h>
#include <actionlib/client/terminal_state.h>

#include "find_line/ArmFixPoint.h"
#include "find_line/FollowByRecord.h"
#include <find_line/RedLineInfoAction.h>

using namespace std;

bool _arm_ini, _follow_line, _back_line, _arm_home = false;

//#define ARM_INI   //OK
//#define FOLLOW_LINE
//#define BACK_LINE
//#define ARM_HOME  //OK

bool follow_success = true;

void doneCb(const actionlib::SimpleClientGoalState& state, 
            const find_line::RedLineInfoResultConstPtr& result)
{
    if (state == state.ABORTED) {
        cout << "server working error, don't finish my job." << endl;
        follow_success = false;
    } else if (state == state.PREEMPTED) {
        cout << "client cancel job." << endl;
        follow_success = false;
    }
}
void activeCb(){;}
void feedbackCb(const find_line::RedLineInfoFeedbackConstPtr& feedback){
    //cout << "> Slope: " << feedback->slope << " ; pace : " << feedback->pace;
}

int main(int argc, char **argv)
{
ros::init(argc, argv, "youbot_follow_redline");
ros::NodeHandle n;

n.getParam("do_arm_ini", _arm_ini);
n.getParam("do_follow_line", _follow_line);
n.getParam("do_back_line", _back_line);
n.getParam("do_arm_home", _arm_home);

cout << "get param : [ arm inital - "  << boolalpha << _arm_ini << " , " 
                   << "follow line - " << boolalpha << _follow_line << " , "
                   << "back line - "   << boolalpha << _back_line << " , "
                   << "arm home - "    << boolalpha << _arm_home << " ]" << endl;

ros::ServiceClient SC_arm_fix;       // arm-fixed           service-client
ros::ServiceClient SC_back_line;     // back-to-startline   service-client
SC_arm_fix = n.serviceClient<find_line::ArmFixPoint>("arm_fix_position_srv");
SC_back_line = n.serviceClient<find_line::FollowByRecord>("follow_record_srv");
actionlib::SimpleActionClient<find_line::RedLineInfoAction> AC_follow_line("follow_line_as", true);
AC_follow_line.waitForServer();

//ros::ServiceClient SC_follow_line;   // line-following      service-client  // action ??
//SC_follow_line = n.serviceClient<find_line::TaskDone>("ss_follow_line");


int readValue = 0;

while(n.ok())
{
    readValue = 0;

    // Initial Arm Position ---------------------------------------------------
if(_arm_ini){
    cout << "\n" << "Ready to start? ( 1 : Yes  ;  otherwise : No ) >>";
    cin >> readValue;
    if(! (readValue == 1) ){
        cout << "Exit ..." << endl;
        break;
    }
    readValue = 0;

    cout << "Initial arm ..." << endl;

    find_line::ArmFixPoint armSrv;
    armSrv.request.pos = "follow_line";
    if(SC_arm_fix.call(armSrv))
    {
        if(armSrv.response.status)
            cout << "follow_line done." << endl;
        else
            cout << "follow_line wrong command." << endl;
    }else{
        cerr << "Failed to call from service \"arm_fix_position_srv\" : follow_line."<< endl;
        break;
    }
}//endif
if(_follow_line){
    // ----------------------------------------------------------------

    cout << "\n" << "Ready to follow line ? ( 1 : Yes  ;  otherwise : No ) >>";
    cin >> readValue;
    if(! (readValue == 1) ){
        cout << "Exit ..." << endl;
        break;
    }
    readValue = 0;

    cout << "Follow Line ..." << endl;

    
    find_line::RedLineInfoGoal goal;
    goal.ready = true;
    AC_follow_line.sendGoal(goal, &doneCb, &activeCb, &feedbackCb);

    bool finished_before_timeout = AC_follow_line.waitForResult(ros::Duration(300.0));

    while (!(finished_before_timeout)){;}   // keep waiting

    if(finished_before_timeout){
        if(follow_success){
            cout << "Follow line complete." << endl;
        }
        else{
            cout << "Failed to Finish." << endl;
            break;
        }
    }else{
        cout << "Time out." << endl;
        break;
    }
}//endif
if(_back_line){
    // Back to start point -----------------------------------------------------------
    
    cout << "\n" << "Back to the start point? ( 1 : Yes  ;  otherwise : No ) >>";
    cin >> readValue;
    if(! (readValue == 1) ){
        cout << "Exit ..." << endl;
        break;
    }
    readValue = 0;

    cout << "Go back ..." << endl;

    find_line::FollowByRecord backSrv;
    backSrv.request.directory = "back";
    if(SC_back_line.call(backSrv))
    {
        if(backSrv.response.status)
            cout << "back_to done." << endl;
        else
            cout << "back_to wrong command." << endl;
    }else{
        cerr << "Failed to call from service \"follow_record_srv\" : back_to."<< endl;
        break;
    }
}//endif
if(_arm_home){
    // Arm Back Home ----------------------------------------------
    
    cout << "\n" << "Arm back to home? ( 1 : Yes  ;  otherwise : No ) >>";
    cin >> readValue;
    if(! (readValue == 1) ){
        cout << "Exit ..." << endl;
        break;
    }
    readValue = 0;

    cout << "Reset arm ..." << endl;

    find_line::ArmFixPoint armSrv;
    armSrv.request.pos = "home";
    if(SC_arm_fix.call(armSrv))
    {
        if(armSrv.response.status)
            cout << "Done." << endl;
        else
            cout << "Wrong command." << endl;
    }else{
        cerr << "Failed to call from \"arm_fix_position_srv\" : home."<< endl;
        break;
    }
} 
    cout << "\nFinish all.\nExit ..." << endl;
    break;
}// end of while

return 0;

}
