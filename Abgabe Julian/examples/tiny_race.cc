#include <pthread.h>
#include <iostream>
#include <thread>
#include <chrono>

const int iter_count = 1;

long int Global;
short Global2;

void *Thread1(void *x)
{
  for (size_t i = 0; i < iter_count; i++)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    Global2 = 42;
    Global = 42;
    
  }
  return x;
}

void *Thread2(void *x)
{
  for (size_t i = 0; i < iter_count; i++)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    Global2 = 40;
    Global = 39;
    
  }
  return x;
}

int main()
{
  pthread_t t1, t2;
  pthread_create(&t1, NULL, Thread1, NULL);
  pthread_create(&t2, NULL, Thread2, NULL);

  for (size_t i = 0; i < iter_count; i++)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (Global == 42)
      std::cout << Global << std::endl;
    Global2 = 43;
    Global = 43;
  }
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  std::cout << Global << std::endl;
  return 0;
}