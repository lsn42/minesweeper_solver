#ifndef MINESWEEPER_SOLVER_CONTROLLER_HPP
#define MINESWEEPER_SOLVER_CONTROLLER_HPP
#include <windows.h>

#include <opencv2/opencv.hpp>
#include <vector>

#include "ann.hpp"
namespace minesweeper_solver
{
class Controller
{
 public:
  /* cv::Rect uses easier than the RECT in <windows.h>! */
  cv::Rect window_rect; // cv::Rect of inner window
  cv::Rect map_rect;    // cv::Rect of map on inner window

  cv::Size map_size;   // current game map size
  cv::Size block_size; // current game block size

  cv::Mat screenshot;
  std::vector<int> map;

  Controller();

  // get current game map
  std::vector<int> get_map();
  // click specific block
  void click(int, int);

  void test();

 private:
  HWND handle_main;  // game process main handle
  HWND handle_inner; // inner window handle
  ANN classifier;

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