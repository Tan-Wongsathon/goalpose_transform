#include <ros/ros.h>
#include <tf/transform_listener.h>
#include <tf2_ros/transform_listener.h>
#include <tf2_msgs/TFMessage.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/PoseWithCovariance.h>
#include <geometry_msgs/TransformStamped.h>
#include <nav_msgs/Odometry.h>

const double r = -10.1;
geometry_msgs::PoseStamped userpose_msg;
double Vx;
double Vy;
double Tx;
double Ty;


void goalpose_callback(const nav_msgs::Odometry::ConstPtr& goalpoint)
{ 
  ROS_INFO("From goalpose_cacllback loop");
  userpose_msg.header.seq = goalpoint->header.seq;
  userpose_msg.header.frame_id = "map";
  userpose_msg.header.stamp = goalpoint->header.stamp;
  userpose_msg.pose.position = goalpoint->pose.pose.position;
  userpose_msg.pose.orientation = goalpoint->pose.pose.orientation;

}


int main(int argc, char** argv){
  ros::init(argc, argv, "goalpose_transform");
  ros::NodeHandle node;
  tf2_ros::Buffer tfBuffer;
  tf2_ros::TransformListener tfListener(tfBuffer);

  ros::Publisher goalpose = 
    node.advertise<geometry_msgs::PoseStamped>("/move_base_simple/goal", 10);

  ros::Subscriber sub = node.subscribe("vo", 1000, goalpose_callback);

  //tf::TransformListener listener;

  ros::Rate rate(5.0);
  while (node.ok()){
    geometry_msgs::TransformStamped transformStamped;
    try{
      transformStamped = tfBuffer.lookupTransform("utm", "map", ros::Time(0)); 

    }
    catch (tf2::TransformException ex){
      ROS_ERROR("%s",ex.what());
      ros::Duration(1.0).sleep();
    }

    Vx = userpose_msg.pose.position.x - transformStamped.transform.translation.x;
    Vy = userpose_msg.pose.position.y - transformStamped.transform.translation.y;
    Tx = (Vx*cos(r))-(Vy*sin(r));
    Ty = (Vx*sin(r))+(Vy*cos(r));

    geometry_msgs::PoseStamped pose_msg;
    pose_msg.header = userpose_msg.header;
    //pose_msg.pose.position.x = userpose_msg.pose.position.x - transformStamped.transform.translation.x;
    //pose_msg.pose.position.y = userpose_msg.pose.position.y - transformStamped.transform.translation.y;
    pose_msg.pose.position.x = Tx;
    pose_msg.pose.position.y = Ty;
    pose_msg.pose.position.z = 0;
    pose_msg.pose.orientation.x = 0;
    pose_msg.pose.orientation.y = 0;
    //pose_msg.pose.orientation.z = atan((sqrt(pow((userpose_msg.pose.position.x - transformStamped.transform.translation.x),2.0)))
                                  //(sqrt(pow((userpose_msg.pose.position.y - transformStamped.transform.translation.y),2.0))));
    //pose_msg.pose.orientation.z = atan((transformStamped.transform.translation.y - userpose_msg.pose.position.y)
                                  //(transformStamped.transform.translation.x - userpose_msg.pose.position.x));
    pose_msg.pose.orientation.z = -atan((Tx)/(Ty));
    pose_msg.pose.orientation.w = 1.0;
    goalpose.publish(pose_msg);

  //ROS_INFO("I heard: [%f]", userpose_msg.position.x);
    ros::spinOnce();
    rate.sleep();
  }

  return 0;
};