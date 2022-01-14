#include "controller.hpp"

#include <windows.h>

#include <cstdio>
#include <opencv2/opencv.hpp>
#include <vector>

#include "util/util.hpp"

namespace minesweeper_solver
{

Controller::Controller():
  handle_main(FindWindow(NULL, "Minesweeper")),
  handle_inner(FindWindowEx(this->handle_main, NULL, "Static", NULL))
{
  this->update_window_info();
  cv::Mat screenshot = this->get_screenshot();
  this->analyze_screenshot_dimension(screenshot);
}

void Controller::update_window_info()
{
  RECT w;
  GetWindowRect(this->handle_inner, &w);
  this->window_rect =
    cv::Rect(w.left, w.right, w.right - w.left, w.bottom - w.top);
}

void Controller::focus()
{
  SetForegroundWindow(this->handle_main);
}

cv::Mat Controller::get_screenshot()
{
  this->update_window_info();

  int width = this->window_rect.width;
  int height = this->window_rect.height;

  LPVOID img = new char[width * height * 4];

  HDC window_DC = GetWindowDC(this->handle_inner);
  HDC memory_DC = CreateCompatibleDC(window_DC);

  HBITMAP bitmap = CreateCompatibleBitmap(window_DC, width, height);
  SelectObject(memory_DC, bitmap);

  BitBlt(memory_DC, 0, 0, width, height, window_DC, 0, 0, SRCCOPY);
  GetBitmapBits(bitmap, width * height * 4, img);

  cv::Mat screenshot = cv::Mat(height, width, CV_8UC4, img);
  // cv::imwrite("screenshot.jpg", screenshot);

  DeleteDC(window_DC);
  DeleteDC(memory_DC);
  DeleteObject(img);

  return screenshot;
}

void Controller::analyze_screenshot_dimension(const cv::Mat& s)
{
  // reference: https://blog.csdn.net/yomo127/article/details/52045146
  using namespace cv;

  /* step 1:
    get gray image and adaptive threshold image
  */
  Mat img_gray, img_threshold;
  // get gray image
  cvtColor(s, img_gray, COLOR_BGR2GRAY);
  // get adaptive threshold image form gray image
  adaptiveThreshold(~img_gray, img_threshold, 255, ADAPTIVE_THRESH_MEAN_C,
    THRESH_BINARY, 15, -2);

  /* step 2:
    sperate line by applying erode then dilate
  */
  Mat hl; // horizontal line
  Mat vl; // vertical line
  // sperate line scale, the bigger the more lines would be detected
  int scale = 15;
  // sperate horizontal line
  hl = img_threshold.clone();
  Mat hs = getStructuringElement(MORPH_RECT, Size(hl.cols / scale, 1));
  erode(hl, hl, hs, Point(-1, -1));
  dilate(hl, hl, hs, Point(-1, -1));
  // sperate vertical line
  vl = img_threshold.clone();
  Mat vs = getStructuringElement(MORPH_RECT, Size(1, vl.rows / scale));
  erode(vl, vl, vs, Point(-1, -1));
  dilate(vl, vl, vs, Point(-1, -1));

  /* step 3
    get mask and joint.
  */
  Mat img_mask = hl + vl;
  Mat img_joints;
  int row, column, block_width, block_height;
  // do intersection to horizontal and vertical line, get joints
  bitwise_and(hl, vl, img_joints);
  Point top_left, bottom_right;
  /* step 4
    get intersection joints of horizontal and vertical line, count them
    to determine row and column count, determine block size and map offsets at
    the same time.
  */
  std::vector<std::vector<Point> > contours;
  std::vector<Vec4i> hierarchy;
  findContours(img_mask, contours, hierarchy, RETR_EXTERNAL,
    CHAIN_APPROX_SIMPLE, Point(0, 0));

  std::vector<cv::Rect> tables;
  for (size_t i = 0; i < contours.size(); i++)
  {
    // filter small contours
    if (contourArea(contours[i]) > 100)
    {
      cv::Rect bound_rect;
      std::vector<Point> contour_poly;
      approxPolyDP(Mat(contours[i]), contour_poly, 3, true);
      bound_rect = boundingRect(Mat(contour_poly));

      // find the number of joints that each table has
      Mat roi = img_joints(bound_rect);
      std::vector<std::vector<Point> > joints_contours;
      findContours(roi, joints_contours, RETR_CCOMP, CHAIN_APPROX_SIMPLE);
      //从表格的特性看，如果这片区域的点数小于4，那就代表没有一个完整的表格，忽略掉
      if (joints_contours.size() > 4)
      {
        tables.push_back(bound_rect);
      }
    }
  }
  int max = 0;
  for (int i = 0; i < tables.size(); ++i)
  {
    if (tables[i].area() > tables[max].area())
    {
      max = i;
    }
  }
  this->map_rect = tables[max];

  /* TODO: add more comments for this method */

  // the gap between blocks, depended on the size of each block
  int gap = 5;
  // counting horizontal joints for row count
  std::vector<int> counts;
  for (int i = 0; i < img_joints.cols; ++i)
  {
    int count = 0;
    for (int j = 0; j < img_joints.rows; ++j)
    {
      if (img_joints.at<uint8_t>(j, i) > 0)
      {
        ++count;
        j += gap; // jump the gap
      }
    }
    if (count > 0)
    {
      counts.push_back(count);
    }
  }
  row = util::get_majority_number(counts, this->window_rect.height) - 1;

  // counting vertical joints for column count
  counts = std::vector<int>();
  for (int i = 0; i < img_joints.rows; ++i)
  {
    int count = 0;
    for (int j = 0; j < img_joints.cols; ++j)
    {
      if (img_joints.at<uint8_t>(i, j) > 0)
      {
        ++count;
        j += gap; // jump the gap
      }
    }
    if (count > 0)
    {
      counts.push_back(count);
    }
  }
  column = util::get_majority_number(counts, this->window_rect.width) - 1;

  this->map_size.width = column;
  this->map_size.height = row;
  this->block_size.width = round(this->map_rect.width / (double)column);
  this->block_size.height = round(this->map_rect.height / (double)row);
}

void Controller::analyze_screenshot_map(const cv::Mat& screenshot)
{
  int map_width = this->map_size.width, map_height = this->map_size.height;
  int map_offset_x = this->map_rect.x + 4, map_offset_y = this->map_rect.y + 4;
  int block_width = this->block_size.width,
      block_height = this->block_size.height;

  // initialize by creating zero row for the later operation of row concat
  // zero row will be removed after data loaded
  cv::Mat all_block = cv::Mat::zeros(1, 14 * 14, CV_32F);
  for (int i = 0; i < map_height; ++i)
  {
    for (int j = 0; j < map_width; ++j)
    {
      cv::Mat img, img_gray, img_row;
      img = screenshot(cv::Rect(map_offset_x + block_width * j,
        map_offset_y + block_height * i, block_width - 4, block_height - 4));
      if (img.rows != 14 or img.cols != 14)
      {
        cv::resize(img, img, cv::Size(14, 14));
      }
      cv::cvtColor(img, img_gray, cv::COLOR_BGR2GRAY);
      char file[1024];
      sprintf(file, "../data/img/gray_block/%02d,%02d.jpg", j, i);
      cv::imwrite(file, img);

      img_gray.reshape(0, 1).convertTo(img_row, CV_32F);
      img_row /= 255.0;
      cv::vconcat(all_block, img_row, all_block);
    }
  }
  // remove the first zero row which created while initialize
  all_block = all_block.rowRange(1, all_block.rows);

  cv::Mat result = this->classifier.predict(all_block);

  for (int i = 0; i < result.rows; ++i)
  {
    double p = 0;
    cv::Point max;
    cv::minMaxLoc(result.rowRange(i, i + 1), NULL, &p, NULL, &max);
    this->map.push_back(max.x);
  }
  for (int i = 0; i < map_height; ++i)
  {
    for (int j = 0; j < map_width; ++j)
    {
      int v = this->map[i * map_width + j];
      char c;
      if (v >= 0 && v < 9)
      {
        printf(" %d", v);
      }
      else if (v == 9)
      {
        printf(" *");
      }
      else if (v == 10)
      {
        printf("  ");
      }
    }
    printf("\n");
  }
}

std::vector<int> Controller::get_map()
{
  cv::Mat screenshot = this->get_screenshot();
  this->analyze_screenshot_map(screenshot);
  return std::vector<int>();
}

void Controller::click(int x, int y)
{
  focus();
  int click_x = this->window_rect.x + this->block_size.width / 2 +
                this->map_rect.x + x * this->block_size.width,
      click_y = this->window_rect.y + this->block_size.height / 2 +
                this->map_rect.y + y * this->block_size.height;
  SetCursorPos(click_x, click_y);
  mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
  mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

void Controller::test()
{
  while (1)
  {
    std::cout << "click!" << std::endl;
    PostMessage(this->handle_main, WM_RBUTTONDOWN, MK_LBUTTON, 0);
    PostMessage(this->handle_main, WM_RBUTTONUP, 0, 0);
    Sleep(1000);
  }
}
} // namespace minesweeper_solver