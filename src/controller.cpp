#include "controller.hpp"

#include <windows.h>

#include <chrono>
#include <cstdio>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <thread>
#include <vector>

#include "util/image.hpp"
#include "util/util.hpp"

namespace minesweeper_solver
{

bool Controller::find_game()
{
  if (!(handle_main = FindWindow(NULL, "Minesweeper")))
  {
    return false;
  }
  if (!(handle_inner = FindWindowEx(this->handle_main, NULL, "Static", NULL)))
  {
    return false;
  }
  return true;
}

bool Controller::update_map()
{
  if (!find_game())
  {
    return false;
  }
  focus();
  cv::Mat screenshot = this->take_screenshot();
  // TODO: validate screenshot
  this->_map = this->analyze(screenshot).second;
  return true;
}

void Controller::click(int x, int y)
{
  focus();
  cv::Rect w = this->window_rect();
  int click_x = w.x + this->_block_size.width / 2 + this->_map_rect.x +
                x * this->_block_size.width,
      click_y = w.y + this->_block_size.height / 2 + this->_map_rect.y +
                y * this->_block_size.height;
  SetCursorPos(click_x, click_y);
  mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
  mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

void Controller::generate_train_data()
{
  cv::Mat s = this->take_screenshot();
  auto map = this->analyze(s);
  cv::Rect mr = map.first.first;   // map rect
  cv::Size b = map.first.second;   // block size
  cv::Size ms = map.second.size(); // map size
  // remove border
  int mox = mr.x + 2; // map offset x
  int moy = mr.y + 2; // map offset y

  for (int i = 0; i < ms.height; ++i)
  {
    for (int j = 0; j < ms.width; ++j)
    {
      cv::Mat img;
      img = s(cv::Rect(mox + b.width * j + 2, moy + b.height * i + 2,
        b.width - 4, b.height - 4));
      if (img.rows != 14 or img.cols != 14)
      {
        cv::resize(img, img, cv::Size(14, 14));
      }
      char file[1024];
      sprintf(file, "%s/%02d,%02d.jpg", GENERATED_TRAIN_DATA.c_str(), j, i);
      cv::imwrite(file, img);
    }
  }
}

cv::Rect Controller::window_rect()
{
  RECT w;
  GetWindowRect(this->handle_inner, &w);
  return cv::Rect(w.left, w.right, w.right - w.left, w.bottom - w.top);
}

void Controller::focus()
{
  SetForegroundWindow(this->handle_main);
}

cv::Mat Controller::take_screenshot()
{
  cv::Rect w = this->window_rect();

  LPVOID img = new char[w.width * w.height * 4];

  HDC window_DC = GetWindowDC(this->handle_inner);
  HDC memory_DC = CreateCompatibleDC(window_DC);

  HBITMAP bitmap = CreateCompatibleBitmap(window_DC, w.width, w.height);
  SelectObject(memory_DC, bitmap);

  BitBlt(memory_DC, 0, 0, w.width, w.height, window_DC, 0, 0, SRCCOPY);
  GetBitmapBits(bitmap, w.width * w.height * 4, img);

  cv::Mat screenshot = cv::Mat(w.height, w.width, CV_8UC4, img);
  // cv::imwrite("screenshot.jpg", screenshot);

  DeleteDC(window_DC);
  DeleteDC(memory_DC);
  DeleteObject(img);

  return screenshot;
}

std::pair<std::pair<cv::Rect, cv::Size>, cv::Mat> Controller::analyze(
  const cv::Mat& screenshot)
{
  // reference: https://blog.csdn.net/yomo127/article/details/52045146
  using namespace std;
  using namespace cv;

  cv::Rect map_rect;
  cv::Size block_size;

  int row = 0, column = 0;
  cv::Mat map;

  /* step 1:
    get gray image and adaptive threshold image
  */

  Mat img_gray, img_threshold;
  cvtColor(screenshot, img_gray, COLOR_BGR2GRAY); // get gray image
  adaptiveThreshold(~img_gray, img_threshold, 255, ADAPTIVE_THRESH_MEAN_C,
    THRESH_BINARY, 15, -2); // get adaptive threshold image form gray image

  /* step 2:
    sperate horizontal and vertical line by applying erode then dilate to
    threshold image
  */

  // define sperate line scale, the bigger the more lines would be detected
  int scale = 15;
  Mat hl = img_threshold.clone(); // horizontal line
  Mat vl = img_threshold.clone(); // vertical line
  // sperate horizontal line
  Mat hs = getStructuringElement(MORPH_RECT, Size(hl.cols / scale, 1));
  erode(hl, hl, hs, Point(-1, -1));
  dilate(hl, hl, hs, Point(-1, -1));
  // sperate vertical line
  Mat vs = getStructuringElement(MORPH_RECT, Size(1, vl.rows / scale));
  erode(vl, vl, vs, Point(-1, -1));
  dilate(vl, vl, vs, Point(-1, -1));

  /* step 3
    get mask and joint from horizontal and vertical line for later operation
  */

  Mat img_mask = hl + vl;
  Mat img_joints;
  // do intersection to horizontal and vertical line, get joints
  bitwise_and(hl, vl, img_joints);

  /* step 4
    using findContours to find the game map location, set up map_rect
  */

  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;
  vector<Rect> tables;
  findContours(img_mask, contours, hierarchy, RETR_EXTERNAL,
    CHAIN_APPROX_SIMPLE, Point(0, 0));
  for (size_t i = 0; i < contours.size(); i++)
  {
    // filter out small contours
    if (contourArea(contours[i]) > 100)
    {
      Rect bound_rect;
      vector<Point> contour_poly;
      approxPolyDP(Mat(contours[i]), contour_poly, 3, true);
      bound_rect = boundingRect(Mat(contour_poly));

      // find the number of joints that each table has
      Mat roi = img_joints(bound_rect);
      vector<vector<Point> > joints_contours;
      findContours(roi, joints_contours, RETR_CCOMP, CHAIN_APPROX_SIMPLE);
      // 从表格的特性看，如果这片区域的点数小于4，那就代表没有一个完整的表格，忽略掉
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
  map_rect = tables[max]; // set up map cv::Rect

  /* step 5
    get intersection joints from horizontal and vertical line, count them
    to determine row and column count, and determine block size.
  */

  // the gap between blocks, depended on the size of each block, introduce this
  // concept to cope with the joints clustered problem.
  int gap = 5;

  // counting horizontal joints for row count
  vector<int> counts;
  for (int i = 0; i < img_joints.cols; ++i)
  {
    int count = 0;
    for (int j = 0; j < img_joints.rows; ++j)
    {
      if (img_joints.at<uint8_t>(j, i) > 0)
      {
        ++count;
        j += gap; // jump the gap, avoid cluster
      }
    }
    if (count > 0) // omit empty
    {
      counts.push_back(count);
    }
  }
  // the most number of joints should be the row number
  row = util::get_majority_number(counts, screenshot.rows) - 1;

  // counting vertical joints for column count, reuse the counts variable
  counts = vector<int>();
  for (int i = 0; i < img_joints.rows; ++i)
  {
    int count = 0;
    for (int j = 0; j < img_joints.cols; ++j)
    {
      if (img_joints.at<uint8_t>(i, j) > 0)
      {
        ++count;
        j += gap; // jump the gap, avoid cluster
      }
    }
    if (count > 0) // omit empty
    {
      counts.push_back(count);
    }
  }
  // the most number of joints should be the column number
  column = util::get_majority_number(counts, screenshot.cols) - 1;

  block_size.width = round(map_rect.width / (double)column);
  block_size.height = round(map_rect.height / (double)row);

  /* step 6:
   */

  // remove border
  int mox = map_rect.x + 2; // map offset x
  int moy = map_rect.y + 2; // map offset y

  // initialize by creating zero row for the later operation of row concat
  // zero row will be removed after data loaded
  Mat all_block = Mat::zeros(1, 14 * 14 * 3, CV_32F);
  for (int i = 0; i < row; ++i)
  {
    for (int j = 0; j < column; ++j)
    {
      Mat img = screenshot(
        Rect(mox + block_size.width * j + 2, moy + block_size.height * i + 2,
          block_size.width - 4, block_size.height - 4));
      img = util::transform_image2(img);
      vconcat(all_block, img, all_block);
    }
  }
  // remove the first zero row created while initializing
  all_block = all_block.rowRange(1, all_block.rows);

  // use classifier to predict all the blocks
  Mat result = this->classifier.predict(all_block);

  map = Mat::zeros(row, column, CV_32S);
  // analyze result, use the maximum probability as the block value
  for (int i = 0; i < result.rows; ++i)
  {
    Point max;
    minMaxLoc(result.rowRange(i, i + 1), NULL, NULL, NULL, &max);
    map.at<int>(i / column, i % column) = max.x;
  }
  return std::make_pair(std::make_pair(map_rect, block_size), map);
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