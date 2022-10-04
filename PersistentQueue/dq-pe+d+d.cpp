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
__VERIFIER_persistent_storage(bool done1 = false);
__VERIFIER_persistent_storage(bool done2 = false);

void *thread1(void *param)
{

  queue->deq(1);

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

  for (int i = 0; i < 3; i++)
    printf("%d\n", queue->removedValues[i]);

  int v1 = queue->removedValues[1];
  int v2 = queue->removedValues[2];

  assert(v1 == 1 || v1 == 2);
  assert(v2 == 1 || v2 == 2);

  if (v1 == 1) assert(v2 == 2);
  if (v1 == 2) assert(v2 == 1);

  return;

}

int main() {

  queue = (DurableQueue*)__VERIFIER_palloc(sizeof(DurableQueue));
  new (queue) DurableQueue();

  queue->enq(1);
  queue->enq(2);

  __VERIFIER_pbarrier();

  pthread_create(&threads[0], NULL, thread1, &param[0]);
  pthread_create(&threads[1], NULL, thread2, &param[1]);

  pthread_join(threads[0], NULL);
  pthread_join(threads[1], NULL);

  return 0;

}
