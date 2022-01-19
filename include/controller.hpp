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

static const std::string GENERATED_TRAIN_DATA = "../data/generate";
static const int SUPERVISE_INTERVAL = 1000;

class Controller
{
 public:
  Controller();
  ~Controller();

  cv::Rect const map_rect() { return this->_map_rect; }
  cv::Size const map_size() { return this->_map_size; }
  cv::Size const block_size() { return this->_block_size; }
  bool const game_available() { return this->_game_available; }

  bool bind_game();

  // get current game map
  std::vector<int> map();
  // get previous game map
  std::vector<int> const previous_map() { return this->_map; }

  // click specific block
  void click(int, int);

  void generate_train_data();

  void test();

 protected:
  bool _game_available;
  std::promise<void> supervisor_running;
  std::thread supervisor;

  HWND handle_main;  // game process main handle
  HWND handle_inner; // inner window handle
  ANN classifier;

  /* cv::Rect uses easier than the RECT in <windows.h>! */

  /* game info */
  std::mutex info_lock;
  cv::Rect _map_rect;   // cv::Rect of map on inner window
  cv::Size _map_size;   // current game map size(row and column)
  cv::Size _block_size; // current game block size(width and height)

  /* game map */
  std::mutex map_lock;
  std::vector<int> _map;

  // get window rect
  cv::Rect window_rect();
  // put the game process at top and focus
  void focus();

  // get screen by controller handle
  cv::Mat take_screenshot();
  // analyze the screenshot, get dimension of the game window
  void analyze_game_info(const cv::Mat& screenshot);
  // analyze the screenshot, get map of the game window
  void analyze_game_map(const cv::Mat& screenshot);
};
} // namespace minesweeper_solver
#endif