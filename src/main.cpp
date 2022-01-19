#include "ann.hpp"
#include "controller.hpp"

int main()
{
  using namespace minesweeper_solver;

  Controller c;
  while (!c.bind_game())
  {
    int r = MessageBoxW(NULL, L"未能找到扫雷窗口。", L"错误", MB_RETRYCANCEL);
    if (r == IDCANCEL)
    {
      return 0;
    }
  }

  std::cout << c.map_rect() << std::endl;

  // auto map = c.map();
  // c.generate_train_data();

  // for (int i = 0; i < c.map_size().height; ++i)
  // {
  //   for (int j = 0; j < c.map_size().width; ++j)
  //   {
  //     int v = map[i * c.map_size().width + j];
  //     char c;
  //     if (v >= 0 && v < 9)
  //     {
  //       printf(" %d", v);
  //     }
  //     else if (v == 9)
  //     {
  //       printf(" *");
  //     }
  //     else if (v == 10)
  //     {
  //       printf("  ");
  //     }
  //   }
  //   printf("\n");
  // }

  // c.click(0, 0);

  // Sleep(100);
  // for (int i = 0; i < 10 && GetForegroundWindow() == c.hm; ++i)
  // {
  //   cout << i << endl;
  //   c.click(i, i);
  //   Sleep(100);
  // }
}