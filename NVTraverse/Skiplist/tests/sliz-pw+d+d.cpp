#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

#include "../SkiplistIz.h"

static pthread_t threads[2];
static int param[2] = {0, 1};

__VERIFIER_persistent_storage(static SkiplistIz* list);
__VERIFIER_persistent_storage(bool t1 = false);
__VERIFIER_persistent_storage(bool t2 = false);

__VERIFIER_persistent_storage(int levelmax = floor_log_2(8));

void *thread1(void *param)
{

  list->remove(1);

  return NULL;

}

void *thread2(void *param)
{

  list->remove(2);

  return NULL;

}

void __VERIFIER_recovery_routine(void)
{

  assert(list->contains(3));
  if (t1) assert(!list->contains(1));
  if (t2) assert(!list->contains(2));

  return;

}

int main() {

  list = (SkiplistIz*)__VERIFIER_palloc(sizeof(SkiplistIz));
  new (list) SkiplistIz();

  list->insert(0, 10, 1);
  list->insert(1, 10, 1);
  list->insert(2, 10, 1);
  list->insert(3, 10, 1);

  __VERIFIER_pbarrier();

  pthread_create(&threads[0], NULL, thread1, &param[0]);
  pthread_create(&threads[1], NULL, thread2, &param[1]);

  pthread_join(threads[0], NULL);
  pthread_join(threads[1], NULL);

  return 0;

}
