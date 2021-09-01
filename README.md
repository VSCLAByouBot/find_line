# Project: Visual Guidance of redline with camera (ver. 2)

## Environment Setup
- Environment: Ubuntu 18.04 + ROS Melodic
- Platform: KUKA youBot
- Visual Device: Pointgrey Flea3 camera 

## Different Features
- Addition of the launch file, which allows to open all the nodes at one time, and has no need to launch camera driver first
- Adjust the Architecture of nodes/ tasks, increase the ability of reuse of each nodes

## Execution

1. First, clone the whole repository to your workspace and compile by catkin. And prepare the sources.
    - [pointgrey_camera_driver](http://wiki.ros.org/pointgrey_camera_driver)
    - [camera_calibration](http://wiki.ros.org/camera_calibration)
    - [image_proc](http://wiki.ros.org/image_proc)

2. Adjust the filepathes in .../script/follow_line.py and .../script/follow_record.py

2. Launch the youbot camera driver
    ```
    roslaunch youbot_driver_ros_interface youbot_driver.launch
    ```

3. Launch youbot_findline.launch
    ```
    roslaunch find_line youbot_findline.launch
    ```

4. Wait for the "Find Line" window pop out and continue to follow the hints on the screen.

### Parameters of launch file

For node <find_red_line>

+ ```/image_pub``` <br>
  the result image will be published to topic /camera/find_red_line if it is set <i>true</i>, or directly show with cv::imshow if it is set <i>false</i>.

+ ```/image_save```<br>
  useless <s>Saving the result image if it is set <i>true</i></s>

+ ```/print_red_info```<br>
  (for debugging) print the information during line detection 

For node <youbot_find_line>

+ ```/do_arm_ini```、```/do_follow_line```、```/do_back_line```、```/do_arm_home```<br>
   enable each task
   
   
## Architecture of the Nodes

```
main.cpp <youbot_follow_redline>
 │  
 ├ ServiceClient "arm_fix_position_srv" pos:=follow_line
 │    │
 │    └ arm_fix_server.cpp <youbot_arm_fix_pos>
 │         ├ ServiceServer "arm_fix_position_srv"                 
 │         └ Publisher "arm_1/arm_controller/position_command"    → youBot arm
 │
 ├ SimpleActionClient "ac_follow_line" ready:=True
 │    │
 │    └ follow_line.py <follow_line>
 │         ├ SimpleActionServer "ac_follow_line"
 │         └ Subscriber "line_slope_and_px"
 │         └ Publisher "cmd_vel"                                  → youBot base
 │              │
 │              └ find_red_line.cpp <find_red_line>
 │                   └ Subscriber "camera/image_rect_color"       ← camera (after calibration)
 │                   └ Publisher  "line_slope_and_px"             → <follow_line>
 │                   └ Publisher  "camera/find_red_line"          → <image_view>
 │
 ├ ServiceClient <follow_record_srv> directory:="back"
 │    │
 │    └ follow_record.py <follow_record>
 │         ├ ServiceServer "follow_record_srv"
 │         ├ ( read from ".../find_line/record/pace.txt")
 │         └ Publisher "cmd_vel"                                  → youBot base
 │
 └ ServiceClient "arm_fix_position_srv" pos:=home
``` 

## Future work

<find_line_test.py> 和 <follow_record.py> 中讀取 txt 的方式是直接給予位址 string，由於針對不同電腦 home 資料夾的位置都不同，因此改起來會很麻煩

+ 方法一：在 launch 檔中增加 arg 或 param
+ 方法二：把 find_line 的 action feedback 改成 pace 的 char sequence，再當作 follow_record 的 request 送出

兩種方法應該都很好實踐，但是我很懶，就先不休了。
