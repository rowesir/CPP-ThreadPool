/*
 *    线程池任务队列 - 双向链表
 */
#ifndef _NJOBSNODE_H
#define _NJOBSNODE_H

struct NJobsNode {
  NJobsNode(int (*f)(void *), void *a, int *r)
      : func(f), arg(a), ret(r), prev(nullptr), next(nullptr) {}

  int (*func)(void *);  // 任务函数地址
  void *arg;            // 任务函数参数存放地址
  int *ret;             // 任务函数返回值存放地址
  NJobsNode *prev;      // 前一个节点
  NJobsNode *next;      // 下一节点
};

#endif
