#include <windows.h>

#include <opencv2/opencv.hpp>
#include <vector>

class Controller
{
 public:
  HWND handle_main;  // game process main handle
  HWND handle_inner; // inner window handle

  RECT window_size; // size of inner window

  std::pair<int, int> map_size; // game map size

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
  // analyze the screenshot, extract information to controller
  void analyze_screenshot();
};