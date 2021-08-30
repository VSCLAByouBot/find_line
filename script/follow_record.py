#!/usr/bin/env python
from __future__ import print_function

import rospy
from find_line.srv import FollowByRecord, FollowByRecordResponse
from geometry_msgs.msg import Twist
import time

pub_time = 0
speed = 0
turn = 0
path_reco = []

moveBindings = {
	# x,y,tetha ratio
	'i':(0.6, 0, 0), 		# forwards
	'r':(0.3, 0, 0.35), 	# left  front 30 degree
	'e':(0.3, 0, 0.7), 		# left  front 60 degree
	't':(0.3, 0, -0.35),	# right front 30 degree
	'y':(0.3, 0, -0.7), 	# right front 60 degree
	# 'k':(0, 0, 0)			# stop
}
moveDirection = {
    # request.directory
    "forward": 1,
    "back":   -1,
    "stop":    0,
}

def read_path():
    global path_reco, pub_time, speed, turn
    path_reco = []

    f = open("/home/yyn/test_ws/src/find_line/record/pace.txt", "r")
    pub_time = float(f.readline())
    speed = float(f.readline())
    turn = float(f.readline())

    print("[Info]")
    print('Publish time : %f' % pub_time)
    print('Linear speed : %f' % speed)
    print('Angular speed : %f' % turn)

    data = f.readline()

    f.close()

    for d in reversed(data):
        path_reco.append(d)

    path_reco = path_reco[2:-1]
    path_reco.append('k')

    #for d in path_reco:
    #    print("pace %c" % d)
    #print("Total data number = %d\n" % len(data))

def go_by_pace_server(req):
    #print("Go back depend on the path record.")

    read_path()

    dir = req.directory
    success = 0

    if dir in moveDirection.keys():
        for pp in path_reco:
            delay_second(0.1)
            go_by_pace(moveDirection[dir], pp)
            success = 1
    else:
        print("Wrong Request.")
    
    return FollowByRecordResponse(success)  # Success if = 1

def delay_second(sec):
    start = time.time()
    end = start
    while((end-start) < sec): # publish how long time
        end = time.time()

def go_by_pace(dir, pace):
    pub = rospy.Publisher('cmd_vel', Twist,queue_size=1)
    global speed, turn

    if pace in moveBindings.keys():
        x  = dir * moveBindings[pace][0]
        y  = dir * moveBindings[pace][1]
        th = dir * moveBindings[pace][2]
    else:
        x = 0;  y = 0;  th = 0;

    print("go % d : %c [ x = %f ; y = %f ; th = %f]" % (dir, pace, x, y, th))

    twist = Twist()
    twist.linear.x = x*speed
    twist.linear.y = y*speed
    twist.linear.z = 0

    twist.angular.x = 0 
    twist.angular.y = 0
    twist.angular.z = th*turn

    if pace == 'k':
        print("stop.")
    #else:
        
        #print("> linear  x = %.3lf ; y = %.3lf ; z = %.3lf" % (twist.linear.x, twist.linear.y, twist.linear.z))
        #print("> Angular x = %.3lf ; y = %.3lf ; z = %.3lf" % (twist.angular.x, twist.angular.y, twist.angular.z))
        #print("-------------------------------------\n")

    pub.publish(twist)

def youbot_goback():
    rospy.init_node('follow_record')
    gb_ss = rospy.Service('follow_record_srv', FollowByRecord, go_by_pace_server)
    rospy.spin();

if __name__=="__main__":
	youbot_goback()