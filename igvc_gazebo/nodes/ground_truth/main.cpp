#include <ros/ros.h>
#include <nav_msgs/Odometry.h>
#include <mutex>
#include <geometry_msgs/Vector3.h>
#include <tf/transform_broadcaster.h>
#include <tf/transform_listener.h>
#include <tf/transform_datatypes.h>
#include <igvc_utils/NodeUtils.hpp>

ros::Publisher ground_truth_pub;
std::mutex mutex;
nav_msgs::Odometry og_pose;
ros::Time last_estimate;


void odom_callback(const nav_msgs::Odometry::ConstPtr& msg) {
  last_estimate = msg->header.stamp;
}

void ground_truth_callback(const nav_msgs::Odometry::ConstPtr& msg) {

  std::lock_guard<std::mutex> truth_lock(mutex);
  if(msg->header.stamp.toSec() < 0.5) {
    og_pose.pose = msg->pose;
    og_pose.pose.pose.position.x = msg->pose.pose.position.x;
    og_pose.pose.pose.position.y = msg->pose.pose.position.y;
  } else {
    nav_msgs::Odometry result;
    result.pose = msg->pose;

    // rotate position into world frame
    result.pose.pose.position.x = msg->pose.pose.position.x - og_pose.pose.pose.position.x;
    result.pose.pose.position.y = msg->pose.pose.position.y - og_pose.pose.pose.position.y;


    // rotate velocity into world frame
    result.twist = msg->twist;
    result.twist.twist.linear.x = -msg->twist.twist.linear.x;
    result.twist.twist.linear.y = msg->twist.twist.linear.y;

    result.header = msg->header;
    result.child_frame_id = "/base_footprint";
    result.header.frame_id = "/odom";

    tf::Quaternion quat(msg->pose.pose.orientation.x, msg->pose.pose.orientation.y,
                        msg->pose.pose.orientation.z, msg->pose.pose.orientation.w);
    //quat *= tf::createQuaternionFromRPY(0, 0, 0);

    result.pose.pose.orientation.x = quat.x();
    result.pose.pose.orientation.y = quat.y();
    result.pose.pose.orientation.z = quat.z();
    result.pose.pose.orientation.w = quat.w();

    // publish odom message
    ground_truth_pub.publish(result);

    // publish transform for tf
    if(std::abs(msg->header.stamp.toSec() - last_estimate.toSec()) > 1.0) {
      static tf::TransformBroadcaster br;
      tf::Transform transform;
      transform.setOrigin( tf::Vector3(result.pose.pose.position.x, result.pose.pose.position.y, result.pose.pose.position.z) );
      transform.setRotation(quat);
      br.sendTransform(tf::StampedTransform(transform, msg->header.stamp, "odom", "base_footprint"));

    }
  }
}

int main(int argc, char** argv) {
  ros::init(argc, argv, "ground_truth_republisher");

  ros::NodeHandle nh;
  ros::NodeHandle pNh("~");

  std::string ground_truth_topic, estimate_topic, pub_topic, diff_topic;
  igvc::param(pNh, "ground_truth_sub_topic", ground_truth_topic, std::string("/ground_truth/state_raw"));
  igvc::param(pNh, "ground_truth_pub_topic", pub_topic, std::string("/ground_truth"));

  ros::Subscriber ground_truth = nh.subscribe<nav_msgs::Odometry>(ground_truth_topic, 10,
                                                         ground_truth_callback);

  ros::Subscriber estimate_sub = nh.subscribe<nav_msgs::Odometry>("/odometry/filtered", 1, odom_callback);
  ground_truth_pub = nh.advertise<nav_msgs::Odometry>(pub_topic, 1);

  ros::spin();
}
