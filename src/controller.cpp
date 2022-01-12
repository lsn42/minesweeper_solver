#include "controller.hpp"

#include <windows.h>

#include <cstdio>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

Controller::Controller():
  handle_main(FindWindow(NULL, "Minesweeper")),
  handle_inner(FindWindowEx(this->handle_main, NULL, "Static", NULL))
{
  update_window_info();
}

void Controller::update_window_info()
{
  GetWindowRect(this->handle_inner, &this->window_size);
}

void Controller::focus()
{
  SetForegroundWindow(this->handle_main);
}

cv::Mat Controller::get_screenshot()
{
  this->update_window_info();

  int width = this->window_size.right - this->window_size.left;
  int height = this->window_size.bottom - this->window_size.top;

  LPVOID img = new char[width * height * 4];

  HDC window_DC = GetWindowDC(this->handle_inner);
  HDC memory_DC = CreateCompatibleDC(window_DC);

  HBITMAP bitmap = CreateCompatibleBitmap(window_DC, width, height);
  SelectObject(memory_DC, bitmap);

  BitBlt(memory_DC, 0, 0, width, height, window_DC, 0, 0, SRCCOPY);
  GetBitmapBits(bitmap, width * height * 4, img);

  cv::Mat screenshot = cv::Mat(height, width, CV_8UC4, img);
  cv::imwrite("screenshot.jpg", screenshot);

  DeleteDC(window_DC);
  DeleteDC(memory_DC);
  DeleteObject(img);

  return screenshot;
}

std::vector<byte> Controller::get_map()
{
  get_screenshot();
  return std::vector<byte>();
}

void Controller::click(int x, int y)
{
  SetCursorPos(
    this->window_size.left + 39 + x * 18, this->window_size.top + 39 + y * 18);
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

int main()
{
  Controller c = Controller();

  auto test = c.get_map();
  // c.focus();

  // Sleep(100);
  // for (int i = 0; i < 10 && GetForegroundWindow() == c.hm; ++i)
  // {
  //   cout << i << endl;
  //   c.click(i, i);
  //   Sleep(100);
  // }
}