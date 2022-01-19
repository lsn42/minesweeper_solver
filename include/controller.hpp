#ifndef MINESWEEPER_SOLVER_CONTROLLER_HPP
#define MINESWEEPER_SOLVER_CONTROLLER_HPP
#include <windows.h>

#include <future>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <thread>
#include <vector>

#include "ann.hpp"

namespace minesweeper_solver
{

static const std::string GENERATED_TRAIN_DATA = "data/generate";
static const int SUPERVISE_INTERVAL = 1000;

class Controller
{
 public:
  Controller() = default;
  ~Controller() = default;

  bool find_game();

  bool update_map();

  // get current game map
  cv::Mat const map() { return this->_map; }

  // click specific block
  void click(int, int);

  void generate_train_data();

  void test();

 protected:
  HWND handle_main;  // game process main handle
  HWND handle_inner; // inner window handle
  ANN classifier;

  /* cv::Rect uses easier than the RECT in <windows.h>! */
  cv::Rect _map_rect;   // cv::Rect of map on inner window
  cv::Size _block_size; // current game block size(width and height)
  cv::Mat _map;

  // get window rect
  cv::Rect window_rect();
  // put the game process at top and focus
  void focus();
  // get screen by controller handle
  cv::Mat take_screenshot();
  // analyze the screenshot to get game info and map
  std::pair<std::pair<cv::Rect, cv::Size>, cv::Mat> analyze(
    const cv::Mat& screenshot);
};
} // namespace minesweeper_solver
#endif