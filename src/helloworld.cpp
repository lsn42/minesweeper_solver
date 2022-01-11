#include <cstdio>
#include <thread>
main()
{
  for (int i = 0; i < 10; ++i)
  {
    std::thread([i]() { printf("helloworld%d\n", i); }).join();
  }
}