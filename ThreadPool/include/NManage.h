/*
 *    线程池管理类 API抛出对应错误
 */
#ifndef _NMANAGE_H
#define _NMANAGE_H
#include "NJobsNode.h"
#include "NWorksNode.h"

extern void *Thread_fun(void *);  // 线程执行函数

class NManage {
  friend void *Thread_fun(void *);    // 线程执行函数为友元
  NManage(const NManage &) = delete;  // 拷贝控制删除
  void operator=(const NManage &) = delete;

 public:
  /*
   * 构造函数 初始化线程池
   * 参数为执行线程个数 > 0
   * @- int number
   */
  NManage(int number);

  /*
   * 添加任务到任务队列
   * 参数为任务执行函数及参数,返回值存放地址
   * @1- int (func)(void *arg)
   * @2- void *arg
   * @3- int *ret
   * 成功返回0, 错误返回 -1
   */
  int AddJob(int(func)(void *), void *arg, int *ret);

  /*
   *  线程池容量动态改变
   *  参数为期望线程数量 > 0 ,大于当前值扩容,小于当前值缩容
   *  @- int size
   *  成功返回0, 错误返回 -1
   */
  int ChangeSize(int size);

  /*
   *  获取线程池当前线程数量
   *  返回数量
   */
  int GetThreadPoolSize();

  /*
   * 销毁线程池中所有任务及线程
   */
  void Destroy();
  virtual ~NManage();

 private:
  int count;               // 线程数量
  NJobsNode *jobs;         // 任务队列
  NWorkNode *works;        // 执行队列
  pthread_cond_t cond;     // 条件变量
  pthread_mutex_t Jmutex;  // 任务队列互斥锁
  pthread_mutex_t Wmutex;  // 工作队列互斥锁

  // 私有接口函数 删除队列头节点
  void rm_jobs(NJobsNode *node);
  void rm_works(NWorkNode *node);

  // 私有接口函数 添加队列头节点
  void add_jobs(NJobsNode *node);
  void add_works(NWorkNode *node);
};

using ThreadPool = NManage;
#endif
