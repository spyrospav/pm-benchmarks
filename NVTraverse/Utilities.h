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
  //asm volatile ("clflush (%0)" :: "r"(p));
#elif PWB_IS_CLFLUSHOPT
  asm volatile(".byte 0x66; clflush %0" : "+m" (*(volatile char *)(p)));
#else
#error "You must define what PWB is. Choose PWB_IS_CLFLUSH if you don't know what your CPU is capable of"
#endif
}

inline void MFENCE()
{
  std::atomic_thread_fence(std::memory_order_seq_cst);
}

inline void SFENCE()
{
  //asm volatile ("sfence" ::: "memory");
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

void BARRIER(void* p){
	FLUSH(p);
	MFENCE();
}

void BARRIER(volatile void* p){
	FLUSH(p);
	MFENCE();
}

void OPT_BARRIER(void* p){
	FLUSH(p);
	//SFENCE();
}

void OPT_BARRIER(volatile void* p){
	FLUSH(p);
	//SFENCE();
}

bool BARRIERM(void* p){
	long pLong = (long)p;
	if((pLong & 2) && !(pLong & 1)){
		return false;
	}
  else {
		FLUSH(p);
		SFENCE();
		return true;
	}
}

bool BARRIERM(volatile void* p){
	long pLong = (long)p;
	if((pLong & 2) && !(pLong & 1)){
		return false;
	}
  else {
		FLUSH(p);
		SFENCE();
		return true;
	}
}

bool OPT_BARRIERM(void* p){
	long pLong = (long)p;
	if((pLong  & 2) && !(pLong & 1)){
		return false;
	}
  else {
		FLUSH(p);
		SFENCE();
		return true;
	}
}

bool OPT_BARRIERM(volatile void* p){
	long pLong = (long)p;
	if((pLong & 2) && !(pLong & 1)){
		return false;
	}
  else {
		FLUSH(p);
		SFENCE();
		return true;
	}
}

bool FLUSHM(void* p){
  long pLong = (long)p;
  if((pLong & 2) && !(pLong & 1)){
    return false;
  }
  else {
    FLUSH(p);
    return true;
  }
}

bool FLUSHM(volatile void* p){
  long pLong = (long)p;
  if((pLong & 2) && !(pLong & 1)){
    return false;
  }
  else {
    FLUSH(p);
    return true;
  }
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
