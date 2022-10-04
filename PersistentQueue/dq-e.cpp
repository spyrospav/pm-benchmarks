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
__VERIFIER_persistent_storage(bool done = false);

void __VERIFIER_recovery_routine(void)
{

  __VERIFIER_assume(done);
  assert(queue->getSize() == 1);

  return;

}

int main() {

  queue = (DurableQueue*)__VERIFIER_palloc(sizeof(DurableQueue));
  new (queue) DurableQueue();

  __VERIFIER_pbarrier();

  done = queue->enq(1);

  return 0;

}
