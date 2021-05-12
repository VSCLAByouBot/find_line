#!/usr/bin/env python
# Initial code created by Graylin Trevor Jay (tjay@cs.brown.edu) an published under Crative Commens Attribution license.
# addition for signal interrupt by Koen Buys

#import youbot_driver_ros_interface
#import roslib; roslib.load_manifest('youbot_oodl')
import rospy
import time
from geometry_msgs.msg import Twist
from std_msgs.msg import Char
import sys, select, termios, tty, signal
pub_time = 0.1# publish how long time
speed = 0.2
turn = 0.3
f = open('/home/huang/data2.txt','w')
f.write(str(pub_time)+'\n')
f.write(str(speed)+'\n')
f.write(str(turn)+'\n')


msg = """
Reading from the keyboard  and Publishing to Twist!
---------------------------
Moving around:
   u    i    o
   j    k    l
   m    ,    .

q/z : increase/decrease max speeds by 10%
w/x : increase/decrease only linear speed by 10%
e/c : increase/decrease only angular speed by 10%
anything else : stop

CTRL-C to quit
"""

moveBindings = {
#		     x,y,tetha ratio
		'i':(0.6,0,0), 	# forwards
		'o':(1,0,-1), 	# forwards + rotation right
		'j':(0,1,0), 	# left
		'l':(0,-1,0),	# right
		'u':(1,0,1), 	# forwards + rotation left
		',':(-1,0,0), 	# backward
		'.':(0,0,-1), 	# turn right on spot
		'm':(0,0,1), 	# turn left on spot
		#new add
		'r':(0.3,0,0.35), # left front 30 degree
		'e':(0.3,0,0.7), # left front 60 degree
		't':(0.3,0,-0.35),	#right front 30 degree
		'y':(0.3,0,-0.7), #right front 60 degree
		'd':(-0.57,1,0),	#left back 30 degree
		'f':(-1,0.57,0), #left back 60 degree
		'h':(-0.57,-1,0)#right back 30 degree
	       }
speedBindings={
		'q':(1.1,1.1),
		'z':(.9,.9),
		'w':(1.1,1),
		'x':(.9,1),
		'p':(1,1.1),
		'o':(1,.9)
	      }


class TimeoutException(Exception): 
    pass 


def vels(speed,turn):
	return "currently:\tspeed %s\tturn %s " % (speed,turn)

def callback(data):
	rospy.loginfo(rospy.get_caller_id() + "I heard %c", data.data)
	pub_to_car(data.data)

def pub_to_car(data):
	pub = rospy.Publisher('cmd_vel', Twist,queue_size=1)
	key = chr(data)
	print('key')
	print(key)
	f.write(key)
	global speed, turn
	if key in moveBindings.keys():
		x = moveBindings[key][0]
		y = moveBindings[key][1]
		th = moveBindings[key][2]
	elif key in speedBindings.keys():
		speed = speed * speedBindings[key][0]
		turn = turn * speedBindings[key][1]

		print vels(speed,turn)
		if (status == 14):
			print msg
		status = (status + 1) % 15
	else:
		x = 0
		y = 0
		th = 0
		#if (key == '\x03'):
			#break

	twist = Twist()
	twist.linear.x = x*speed 
	twist.linear.y = y*speed 
	twist.linear.z = 0

	twist.angular.x = 0 
	twist.angular.y = 0
	twist.angular.z = th*turn
	start = time.time()
	end = start
	if key == 'k':	 #stop
		print('stop')				
	else:
		while((end-start)<pub_time): # publish how long time
			pub.publish(twist)
			end = time.time()
			#print("end-start",end-start)
			if (key == '\x03'): #ctrl+c
				#print('break2')
				break
		print('stop')
		twist.linear.x = 0; twist.linear.y = 0; twist.linear.z = 0
		twist.angular.x = 0; twist.angular.y = 0; twist.angular.z = 0
		pub.publish(twist)
		
	

def listener():
	#pub = rospy.Publisher('cmd_vel', Twist)
	print('listener')
	rospy.init_node('teleop_twist_keyboard')
	rospy.Subscriber("chatter", Char, callback)
	print vels(speed,turn)
	x = 0
	y = 0
	th = 0
	status = 0
	rospy.spin()

if __name__=="__main__":
    	settings = termios.tcgetattr(sys.stdin)
	print msg
	listener()
	f.close()


