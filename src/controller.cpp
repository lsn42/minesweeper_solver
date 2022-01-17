#include "controller.hpp"

#include <windows.h>

#include <cstdio>
#include <opencv2/opencv.hpp>
#include <vector>

#include "util/image.hpp"
#include "util/util.hpp"

namespace minesweeper_solver
{

Controller::Controller():
  handle_main(FindWindow(NULL, "Minesweeper")),
  handle_inner(FindWindowEx(this->handle_main, NULL, "Static", NULL))
{
  this->analyze_game_info(this->take_screenshot());
}

std::vector<int> Controller::map()
{
  this->analyze_game_map(this->take_screenshot());
  return this->_map;
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

void Controller::analyze_game_info(const cv::Mat& screenshot)
{
  // reference: https://blog.csdn.net/yomo127/article/details/52045146
  using namespace std;
  using namespace cv;

  Rect w = this->window_rect();

  /* step 1:
    get gray image and adaptive threshold image
  */

  Mat img_gray, img_threshold;
  // get gray image
  cvtColor(screenshot, img_gray, COLOR_BGR2GRAY);
  // get adaptive threshold image form gray image
  adaptiveThreshold(~img_gray, img_threshold, 255, ADAPTIVE_THRESH_MEAN_C,
    THRESH_BINARY, 15, -2);

  /* step 2:
    sperate horizontal and vertical line by applying erode then dilate
  */

  // define sperate line scale, the bigger the more lines would be detected
  int scale = 15;
  Mat hl, vl; // horizontal and vertical line

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
    get mask and joint for after operation
  */

  Mat img_mask = hl + vl;
  Mat img_joints;
  // do intersection to horizontal and vertical line, get joints
  bitwise_and(hl, vl, img_joints);

  /* step 4
    using findContours to find the game map location, set up map Rect
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

  /* step 5
    get intersection joints from horizontal and vertical line, count them
    to determine row and column count, and determine block size.
  */

  // the gap between blocks, depended on the size of each block, introduce this
  // concept to cope with the joints clustered problem.
  int gap = 5;

  int row, column;

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
  row = util::get_majority_number(counts, w.height) - 1;

  // counting vertical joints for column count
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
  column = util::get_majority_number(counts, w.width) - 1;

  // set up variebles
  this->info_lock.lock();
  this->_map_rect = tables[max];
  this->_map_size.width = column;
  this->_map_size.height = row;
  this->_block_size.width = round(this->_map_rect.width / (double)column);
  this->_block_size.height = round(this->_map_rect.height / (double)row);
  this->info_lock.unlock();
}

void Controller::analyze_game_map(const cv::Mat& screenshot)
{
  std::vector<int> map;
  // acquire and save info
  this->info_lock.lock();
  cv::Size ms = this->_map_size;  // map size
  cv::Rect mr = this->_map_rect;  // map rect
  cv::Size b = this->_block_size; // block size
  this->info_lock.unlock();
  // remove border
  int mox = this->_map_rect.x + 2; // map offset x
  int moy = this->_map_rect.y + 2; // map offset y

  // initialize by creating zero row for the later operation of row concat
  // zero row will be removed after data loaded
  cv::Mat all_block = cv::Mat::zeros(1, 14 * 14 * 3, CV_32F);
  for (int i = 0; i < ms.height; ++i)
  {
    for (int j = 0; j < ms.width; ++j)
    {
      cv::Mat img;
      //
      img = screenshot(cv::Rect(mox + b.width * j + 2, moy + b.height * i + 2,
        b.width - 4, b.height - 4));
      // img = util::transform_and_save_image1(
      //   img, "../data/img/gray_block", std::make_pair(j, i));
      img = util::transform_image2(img);
      cv::vconcat(all_block, img, all_block);
    }
  }
  // remove the first zero row which created while initialize
  all_block = all_block.rowRange(1, all_block.rows);

  // use classifier to predict all the blocks
  cv::Mat result = this->classifier.predict(all_block);

  // analyze result, use the maximum probability as the block value
  for (int i = 0; i < result.rows; ++i)
  {
    cv::Point max;
    cv::minMaxLoc(result.rowRange(i, i + 1), NULL, NULL, NULL, &max);
    map.push_back(max.x);
  }

  // verify info unchanged
  this->info_lock.lock();
  if (ms == this->_map_size && mr == this->_map_rect && b == this->_block_size)
  { // unchanged, set up the map
    this->info_lock.unlock();
    this->_map = map;
  }
  else
  { // changed, call this function again
    this->info_lock.unlock();
    this->analyze_game_map(screenshot);
  }
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