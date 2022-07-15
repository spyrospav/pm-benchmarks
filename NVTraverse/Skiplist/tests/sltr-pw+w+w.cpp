#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

#include "../SkiplistTraverse.h"

static pthread_t threads[2];
static int param[2] = {0, 1};

__VERIFIER_persistent_storage(static SkiplistTraverse* list);
__VERIFIER_persistent_storage(bool res2);

__VERIFIER_persistent_storage(int levelmax = floor_log_2(4));

void *thread1(void *param)
{

  list->insert(1, 10, 1);

  return NULL;

}

void *thread2(void *param)
{

  res2 = list->insert(2, 10, 1);

  return NULL;

}

void __VERIFIER_recovery_routine(void)
{

  if (res2)
    assert(list->contains(2));

  return;

}

int main() {

  list = (SkiplistTraverse*)__VERIFIER_palloc(sizeof(SkiplistTraverse));
  new (list) SkiplistTraverse();

  list->insert(0, 10, 1);
  list->insert(3, 10, 1);

  res2 = false;

  __VERIFIER_pbarrier();

  pthread_create(&threads[0], NULL, thread1, &param[0]);
  pthread_create(&threads[1], NULL, thread2, &param[1]);

  pthread_join(threads[0], NULL);
  pthread_join(threads[1], NULL);

  return 0;

}
