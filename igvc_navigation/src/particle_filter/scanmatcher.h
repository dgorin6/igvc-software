#ifndef PROJECT_SCANMATCHER_H
#define PROJECT_SCANMATCHER_H
#pragma once

#include <boost/circular_buffer.hpp>

#include <pcl/point_cloud.h>
#include <pcl_ros/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/registration/icp.h>

#include <tf/transform_datatypes.h>
#include <tf_conversions/tf_eigen.h>
#include <ros/ros.h>
#include <igvc_utils/NodeUtils.hpp>
#include "octomapper.h"

class Scanmatcher {
public:
    explicit Scanmatcher(const ros::NodeHandle& pNh, const Octomapper& octomapper);
    double scanmatch(pcl::PointCloud<pcl::PointXYZ>::ConstPtr input, tf::Transform& transform,
            const tf::Transform& guess);
    double optimize(tf::Transform& optimized_transform, const pc_map_pair& pair,
            const tf::Transform& guess, const pcl::PointCloud<pcl::PointXYZ>::ConstPtr& cloud);

private:
    bool m_use_guess = false;
    int m_icp_iterations;
    double m_RANSAC_outlier_rejection;
    double m_correspondence_distance;
    double m_transformation_epsilon;
    pcl::PointCloud<pcl::PointXYZ>::ConstPtr m_last_cloud;
    ros::Publisher m_prev_pub;
    ros::Publisher m_last_pub;
    ros::Publisher m_next_pub;
    Octomapper m_octomapper;
};

#endif  // PROJECT_SCANMATCHER_H
