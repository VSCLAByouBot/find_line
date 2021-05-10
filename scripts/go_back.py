#!/usr/bin/env python
import rospy
import time
from geometry_msgs.msg import Twist
from std_msgs.msg import Char
import sys, select, termios, tty, signal
pub_time = 0
speed = 0
turn = 0
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


def pub_to_car(data):
	pub = rospy.Publisher('cmd_vel', Twist,queue_size=1)
	#key = chr(data)
	key = data
	print('key')
	global speed, turn
	if key in moveBindings.keys():
		x = moveBindings[key][0]
		y = moveBindings[key][1]
		th = moveBindings[key][2]
		print(key)
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
	twist.linear.x = x*speed*(-1) 
	twist.linear.y = y*speed*(-1) 
	twist.linear.z = 0

	twist.angular.x = 0 
	twist.angular.y = 0
	twist.angular.z = th*turn*(-1)
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

if __name__=="__main__":
	rospy.init_node('teleop_twist_keyboard')
	f = open("/home/huang/data2.txt","r")
	pub_time  = float(f.readline())
	print(pub_time)
	speed = float(f.readline())
	print(speed)
	turn = float(f.readline())
	print(turn)
	data = f.readline()
	for i in reversed(data):
		pub_to_car(i)
		#print(i)
	print(len(data))
	f.close()

