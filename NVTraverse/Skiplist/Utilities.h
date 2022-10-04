/*
 * Utilities.h
 *
 *  Created on: Jul 14, 2016
 *      Author: michal
 *  Modified on: May 27, 2022
 *      by: spyrospav
 */

#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <stddef.h>
#include <genmc.h>
#include <atomic>

#define INT_MIN -2147483000
#define INT_MAX 2147483000
#define FRASER_MAX_MAX_LEVEL 3 /* covers up to 2^64 elements */

#define CAS __sync_bool_compare_and_swap

extern int levelmax;

extern "C"{
 void __VERIFIER_clflush(void*);
}

int get_rand_level(int seed) {
  int level = 1;
  for (int i = 0; i < levelmax - 1; i++) {
    if ((rand_r_32((unsigned int *)&seed) % 101) < 50)
      level++;
    else
      break;
  }
  return level;
}

class Node {
public:
  int key;
  int val;
  unsigned char toplevel;
  Node* next[FRASER_MAX_MAX_LEVEL];

  Node() {
    key = INT_MIN;
    val = INT_MIN;
    for (int i = 0; i < levelmax; i++) {
      next[i] = nullptr;
    }
    toplevel = get_rand_level(1);
  }

  Node(int k, int v, int topl) {
    key = k;
    val = v;
    toplevel = topl;
  }

  Node(int k, int v, Node* n, int topl) {
    key = k;
    val = v;
    toplevel = topl;
    for (int i = 0; i < levelmax; i++)
    {
      next[i] = n;
    }
  }

  void setF(int k, int v, Node* n, int topl);

  void set(int k, int v, Node* n, int topl) {
    key = k;
    val = v;
    toplevel = topl;
    for (int i = 0; i < levelmax; i++) {
      next[i] = n;
    }
  }

  bool CASNextF(Node* exp, Node* n, int i);

  bool CASNext(Node* exp, Node* n, int i) {
    Node* old = next[i];
    if (exp != old) {
      return false;
    }
    bool ret = CAS(&next[i], exp, n);
    return ret;
  }

  Node* getNextF(int i);

  Node* getNext(int i) {
    return next[i];
  }

};


void FLUSH(Node *p)
{
  __VERIFIER_clflush(&(p->key));
  __VERIFIER_clflush(&(p->val));
  __VERIFIER_clflush(&(p->toplevel));
  for (int i = 0; i < levelmax; i++) {
    __VERIFIER_clflush(&(p->next[i]));
  }

}


inline void MFENCE()
{
  __sync_synchronize();
}

inline void SFENCE()
{
  // std::atomic_thread_fence(std::memory_order_seq_cst);
  // asm volatile ("sfence" ::: "memory");
}

int floor_log_2(unsigned int n)
{
  int pos = 0;
  if (n >= 1 << 16)
  {
    n >>= 16;
    pos += 16;
  }
  if (n >= 1 << 8)
  {
    n >>= 8;
    pos += 8;
  }
  if (n >= 1 << 4)
  {
    n >>= 4;
    pos += 4;
  }
  if (n >= 1 << 2)
  {
    n >>= 2;
    pos += 2;
  }
  if (n >= 1 << 1)
  {
    pos += 1;
  }

  return ((n == 0) ? (-1) : pos);

}

void BARRIER(Node* p){
	FLUSH(p);
	MFENCE();
}

void Node::setF(int k, int v, Node* n, int topl) {
  key = k;
  val = v;
  toplevel = topl;
  for (int i = 0; i < levelmax; i++) {
    next[i] = n;
  }
  FLUSH(this);
}

bool Node::CASNextF(Node* exp, Node* n, int i) {
  Node* old = next[i];
  if (exp != old) {
    __VERIFIER_clflush(&next[i]);
    MFENCE();
    return false;
  }
  bool ret = CAS(&next[i], exp, n);
  __VERIFIER_clflush(&next[i]);
  MFENCE();
  return ret;
}

Node* Node::getNextF(int i) {
  Node* n = next[i];
  if (n) {
    __VERIFIER_clflush(&next[i]);
    MFENCE();
  }
  return n;
}

/*
 *  Usefull functions for lock-free data structures
 */

static inline bool isMarked(void *ptr)
{
  auto ptrLong = (long long)(ptr);
  return ((ptrLong & 1) == 1);
}

static inline void *getCleanReference(void *ptr)
{
  auto ptrLong = (long long)(ptr);
  ptrLong &= ~1;
  return (void *)(ptrLong);
}

static inline void *getMarkedReference(void *ptr)
{
  auto ptrLong = (long long)(ptr);
  ptrLong |= 1;
  return (void *)(ptrLong);
}

#endif /* UTILITIES_H_ */
