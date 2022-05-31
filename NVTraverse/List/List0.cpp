#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

int __thread tid;

#include "ListOriginal.h"

static pthread_t threads[2];
static int param[2] = {0, 1};

__VERIFIER_persistent_storage(static ListOriginal* list);
__VERIFIER_persistent_storage(bool res2);

void *thread1(void *param)
{

  list->insert(1, 10);

  return NULL;

}

void *thread2(void *param)
{

  res2 = list->insert(2, 10);

  return NULL;

}

void __VERIFIER_recovery_routine(void)
{

  if (list->contains(3))
    assert(list->contains(0));

  return;

}

int main() {

  list = (ListOriginal*)__VERIFIER_palloc(sizeof(ListOriginal));
  new (list) ListOriginal();

  __VERIFIER_pbarrier();

  list->insert(0,10);
  list->insert(3,10);

  return 0;

}
