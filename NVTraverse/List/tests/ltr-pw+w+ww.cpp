#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

int __thread tid;

#include "../ListTraverse.h"

static pthread_t threads[2];
static int param[2] = {0, 1};

__VERIFIER_persistent_storage(static ListTraverse* list);

void *thread1(void *param)
{

  list->insert(1, 10);

  return NULL;

}

void *thread2(void *param)
{

  list->insert(2, 10);
  list->insert(4, 10);

  return NULL;

}

void __VERIFIER_recovery_routine(void)
{

  if (list->contains(4))
    assert(list->contains(2));

  return;

}

int main() {

  list = (ListTraverse*)__VERIFIER_palloc(sizeof(ListTraverse));
  new (list) ListTraverse();

  list->insert(0,10);
  list->insert(3,10);

  __VERIFIER_pbarrier();

  pthread_create(&threads[0], NULL, thread1, &param[0]);
  pthread_create(&threads[1], NULL, thread2, &param[1]);

  pthread_join(threads[0], NULL);
  pthread_join(threads[1], NULL);

  return 0;

}
