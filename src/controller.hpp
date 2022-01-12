#include <windows.h>

#include <opencv2/opencv.hpp>
#include <vector>
#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP
namespace minesweeper_solver
{
class Controller
{
 public:
  HWND handle_main;  // game process main handle
  HWND handle_inner; // inner window handle

  RECT window_rect;                // rect of inner window
  std::pair<int, int> window_size; // dimension(width, height) of inner window

  std::pair<int, int> map_offset; // offset(x, y) of the map on inner window
  std::pair<int, int> map_size;   // dimension(width, height) of game map
  std::pair<int, int> block_size; // dimension(width, height) of each block

  Controller();

  // get current game map
  std::vector<byte> get_map();
  // click specific block
  void click(int, int);

  void test();

 private:
  // update window rect
  void update_window_info();
  // put the game process at top and focus
  void focus();
  // get screen by controller handle
  cv::Mat get_screenshot();
  // analyze the screenshot, get dimension of the game window
  void analyze_screenshot_dimension(const cv::Mat&);
  // analyze the screenshot, get map of the game window
  void analyze_screenshot_map(const cv::Mat&);
};
} // namespace minesweeper_solver
#endif