#include "ann.hpp"
#include "controller.hpp"

int main()
{
  using namespace minesweeper_solver;

  Controller c = Controller();
  auto test = c.get_map();

  // c.click(0, 0);

  // Sleep(100);
  // for (int i = 0; i < 10 && GetForegroundWindow() == c.hm; ++i)
  // {
  //   cout << i << endl;
  //   c.click(i, i);
  //   Sleep(100);
  // }
}