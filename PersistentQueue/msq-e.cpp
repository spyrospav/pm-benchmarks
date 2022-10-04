#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

#include "MSQueue.h"

static pthread_t threads[2];
static int param[2] = {0, 1};

__VERIFIER_persistent_storage(static MSQueue* queue);
__VERIFIER_persistent_storage(bool done);

void __VERIFIER_recovery_routine(void)
{

  __VERIFIER_assume(done);
  assert(!queue->isEmpty());

  return;

}

int main() {

  queue = (MSQueue*)__VERIFIER_palloc(sizeof(MSQueue));
  new (queue) MSQueue();

  __VERIFIER_pbarrier();

  done = queue->enq(2);

  int x = queue->getSize();
  assert(x == 1);

  return 0;

}
