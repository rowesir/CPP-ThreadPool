/*
 *   线程池管理类 API接口定义
 */

#include "NManage.h"

#include <stdexcept>         // 标准异常类
static pthread_attr_t attr;  // 线程属性(分离态)

void *Thread_fun(void *arg) {          // 线程执行函数
  NWorkNode *work = (NWorkNode *)arg;  // 取得工作节点
  NManage *manag = work->manage;       // 取得管理类指针
  while (1) {
    pthread_mutex_lock(&manag->Jmutex);  // 上锁检查任务队列
    while (!manag->jobs && !work->termination)
      pthread_cond_wait(&manag->cond, &manag->Jmutex);
    //等待唤醒,若任务队列仍然为空且标志未置位则此次唤醒是销毁线程唤醒,继续等待任务到来唤醒
    if (work->termination)  // 查看自身销毁标志是否置位
    {
      manag->rm_works(work);                 // 删除本工作节点
      pthread_mutex_unlock(&manag->Jmutex);  // 解锁
      pthread_cond_broadcast(&manag->cond);
      // 若线程需要销毁并竞争得到任务则让出当前任务,唤醒其他线程
      pthread_exit(0);
    }
    int (*f)(void *) = manag->jobs->func;  // 取得任务函数地址
    void *arg = manag->jobs->arg;          // 取得任务函数参数地址
    int *ret = manag->jobs->ret;           // 取得返回值(int)地址
    manag->rm_jobs(manag->jobs);  // 已获取任务,任务队列删除该节点
    pthread_mutex_unlock(&manag->Jmutex);  // 解锁
    if (ret)
      *ret = (int)f(arg);  // 若返回地址不为空则将函数运行结果存入该地址中
    else
      f(arg);
  }
  return (void *)0;
}

/* 线程池初始化 */
NManage::NManage(int number) : count(0), jobs(nullptr), works(nullptr) {
  if (number < 1)
    throw std::invalid_argument(
        "线程池初始化参数必须大于0!");  // 初始线程数小于1则抛出异常
  Wmutex = PTHREAD_MUTEX_INITIALIZER;
  Jmutex = PTHREAD_MUTEX_INITIALIZER;  // 初始化两把互斥锁
  pthread_cond_init(&cond, nullptr);   // 初始化条件变量
  pthread_t tid;                       // 线程tid
  pthread_attr_init(&attr);            // 初始化线程属性结构
  pthread_attr_setdetachstate(&attr,
                              PTHREAD_CREATE_DETACHED);  // 设置线程状态为分离态

  for (int i = 0; i < number; - ++i) {  // 循环创建线程并加入工作队列中
    NWorkNode *node = new NWorkNode(tid, this);
    // 创建工作队列节点,若new错误则由new抛出bad_alloc异常
    if (pthread_create(&tid, &attr, Thread_fun, node) != 0) {  // 以分离态启动
      delete node;  // 若线程创建失败则释放工作节点并抛出异常
      throw std::runtime_error("线程池创建线程失败!");
    }
    add_works(node);  // 添加到工作队列中
  }
}

/* 添加任务 */
int NManage::AddJob(int(func)(void *), void *arg, int *ret) {
  if (!func) return -1;  // 若地址为空则返回-1
  NJobsNode *node =
      new NJobsNode(func, arg, ret);  // 若new错误则又new抛出bad_alloc异常
  add_jobs(node);                     // 添加任务节点到任务队列
  return 0;
}

/* 动态扩容 */
int NManage::ChangeSize(int size) {
  if (size < 0) return -1;      // 若更改线程池大小 小于0 则返回-1
  pthread_mutex_lock(&Wmutex);  // 对工作队列上锁
  if (size <= count) {          // 缩容
    NWorkNode *next = works;
    for (int i = count - size; i != 0; --i) {
      if (next) {
        next->termination = true;  // 销毁标记置位
        next = next->next;
      } else
        break;
    }
    pthread_mutex_unlock(&Wmutex);  // 对工作队列解锁
    pthread_cond_broadcast(&cond);  // 唤醒所有线程检查自身销毁标志
  } else if (size > count) {        // 扩容
    pthread_mutex_unlock(&Wmutex);  // 对之前已上锁的Wmutex解锁
    pthread_t tid;
    for (int i = size - count; i != 0; --i) {
      NWorkNode *node =
          new NWorkNode(tid, this);  // 创建工作队列节点,由new抛出异常
      if (pthread_create(&tid, &attr, Thread_fun, node) != 0) {
        delete node;
        return -1;  // 若线程创建失败则释放已创建的工作节点并返回-1
      }
      add_works(node);  // 添加到工作队列中
    }
  }
  return 0;
}

int NManage::GetThreadPoolSize() { return count; }  // 返回当前线程池中线程数量

void NManage::Destroy() {
  ChangeSize(0);
}  // 对每一个线程(工作队列)的销毁标志置位

NManage::~NManage() {
  Destroy();                    // 销毁工作队列
  pthread_mutex_lock(&Jmutex);  // 对任务队列上锁
  for (NJobsNode *p = jobs; p != nullptr; p = p->next)
    delete p;                     // 遍历任务队列删除并释放节点
  jobs = nullptr;                 //  任务队列为空
  pthread_mutex_unlock(&Jmutex);  //  释放任务队列锁
  while (1) {                     // 等待所有线程销毁
    pthread_mutex_lock(&Wmutex);
    if (!works) {  // 若队列已空则解锁并跳出循环
      pthread_mutex_unlock(&Wmutex);
      break;
    }
    pthread_mutex_unlock(&Wmutex);  // 释放锁继续循环等待
  }
  pthread_cond_destroy(&cond);  //  反初始化条件变量
}

void NManage::rm_jobs(
    NJobsNode *node) {  // 删除任务节点 函数进入前 工作队列锁已上锁
  if (node->next) node->next->prev = node->prev;
  if (node->prev)
    node->prev->next = node->next;
  else
    jobs = node->next;
  delete node;
}
void NManage::rm_works(NWorkNode *node) {  // 删除工作节点
  pthread_mutex_lock(&Wmutex);
  if (node->next) node->next->prev = node->prev;
  if (node->prev)
    node->prev->next = node->next;
  else
    works = node->next;
  delete node;
  --count;  // 线程池中线程数量-1
  pthread_mutex_unlock(&Wmutex);
}

void NManage::add_jobs(NJobsNode *node) {  // 添加任务节点到队头
  pthread_mutex_lock(&Jmutex);
  if (jobs) {
    jobs->prev = node;
    node->next = jobs;
  }
  jobs = node;
  pthread_mutex_unlock(&Jmutex);
  pthread_cond_broadcast(&cond);  // 换新所有线程竞争任务
}

void NManage::add_works(NWorkNode *node) {  // 添加工作节点到队头
  pthread_mutex_lock(&Wmutex);
  if (works) {
    works->prev = node;
    node->next = works;
  }
  works = node;
  ++count;  // 线程数量+1
  pthread_mutex_unlock(&Wmutex);
}
