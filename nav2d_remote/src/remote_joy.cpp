#include <ros/ros.h>
#include <sensor_msgs/Joy.h>
#include <std_srvs/Trigger.h>
#include <nav2d_operator/cmd.h>
#include <nav2d_navigator/commands.h>

/******************************************************
Buttons:
 0: A
 1: B
 2: X
 3: Y
 4: LB
 5: RB
 6: BACK
 7: START
 8: Logitech*
 9: Left Stick
10: Right Stick

 ******************************************************/

class Teleoperator
{
public:
	Teleoperator();
	
private:
	void joyCB(const sensor_msgs::Joy::ConstPtr& msg);

	ros::NodeHandle mNode;
	ros::Publisher mCommandPublisher;
	ros::Subscriber mJoySubscriber;
	ros::ServiceClient mStopClient;
	ros::ServiceClient mPauseClient;
	ros::ServiceClient mExploreClient;
	ros::ServiceClient mGetMapClient;
	
	int mAxisVelocity;
	int mAxisDirection;
	int mButtonDriveMode;
	int mButtonPauseNavigator;
	int mButtonStartExploration;
	int mButtonGetMap;
	int mButtonStop;
	
	bool mButtonPressed;
};

Teleoperator::Teleoperator()
{
	// Button and Axis configuration
	mAxisVelocity = 4;
	mAxisDirection = 0;

	mButtonDriveMode = 5;
	mButtonPauseNavigator = 6;
	mButtonStartExploration = 0;
	mButtonGetMap = 3;
	mButtonStop = 1;
	
	mCommandPublisher = mNode.advertise<nav2d_operator::cmd>("cmd", 1);
	mJoySubscriber = mNode.subscribe<sensor_msgs::Joy>("joy", 10, &Teleoperator::joyCB, this);
	mStopClient = mNode.serviceClient<std_srvs::Trigger>(NAV_STOP_SERVICE);
	mPauseClient = mNode.serviceClient<std_srvs::Trigger>(NAV_PAUSE_SERVICE);
	mExploreClient = mNode.serviceClient<std_srvs::Trigger>(NAV_EXPLORE_SERVICE);
	mGetMapClient = mNode.serviceClient<std_srvs::Trigger>(NAV_GETMAP_SERVICE);
	
	mButtonPressed = false;
}

void Teleoperator::joyCB(const sensor_msgs::Joy::ConstPtr& msg)
{
	// Ignore Button-Release events
	if(mButtonPressed)
	{
		mButtonPressed = false;
	}else
	{	
		nav2d_operator::cmd cmd;
		cmd.Turn = msg->axes[mAxisDirection] * -1.0;
		cmd.Velocity = msg->axes[mAxisVelocity];
		cmd.Mode = 0;
		if(msg->buttons[mButtonDriveMode]) cmd.Mode = 1;
		mCommandPublisher.publish(cmd);
	}

	if(msg->buttons[mButtonStop])
	{
		std_srvs::Trigger srv;
		if(!mStopClient.call(srv))
		{
			ROS_ERROR("Failed to send STOP_COMMAND to Navigator.");
		}
		return;
	}

	if(msg->buttons[mButtonPauseNavigator])
	{
		std_srvs::Trigger srv;
		if(!mPauseClient.call(srv))
		{
			ROS_ERROR("Failed to send PAUSE_COMMAND to Navigator.");
		}
		return;
	}

	if(msg->buttons[mButtonGetMap])
	{
		std_srvs::Trigger srv;
		if(!mGetMapClient.call(srv))
		{
			ROS_ERROR("Failed to send GETMAP_COMMAND to GetMap-Client.");
		}
		mButtonPressed = true;
		return;
	}

	if(msg->buttons[mButtonStartExploration])
	{
		std_srvs::Trigger srv;
		if(!mExploreClient.call(srv))
		{
			ROS_ERROR("Failed to send EXPLORE_COMMAND to Explore-Client.");
		}
		mButtonPressed = true;
		return;
	}
}

int main(int argc, char** argv)
{
	ros::init(argc, argv, "teleoperator");
	Teleoperator tele_op;
	
	ros::spin();
}
