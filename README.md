## Project:Visual Guidance of redline with camera
## Environment Setup
- Environment:Ubuntu 16.04 + ROS Kinetic
- KUKA youBot
- [ROS Image Proc Package](http://wiki.ros.org/image_proc)
- Camera:flea3 point grey camera 
## Background
## How to compile the code
- compile it with ROS catkin_make
## The order of executing the code
1.launch the youbot and camera driver
```
$ roslaunch youbot_driver_ros_interface youbot_driver.launch
$ roslaunch pointgrey_camera_driver camera.launch
```
2.Open the image_proc package
```
$ ROS_NAMESPACE=/camera rosrun image_proc image_proc
```
Then you can see the picture by the instruction
```
$ rosrun image_view image_view image:=my_camera/image_rect_color
```
3. Open the node
## File Description
- findline3.cpp
  - It is an unfinished code, we want to modify the window of findline2.cpp which was finding the redline, to turn the window moveable.
- findline2.cpp
- arm_init.cpp
- go_back.py
- 
## Method
- Node架構圖
