#include <windows.h>

#include <iostream>

using namespace std;
class Controller
{
 public:
  HWND hm;  // main handle
  HWND hiw; // inner window handle

  RECT rect;

  Controller();

  void func1();
  void click(int, int);
  void focus();
};
Controller::Controller()
{
  this->hm = FindWindow(NULL, "Minesweeper");
  this->hiw = FindWindowEx(this->hm, NULL, "Static", NULL);

  GetWindowRect(this->hiw, &this->rect);
}
void Controller::func1()
{
  while (1)
  {
    cout << "click!" << endl;
    PostMessage(this->hm, WM_RBUTTONDOWN, MK_LBUTTON, 0);
    PostMessage(this->hm, WM_RBUTTONUP, 0, 0);
    Sleep(1000);
  }
}
void Controller::click(int x, int y)
{
  SetCursorPos(this->rect.left + 39 + x * 18, this->rect.top + 39 + y * 18);
  mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
  mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}
void Controller::focus()
{
  // ShowWindow(this->main, SW_SHOW);
  SetForegroundWindow(this->hm);
}

int main()
{
  Controller c = Controller();
  cout << "handle: " << c.hm << " inner: " << c.hiw << endl;
  c.focus();

  // Sleep(100);
  // for (int i = 0; i < 10 && GetForegroundWindow() == c.hm; ++i)
  // {
  //   cout << i << endl;
  //   c.click(i, i);
  //   Sleep(100);
  // }
}