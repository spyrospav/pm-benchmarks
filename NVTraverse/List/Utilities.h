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

#define INT_MIN -1
#define INT_MAX 100

#define CAS __sync_bool_compare_and_swap

extern "C"{
 void __VERIFIER_clflush(void*);
}

class Node{
public:
  int key;
  int value;
  Node* next;

  Node(int k, int val, Node* n) : key(k), value(val), next(n) {}

  Node() {
    key = INT_MIN;
    value = INT_MIN;
    next = NULL;
  }

  void set(int k, int val, Node* n) {
    key = k;
    value = val;
    next = n;
  }

  Node* getNext() {
    return next;
  }

  Node* getNextF();

  Node* getNextB();

  bool CAS_next(Node* exp, Node* n) {
    Node* old = next;
    if(exp != old) return false;
    bool ret = CAS(&next, old, n);
    return ret;
  }

  bool CAS_nextF(Node* exp, Node* n);

  bool CAS_nextB(Node* exp, Node* n);

};

void FLUSH(Node *n)
{
  #ifdef BMFK
  __VERIFIER_clflush(&n);
  #else
  __VERIFIER_clflush(&(n->key));
  __VERIFIER_clflush(&(n->value));
  __VERIFIER_clflush(&(n->next));
  #endif
}

Node* Node::getNextF() {
  Node* n = next;
  FLUSH(next);
  return n;
}

bool Node::CAS_nextF(Node* exp, Node* n) {
  Node* old = next;
  if(exp != old) {
    FLUSH(next);
    return false;
  }
  bool ret = CAS(&next, old, n);
  FLUSH(next);
  return ret;
}

inline void MFENCE()
{
  // Which of the following should we use?
  // NOTE: __sync_synchronize() creates a full memory barrier a.k.a. mfence
  // std::atomic_thread_fence(std::memory_order_seq_cst);
  __sync_synchronize();
}

inline void SFENCE()
{
  // __VERIFIER_sfence();
  // asm volatile ("sfence" ::: "memory");
}

inline void FENCE()
{

#ifdef PWB_IS_CLFLUSH
  MFENCE();
#elif PWB_IS_CLFLUSHOPT
  SFENCE();
#else
#error "You must define what PWB is. Choose PWB_IS_CLFLUSH if you don't know what your CPU is capable of"
#endif
}

void BARRIER(Node* n){
	FLUSH(n);
	MFENCE();
}

bool Node::CAS_nextB(Node* exp, Node* n) {
  Node* old = next;
  if (exp != old) {
    BARRIER(next);
    return false;
  }
  bool ret = CAS(&next, exp, n);
  BARRIER(next);
  return ret;
}

Node* Node::getNextB() {
  Node* n = next;
  BARRIER(next);
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
