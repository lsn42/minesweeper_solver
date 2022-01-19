#ifndef MINESWEEPER_SOLVER_UTIL_IMAGE_HPP
#define MINESWEEPER_SOLVER_UTIL_IMAGE_HPP
#include <opencv2/opencv.hpp>
namespace minesweeper_solver
{
namespace util
{
  inline cv::Mat transform_image1(cv::Mat image)
  {
    cv::Mat result = image.clone();
    if (result.rows != 14 or result.cols != 14)
    {
      cv::resize(result, result, cv::Size(14, 14));
    }
    // convert to gray image
    cv::cvtColor(result, result, cv::COLOR_BGR2GRAY);
    result.reshape(0, 1).convertTo(result, CV_32F);
    // normalization
    result /= 255.0;
    return result;
  }
  inline cv::Mat transform_image2(cv::Mat image)
  {
    cv::Mat i = image.clone();
    if (i.rows != 14 or i.cols != 14)
    {
      cv::resize(i, i, cv::Size(14, 14));
    }

    std::vector<cv::Mat> channels;
    split(image, channels);
    channels[0].reshape(0, 1).convertTo(channels[0], CV_32F);
    channels[1].reshape(0, 1).convertTo(channels[1], CV_32F);
    channels[2].reshape(0, 1).convertTo(channels[2], CV_32F);

    cv::Mat result = channels[0].clone();
    cv::hconcat(result, channels[1], result);
    cv::hconcat(result, channels[2], result);

    // normalization
    result /= 255.0;
    return result;
  }
} // namespace util
} // namespace minesweeper_solver
#endif