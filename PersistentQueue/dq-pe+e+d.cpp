#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

#include "DurableQueue.h"

static pthread_t threads[3];
static int param[3] = {0, 1, 2};

__VERIFIER_persistent_storage(DurableQueue* queue);
__VERIFIER_persistent_storage(bool done1 = false);
__VERIFIER_persistent_storage(bool done2 = false);

void *thread1(void *param)
{

  queue->enq(2);
  done1 = true;

  return NULL;

}

void *thread2(void *param)
{

  queue->deq(2);
  done2 = true;

  return NULL;

}

void __VERIFIER_recovery_routine(void)
{

  __VERIFIER_assume(done1 && done2);
  assert(queue->removedValues[2] == 1);
  int x = queue->getSize(true);
  assert(x == 1);

  return;

}

int main() {

  queue = (DurableQueue*)__VERIFIER_palloc(sizeof(DurableQueue));
  new (queue) DurableQueue();

  queue->enq(1);

  __VERIFIER_pbarrier();

  pthread_create(&threads[0], NULL, thread1, &param[0]);
  pthread_create(&threads[1], NULL, thread2, &param[1]);

  pthread_join(threads[0], NULL);
  pthread_join(threads[1], NULL);

  assert(queue->getSize() == 1);

  return 0;

}
