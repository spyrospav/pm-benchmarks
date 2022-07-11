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

#define CAS __sync_bool_compare_and_swap

extern "C"{
 void __VERIFIER_clflush(void*);
}

inline void FLUSH(void *p)
{
#ifdef PWB_IS_CLFLUSH
  __VERIFIER_clflush(&p);
  //asm volatile ("clflush (%0)" :: "r"(p));
#elif PWB_IS_CLFLUSHOPT
  asm volatile(".byte 0x66; clflush %0" : "+m" (*(volatile char *)(p)));
#else
#error "You must define what PWB is. Choose PWB_IS_CLFLUSH if you don't know what your CPU is capable of"
#endif
}

inline void FLUSH(volatile void *p)
{
#ifdef PWB_IS_CLFLUSH
  __VERIFIER_clflush(&p);
  // asm volatile ("clflush (%0)" :: "r"(p));
#elif PWB_IS_CLFLUSHOPT
  // __VERIFIER_clflushopt(p);
  asm volatile(".byte 0x66; clflush %0" : "+m" (*(volatile char *)(p)));
#else
#error "You must define what PWB is. Choose PWB_IS_CLFLUSH if you don't know what your CPU is capable of"
#endif
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
  // std::atomic_thread_fence(std::memory_order_seq_cst);
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

void BARRIER(void* p){
	FLUSH(p);
	MFENCE();
}

void BARRIER(volatile void* p){
	FLUSH(p);
	MFENCE();
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
