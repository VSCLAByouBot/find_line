#include "ros/ros.h"
#include "std_msgs/String.h"
#include "std_msgs/Char.h"
#include <sstream>
int main(int argc, char **argv)
{
  ros::init(argc, argv, "talker");
  ros::NodeHandle n;
  //ros::Publisher chatter_pub = n.advertise<std_msgs::String>("chatter", 10);
  ros::Publisher chatter_pub = n.advertise<std_msgs::Char>("chatter",10);
  ros::Rate loop_rate(10);

  if(ros::ok())
  {
    for(int i=0;i<10;i++)
    {
    ros::spinOnce();
    loop_rate.sleep();
    }
    //for(int i=0;i<10;i++)
    //{
    /*std_msgs::Char msg;
    msg.data = 'k';
    ROS_INFO("%c", msg.data);
     chatter_pub.publish(msg);*/
    //ros::spinOnce();
    //loop_rate.sleep();
     //}
    
  }
    std_msgs::Char msg;
    msg.data = 'i';
    ROS_INFO("%c", msg.data);
    chatter_pub.publish(msg);
    /*for(int i=0;i<50;i++)
    {
    ros::spinOnce();
    loop_rate.sleep();
    }*/
    msg.data = 'i';
    ROS_INFO("%c", msg.data);
    chatter_pub.publish(msg);
    /*msg.data = 'i';
    ROS_INFO("%c", msg.data);
    chatter_pub.publish(msg);*/
    msg.data = 'j';
    ROS_INFO("%c", msg.data);
    chatter_pub.publish(msg);
    
    //loop_rate.sleep();
    
    
    for(int i=0;i<50;i++)
    {
    ros::spinOnce();
    loop_rate.sleep();
    }


  return 0;
}

