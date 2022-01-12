#include <algorithm>
#include <iostream>
#include <vector>

namespace minesweeper_solver
{
namespace util
{
  int get_majority_number(std::vector<int> list, int total_count)
  {
    int* bucket = new int[total_count]();
    int max = 0;
    for (int i = 0; i < list.size(); ++i)
    {
      ++bucket[list[i]];
    }
    for (int i = 0; i < total_count; ++i)
    {
      if (bucket[i] > bucket[max])
      {
        max = i;
      }
    }
    delete[] bucket;
    return max;
  }
} // namespace util
} // namespace minesweeper_solver