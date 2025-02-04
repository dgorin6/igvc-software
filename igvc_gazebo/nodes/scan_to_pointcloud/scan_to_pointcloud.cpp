#include "scan_to_pointcloud.h"

scan_to_pointcloud::scan_to_pointcloud() : private_node_handle{ "~" }
{
  _pointcloud_pub = node_handle.advertise<pcl::PointCloud<pcl::PointXYZ>>("/pc2", 1);
  _pointcloud_sub = node_handle.subscribe("/scan", 1, &scan_to_pointcloud::scanCallback, this);

  assertions::getParam(private_node_handle, "min_dist", min_dist);
  assertions::getParam(private_node_handle, "neighbor_dist", neighbor_dist);
  assertions::getParam(private_node_handle, "offset", offset);
  scan_to_pointcloud::spinnerUpdate();
}

void scan_to_pointcloud::spinnerUpdate()
{
  while (ros::ok())
  {
    ros::spinOnce();
  }
}

void scan_to_pointcloud::scanCallback(const sensor_msgs::LaserScan::ConstPtr &msg)
{
  sensor_msgs::LaserScan scanData = *msg;

  auto rangeIsValid = [&scanData](float range) {
    return !std::isnan(range) && range > scanData.range_min && range < scanData.range_max;
  };

  for (unsigned int i = 0; i < scanData.ranges.size(); ++i)
  {
    float &range = scanData.ranges[i];
    if (range > scanData.range_max || range < scanData.range_min)
      continue;
    // Too close
    if (range < min_dist)
    {
      range = scanData.range_max + 1;
    }
    // No valid neighbors
    else if ((i == 0 || !rangeIsValid(scanData.ranges[i - 1])) &&
             (i == (scanData.ranges.size() - 1) || !rangeIsValid(scanData.ranges[i + 1])))
    {
      range = scanData.range_max + 1;
    }
    // No close neighbors
    else if ((i == 0 || abs(scanData.ranges[i - 1] - range) > neighbor_dist) &&
             (i == (scanData.ranges.size() - 1) || abs(scanData.ranges[i + 1] - range) > neighbor_dist))
    {
      range = scanData.range_max + 1;
    }
  }
  sensor_msgs::PointCloud2 cloud;
  projection.projectLaser(scanData, cloud);
  cloud.header.frame_id = "/lidar";

  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_for_pub(new pcl::PointCloud<pcl::PointXYZ>());

  fromROSMsg(cloud, *cloud_for_pub);
  tf::Quaternion quaternion_mag;
  quaternion_mag.setRPY(0, 0, offset);

  tf::Transform trans;
  trans.setRotation(quaternion_mag);
  pcl_ros::transformPointCloud(*cloud_for_pub, *cloud_for_pub, trans);
  _pointcloud_pub.publish(*cloud_for_pub);
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "scan_to_pointcloud");
  scan_to_pointcloud scan_object;
  ros::spin();
}