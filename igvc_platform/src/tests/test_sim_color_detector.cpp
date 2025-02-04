#include <gtest/gtest.h>
#include <ros/ros.h>
#include <mocking_utils/mock_subscriber.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/CameraInfo.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <parameter_assertions/assertions.h>

class TestSimColorDetector : public testing::Test
{
public:
  TestSimColorDetector()
  {
    handle.getParam("sim_color_detector/camera_names", camera_names);

    for (size_t i; i < camera_names.size(); ++i)
    {
      auto camera_name = camera_names[i];
      std::string img_name = camera_name + "/raw/image";
      std::string info_name = camera_name + "/raw/camera_info";

      ros::Publisher img_pub = handle.advertise<sensor_msgs::Image>(img_name, 1);
      ros::Publisher info_pub = handle.advertise<sensor_msgs::CameraInfo>(info_name, 1);

      mock_image_pubs.push_back(img_pub);
      mock_info_pubs.push_back(info_pub);
    }
  }

protected:
  ros::NodeHandle handle;
  std::vector<ros::Publisher> mock_image_pubs;
  std::vector<ros::Publisher> mock_info_pubs;
  std::vector<std::string> camera_names;
};

sensor_msgs::Image createCircleImgMsg(double height, double width, double h, double s, double v)
{
  cv::Mat3f hsv(cv::Vec3f(h, s, v));
  cv::Mat3f bgr;
  cvtColor(hsv, bgr, CV_HSV2BGR);

  cv::Mat cv_img = cv::Mat(height, width, CV_8UC3, cv::Scalar(0, 0, 0));

  cv::Point center = cv::Point(height / 2, width / 2);
  cv::circle(cv_img, center, width / 8, cv::Scalar(bgr.data[0], bgr.data[1], bgr.data[2]), cv::LineTypes::FILLED,
             cv::LineTypes::LINE_8);

  cv_bridge::CvImage img_bridge;
  sensor_msgs::Image img_msg;

  std_msgs::Header header;
  img_bridge = cv_bridge::CvImage(header, sensor_msgs::image_encodings::BGR8, cv_img);
  img_bridge.toImageMsg(img_msg);

  return img_msg;
}

TEST_F(TestSimColorDetector, CircleTest)
{
  for (size_t i; i < camera_names.size(); ++i)
  {
    std::string sub_name = camera_names[i] + "/segmented/image";
    MockSubscriber<sensor_msgs::Image> mock_sub(sub_name);
    ASSERT_TRUE(mock_sub.waitForPublisher());
    ASSERT_TRUE(mock_sub.waitForSubscriber(mock_image_pubs[i]));

    const double height = 480;
    const double width = 640;
    const double h = 160;
    const double s = 240;
    const double v = 240;

    sensor_msgs::Image img_msg = createCircleImgMsg(height, width, h, s, v);
    sensor_msgs::CameraInfo info_msg;
    ros::Time curr = ros::Time::now();
    img_msg.header.stamp = curr;
    info_msg.header.stamp = curr;

    mock_image_pubs[i].publish(img_msg);
    mock_info_pubs[i].publish(info_msg);

    ASSERT_TRUE(mock_sub.spinUntilMessages());

    ASSERT_EQ(mock_sub.messages().size(), 1LU);
    const sensor_msgs::Image& response = mock_sub.front();

    constexpr double expected_height = 400;
    constexpr double expected_width = 400;
    EXPECT_EQ(response.height, expected_height);
    EXPECT_EQ(response.width, expected_width);

    cv_bridge::CvImagePtr cv_ptr;
    try
    {
      cv_ptr = cv_bridge::toCvCopy(response, sensor_msgs::image_encodings::MONO8);
    }
    catch (cv_bridge::Exception& e)
    {
      ROS_ERROR("cv_bridge exception: %s", e.what());
      return;
    }

    cv::Mat expect_img = cv::Mat(height, width, CV_8UC1, cv::Scalar(0));
    cv::Mat expect_img_resize = cv::Mat(400, 400, CV_8UC1, cv::Scalar(0));

    cv::Point center = cv::Point(height / 2, width / 2);
    cv::circle(expect_img, center, width / 8, cv::Scalar(255), cv::LineTypes::FILLED, cv::LineTypes::LINE_8);
    cv::resize(expect_img, expect_img_resize, cv::Size(400, 400), 0, 0, cv::InterpolationFlags::INTER_AREA);

    int num_errors = 0;
    for (int i = 0; i < cv_ptr->image.rows; ++i)
    {
      for (int j = 0; j < cv_ptr->image.cols; j++)
      {
        int val = (int)cv_ptr->image.at<uchar>(i, j);
        int expect_val = (int)expect_img_resize.at<uchar>(i, j);
        if (val != expect_val)
        {
          num_errors++;
        }
      }
    }
    EXPECT_LT(num_errors, (400 * 400) * 0.01);
  }
}

sensor_msgs::Image createSolidImgMsg(double height, double width, double h, double s, double v)
{
  cv::Mat3f hsv(cv::Vec3f(h, s, v));
  cv::Mat3f bgr;
  cvtColor(hsv, bgr, CV_HSV2BGR);

  cv::Mat cv_img = cv::Mat(height, width, CV_8UC3, cv::Scalar(bgr.data[0], bgr.data[1], bgr.data[2]));

  cv_bridge::CvImage img_bridge;
  sensor_msgs::Image img_msg;  // >> message to be sent

  std_msgs::Header header;  // empty header
  img_bridge = cv_bridge::CvImage(header, sensor_msgs::image_encodings::BGR8, cv_img);
  img_bridge.toImageMsg(img_msg);  // from cv_bridge to sensor_msgs::Image

  return img_msg;
}

TEST_F(TestSimColorDetector, AllLineTest)
{
  for (size_t i; i < camera_names.size(); ++i)
  {
    std::string sub_name = camera_names[i] + "/segmented/image";
    MockSubscriber<sensor_msgs::Image> mock_sub(sub_name);
    ASSERT_TRUE(mock_sub.waitForPublisher());
    ASSERT_TRUE(mock_sub.waitForSubscriber(mock_image_pubs[i]));

    const double height = 480;
    const double width = 640;
    const double h = 160;
    const double s = 240;
    const double v = 240;

    sensor_msgs::Image img_msg = createSolidImgMsg(height, width, h, s, v);
    sensor_msgs::CameraInfo info_msg;  // blank msg. ok bc dont care about this
    ros::Time curr = ros::Time::now();
    img_msg.header.stamp = curr;
    info_msg.header.stamp = curr;  // important that both messages have same time stamp

    mock_image_pubs[i].publish(img_msg);
    mock_info_pubs[i].publish(info_msg);

    ASSERT_TRUE(mock_sub.spinUntilMessages());

    ASSERT_EQ(mock_sub.messages().size(), 1LU);
    const sensor_msgs::Image& response = mock_sub.front();

    const double expected_height = 400;
    const double expected_width = 400;
    EXPECT_EQ(response.height, expected_height);
    EXPECT_EQ(response.width, expected_width);

    cv_bridge::CvImagePtr cv_ptr;
    try
    {
      cv_ptr = cv_bridge::toCvCopy(response, sensor_msgs::image_encodings::MONO8);
    }
    catch (cv_bridge::Exception& e)
    {
      ROS_ERROR("cv_bridge exception: %s", e.what());
      return;
    }

    for (int i = 0; i < cv_ptr->image.rows; i++)
    {
      for (int j = 0; j < cv_ptr->image.cols; j++)
      {
        int val = (int)cv_ptr->image.at<uchar>(i, j);
        EXPECT_EQ(val, 255);
      }
    }
  }
}

int main(int argc, char** argv)
{
  ros::init(argc, argv, "test_sim_color_detector");
  testing::InitGoogleTest(&argc, argv);

  int ret = RUN_ALL_TESTS();
  ros::shutdown();
  return ret;
}
