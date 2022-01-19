#include <chrono>
#include <thread>

#include "ann.hpp"
#include "controller.hpp"

int main()
{
  using namespace minesweeper_solver;

  Controller c;
  while (!c.find_game())
  {
    int r = MessageBoxW(NULL, L"未能找到扫雷窗口。", L"错误", MB_RETRYCANCEL);
    if (r == IDCANCEL)
    {
      return 0;
    }
  }
  while (c.update_map())
  {
    auto map = c.map();
    for (int i = 0; i < map.rows; ++i)
    {
      for (int j = 0; j < map.cols; ++j)
      {
        int v = map.at<int>(i, j);
        char c;
        if (v >= 0 && v < 9)
        {
          printf(" %d", v);
        }
        else if (v == 9)
        {
          printf(" *");
        }
        else if (v == 10)
        {
          printf("  ");
        }
      }
      printf("\n");
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  // std::cout << c.map_rect() << std::endl;

  // auto map = c.map();
  // c.generate_train_data();

  // c.click(0, 0);

  // Sleep(100);
  // for (int i = 0; i < 10 && GetForegroundWindow() == c.hm; ++i)
  // {
  //   cout << i << endl;
  //   c.click(i, i);
  //   Sleep(100);
  // }
}