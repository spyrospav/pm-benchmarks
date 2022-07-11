#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

int __thread tid;

#include "DurableQueue.h"

static pthread_t threads[2];
static int param[2] = {0, 1};

__VERIFIER_persistent_storage(DurableQueue* queue);
__VERIFIER_persistent_storage(bool done1);
__VERIFIER_persistent_storage(bool done2);

void *thread1(void *param)
{

  done1 = queue->enq(3);

  return NULL;

}

void *thread2(void *param)
{

  done2 = queue->enq(2);

  return NULL;

}

void __VERIFIER_recovery_routine(void)
{

  if (done1) assert(!queue->isEmpty());

  return;

}

int main() {

  queue = (DurableQueue*)__VERIFIER_palloc(sizeof(DurableQueue));
  new (queue) DurableQueue();

  done1 = false;
  done2 = false;

  __VERIFIER_pbarrier();

  done1 = queue->enq(2);
  // done2 = queue->enq(1);

  // pthread_create(&threads[0], NULL, thread1, &param[0]);
  // pthread_create(&threads[1], NULL, thread2, &param[1]);
  //
  // pthread_join(threads[0], NULL);
  // pthread_join(threads[1], NULL);

  return 0;

}
