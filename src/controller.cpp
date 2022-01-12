#include "controller.hpp"

#include <windows.h>

#include <cstdio>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
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
  GetWindowRect(this->handle_inner, &this->window_rect);
  this->window_size =
    std::make_pair(this->window_rect.right - this->window_rect.left,
      this->window_rect.bottom - this->window_rect.top);
}

void Controller::focus()
{
  SetForegroundWindow(this->handle_main);
}

cv::Mat Controller::get_screenshot()
{
  this->update_window_info();

  int width = this->window_size.first;
  int height = this->window_size.second;

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
  using namespace cv;

  Mat img_gray, img_threshold;

  Mat hl; // horizontal line
  Mat vl; // vertical line
  Mat img_mask, img_joints;

  /* step 1. get gray image and adaptive threshold image */
  // get gray image
  cvtColor(s, img_gray, COLOR_BGR2GRAY);
  // get adaptive threshold image form gray image
  adaptiveThreshold(~img_gray, img_threshold, 255, ADAPTIVE_THRESH_MEAN_C,
    THRESH_BINARY, 15, -2);

  /* step 2. sperate line by applying erode then dilate */
  // sperate line scale, the bigger the more lines would be detected
  int scale = 20;
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

  /* step 3. get intersection joints of horizontal and vertical line, count them
  to determine row and column count, determine block size and map offsets at the
  same time. */
  /* TODO: add more comments for this method */
  int row, column, block_width, block_height;
  Point top_left, bottom_right;
  bool is_top_left_found = false;
  // do intersection to horizontal and vertical line, get joints
  bitwise_and(hl, vl, img_joints);

  // the gap between blocks, depended on the size of each block
  int gap = 10;
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
        if (!is_top_left_found)
        {
          top_left = {i, j};
          is_top_left_found = true;
        }
        bottom_right = {i, j};
        j += gap; // jump the gap
      }
    }
    if (count > 0)
    {
      counts.push_back(count);
    }
  }
  row = util::get_majority_number(counts, this->window_size.second) - 1;

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
        j += gap; // jump one gap
      }
    }
    if (count > 0)
    {
      counts.push_back(count);
    }
  }
  column = util::get_majority_number(counts, this->window_size.second) - 1;

  this->map_size = std::make_pair(column, row);
  this->map_offset = std::make_pair(top_left.x, top_left.y);
  this->block_size = std::make_pair<int, int>(
    round((bottom_right.x - top_left.x) / (double)column),
    round((bottom_right.y - top_left.y) / (double)row));

  // printf("top_left: (%d, %d), bottom_right: (%d, %d)\n", top_left.x,
  // top_left.y,
  //   bottom_right.x, bottom_right.y);
  // printf(
  //   "width: %d, height: %d\n", this->map_size.first, this->map_size.second);
  // printf("offset x: %d, offset y: %d\n", this->map_offset.first,
  //   this->map_offset.second);
  // printf("block width: %d, block height: %d\n", this->block_size.first,
  //   this->block_size.second);
}

void Controller::analyze_screenshot_map(const cv::Mat&)
{
}

std::vector<byte> Controller::get_map()
{
  return std::vector<byte>();
}

void Controller::click(int x, int y)
{
  focus();
  int click_x = this->window_rect.left + this->block_size.first / 2 +
                this->map_offset.first + x * this->block_size.first,
      click_y = this->window_rect.top + this->block_size.second / 2 +
                this->map_offset.second + x * this->block_size.second;
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

int main()
{
  using namespace minesweeper_solver;
  Controller c = Controller();
  auto test = c.get_map();

  c.click(0, 0);

  // Sleep(100);
  // for (int i = 0; i < 10 && GetForegroundWindow() == c.hm; ++i)
  // {
  //   cout << i << endl;
  //   c.click(i, i);
  //   Sleep(100);
  // }
}