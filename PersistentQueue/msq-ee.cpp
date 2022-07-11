#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

int __thread tid;

#include "MSQueue.h"

static pthread_t threads[2];
static int param[2] = {0, 1};

__VERIFIER_persistent_storage(static MSQueue* queue);
__VERIFIER_persistent_storage(bool done);

void *thread1(void *param)
{

  return NULL;

}

void *thread2(void *param)
{

  return NULL;

}

void __VERIFIER_recovery_routine(void)
{

  if (done) assert(!queue->isEmpty());
  // if (done) {
  //   for (int i = 0; i < 2; i++) {
  //     assert(!queue->isEmpty());
  //     int x = queue->deq();
  //   }
  // }
  // assert(queue->isEmpty());

  return;

}

int main() {

  queue = (MSQueue*)__VERIFIER_palloc(sizeof(MSQueue));
  new (queue) MSQueue();

  done = false;

  __VERIFIER_pbarrier();

  queue->enq(1);
  queue->enq(2);

  done = true;

  return 0;

}
