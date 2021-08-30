#!/usr/bin/env python
import rospy
import actionlib
import find_line.msg
from geometry_msgs.msg import Twist
import time

ac_start = rospy.get_param('/using_action', True)
rec = rospy.get_param('/record_path', True)

moveBindings = {
	# x,y,tetha ratio
	'i':(0.6, 0, 0), 		# forwards
	'r':(0.3, 0, 0.35), 	# left  front 30 degree
	'e':(0.3, 0, 0.7), 		# left  front 60 degree
	't':(0.3, 0, -0.35),	# right front 30 degree
	'y':(0.3, 0, -0.7), 	# right front 60 degree
	# 'k':(0, 0, 0)			# stop
}
moveMeaning = {
	'i': "forwards",
	'r': "left front 30 degree",
	'e': "left front 60 degree",
	't': "right front 30 degree",
	'y': "right front 60 degree",
	'k': "stop",
}

class FollowLine():

    _feedback = find_line.msg.RedLineInfoFeedback()
    _result = find_line.msg.RedLineInfoResult()
    thres = 1000
    pub_time = 0.1
    speed = 0.2
    turn = 0.3

    def __init__(self):

        f = open("/home/yyn/test_ws/src/find_line/record/pace.txt", "w")
        f.write(str(self.pub_time)); f.write("\n")
        f.write(str(self.speed));    f.write("\n")
        f.write(str(self.turn));     f.write("\n")
        f.close()

        self.slope = 0
        self.up_px = self.thres
        self.dn_px = self.thres

        rospy.Subscriber("line_slope_and_px", find_line.msg.SlopeAndPixel, self.slope_and_px_cb)
        self.ac_server = actionlib.SimpleActionServer('follow_line_as', find_line.msg.RedLineInfoAction, execute_cb=self.exe_cb, auto_start=False)
        
        if ac_start:
            self.ac_server.start()
        else:
            rospy.sleep(1)  # delay one sec

            loop_rate = rospy.Rate(1)
            while not rospy.is_shutdown():
                if self.cal_white_px(self.up_px, self.dn_px):
                    pace = self.go_by_slope(self.slope)
                    print("Slope: %3f => pace : %c ; %s" % (self.slope, pace, moveMeaning[pace]))  
                    self.pub_to_car(pace)
                else:
                    self.pub_to_car('k')
                    break

                loop_rate.sleep()
        #rospy.spin()

    def slope_and_px_cb(self, data):
        
        self.slope = data.slope
        self.up_px = data.up_px
        self.dn_px = data.down_px
        #print("----------------------------------------")
        #print("Slope: %3f ; White up : %d ;  White dn : %d" % (data.slope, data.up_px, data.down_px))
        
    def exe_cb(self, goal):

        if goal.ready:

            #loop_rate = rospy.Rate(1)   # Hz
            success = True
            #print('Start action processing.')
            pace = 'z'

            while self.cal_white_px(self.up_px, self.dn_px):

                if self.ac_server.is_preempt_requested():
                    print("Preempt occured.")
                    self.ac_server.set_preempted()
                    success = False
                    break
                
                new_pace = self.go_by_slope(self.slope)

                if new_pace == pace:
                    #rospy.sleep(1)  # delay one sec
                    self.delay_second(0.1)
                pace = new_pace

                print("Slope: %3f => pace : %c ; %s" % (self.slope, pace, moveMeaning[pace]))

                self.pub_to_car(pace)

                self._feedback.slope = self.slope
                self._feedback.up_px = self.up_px
                self._feedback.dn_px = self.dn_px
                self._feedback.pace  = pace
                #self.ac_server.publish_feedback(self._feedback)

                #loop_rate.sleep()

            if success:
                self._result.status = True
                #print('Action succeed.')
                self.ac_server.set_succeeded(self._result)

    def delay_second(self, sec):
        start = time.time()
        end = start
        while((end-start) < sec): # publish how long time
            end = time.time()

    def cal_white_px(self, up, dn):

        if up < self.thres:
            print("Cannot detect pixiel in upper zone.")
            self.pub_to_car('k')
            return False

        elif dn < self.thres :
            print("Cannot detect pixiel in lower zone.")
            self.pub_to_car('k')
            return False
        
        else:
            return True

    def go_by_slope(self, slp):

        if -0.1 <= slp and slp < 0.1:
            return 'i'
        elif 0.1 <= slp and slp < 1.1:
            return 't'
        elif 1.1 <= slp and slp < 5:
            return 'y'
        elif -1.1 <= slp and slp < -0.1:
            return 'r'
        elif -5 <= slp and slp < -1.1:
            return 'e'
        else:
            return 'k'
        
    def pub_to_car(self, pace):
        pub = rospy.Publisher('cmd_vel', Twist, queue_size=1)        

        if rec:
            self.record_pace(pace)

        if pace in moveBindings.keys():
            x  = moveBindings[pace][0]
            y  = moveBindings[pace][1]
            th = moveBindings[pace][2]
        else:
            x = 0;  y = 0;  th = 0;

        twist = Twist()
        twist.linear.x = x * self.speed
        twist.linear.y = y * self.speed
        twist.linear.z = 0

        twist.angular.x = 0 
        twist.angular.y = 0
        twist.angular.z = th * self.turn

        if pace == 'k':
            print("stop.")
            twist.linear.x = 0; twist.linear.y = 0; twist.linear.z = 0;
            twist.angular.x = 0; twist.angular.y = 0; twist.angular.z = 0;
            pub.publish(twist)
        else:

            #print("> linear  x = %.3lf ; y = %.3lf ; z = %.3lf" % (twist.linear.x, twist.linear.y, twist.linear.z))
            #print("> Angular x = %.3lf ; y = %.3lf ; z = %.3lf" % (twist.angular.x, twist.angular.y, twist.angular.z))
            #print("-------------------------------------\n")

            pub.publish(twist)

            #twist.linear.x = 0; twist.linear.y = 0; twist.linear.z = 0;
            #twist.angular.x = 0; twist.angular.y = 0; twist.angular.z = 0;
            #pub.publish(twist)

    def record_pace(self, pace):
        f = open("/home/yyn/test_ws/src/find_line/record/pace.txt", "a")
        f.write(pace)
        f.close()
        
if __name__=="__main__":
    rospy.init_node('follow_path_server')
    ac = FollowLine()
    rospy.spin()