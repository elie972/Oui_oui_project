#!/usr/bin/env python
# license removed for brevity
import rospy
from std_msgs.msg import Float64, String

class Speed_control:

    def __init__(self):
        rospy.init_node('talker', anonymous=True)
        self.pub_speed = rospy.Publisher('/dual_motor_controller/command', Float64, queue_size=10)
        self.pub_pos = rospy.Publisher('/tilt_controller/command', Float64, queue_size=10)
        rospy.Subscriber("/chatter", String, self.callback)
        rospy.Subscriber('/distance', Float64, self.changedist)
        rospy.sleep(10)
        #self.pub_speed.publish(0)
        #self.pub_pos.publish(1.5)
        #rospy.loginfo("change speed value")

        self.speed = 5
        self.pos = 0
        self.dist = 2000

        self.command = ''

        rate = rospy.Rate(1)

        #while not rospy.is_shutdown():
            #rospy.loginfo("I'm Running ! ")
            #rate.sleep()

        self.run()
    
    def callback(self, data):
        rospy.loginfo(data)
        self.command = data.data
    
    def changedist(self, data):
        self.dist = data.data

    def run(self):
        while not rospy.is_shutdown():
            if self.command == 'ForwardStart' :
                self.pub_speed.publish(- self.speed)
                #self.pub_pos.publish(self.pos)
            if self.command == 'BackwardStart' :
		if self.dist> 50 and self.dist< 300 :
                	self.pub_speed.publish((self.dist-100)/300*self.speed)
		else :
			self.pub_speed.publish(self.speed)
                #self.pub_pos.publish(self.pos)
            if self.command == 'ForwardStop' or self.command == 'BackwardStop' :
                self.pub_speed.publish(0)
		self.dist = 2000

            if self.command == 'RightStart' :
                self.pub_pos.publish(self.pos + 0.7)
                
            if self.command == 'LeftStart' :
                self.pub_pos.publish(self.pos - 0.7)
                
            if self.command == 'LeftStop' or self.command == 'RightStop' :
                self.pub_pos.publish(self.pos)
            #self.pub_speed.publish(0)
            rospy.sleep(0.1)

        

if __name__ == '__main__':
    try:
        speed = Speed_control()
        rospy.spin()
    except rospy.ROSInterruptException:
        pass
