cmake_minimum_required(VERSION 3.0.2)
project(find_line)

set(OpenCV_DIR /usr/share/OpenCV/)
set(CMAKE_CXX_STANDARD 11)

find_package(catkin REQUIRED COMPONENTS
  image_transport
  roscpp
  rospy
  sensor_msgs
  std_msgs
  cv_bridge
  OpenCV
  actionlib
  actionlib_msgs
  message_generation
)

add_message_files(
  FILES
  SlopeAndPixel.msg
)

add_service_files(
  FILES
  ArmFixPoint.srv
  FollowByRecord.srv
)

add_action_files(
  DIRECTORY action
  FILES
  RedLineInfo.action
)

generate_messages(
  DEPENDENCIES
  sensor_msgs
  std_msgs
  actionlib_msgs
)


catkin_package(
  CATKIN_DEPENDS
  actionlib_msgs
)

###########
## Build ##
###########

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
)

# CPP files

add_executable(youbot_find_line src/main.cpp)
target_link_libraries(youbot_find_line
  ${catkin_LIBRARIES}
)

add_executable(youbot_arm_fix_pos src/arm_fix_server.cpp)
target_link_libraries(youbot_arm_fix_pos
  ${catkin_LIBRARIES}
)

add_executable(camera_red_line src/find_red_line_camera.cpp)
target_link_libraries(
  camera_red_line
  ${catkin_LIBRARIES}
  ${OpenCV_INCLUDE_DIRS}
)


## python files
catkin_install_python(PROGRAMS
  script/follow_record.py
  script/follow_line_test.py
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
