#include <unistd.h>
#include <iostream>
#include "NManage.h"
using namespace std;

int f(void *arg) {
  int *p = (int *)arg;
  cout << "Hello World" << endl;
  return (*p) * 2 - 1;
}

int main() {
  int ret[100];
  int arg[100];
  for (int i = 0; i < 100; ++i) arg[i] = i;

  ThreadPool test(1);
  cout << "size :" << test.GetThreadPoolSize() << endl;
  test.ChangeSize(10);
  cout << "size :" << test.GetThreadPoolSize() << endl;
  getchar();
  for (int i = 0; i < 100; ++i) test.AddJob(f, &arg[i], &ret[i]);
  sleep(2);
  cout << "Done!" << endl;
  for (const auto &c : ret) cout << c << endl;
}
