/*
 *    线程池工作队列 - 双向链表
 */
#ifndef _NWORKSNode_H
#define _NWORKSNode_H
#include <pthread.h>
class NManage;
struct NWorkNode {
  NWorkNode(pthread_t t, NManage *m)
      : tid(t), manage(m), prev(nullptr), next(nullptr), termination(0) {}
  pthread_t tid;     // 线程ID
  NManage *manage;   // 管理类
  NWorkNode *prev;   // 前一节点
  NWorkNode *next;   // 下一节点
  bool termination;  // 终止标志
};

#endif
