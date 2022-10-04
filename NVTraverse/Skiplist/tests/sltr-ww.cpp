#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

#include "../SkiplistTraverse.h"

__VERIFIER_persistent_storage(int levelmax = floor_log_2(4));

__VERIFIER_persistent_storage(static SkiplistTraverse* list);
__VERIFIER_persistent_storage(bool done = false);

void __VERIFIER_recovery_routine(void)
{

  if (done)
    assert(list->contains(1));
  return;

}

int main() {

  list = (SkiplistTraverse*)__VERIFIER_palloc(sizeof(SkiplistTraverse));
  new (list) SkiplistTraverse();

  list->insert(2, 10, 7);

  __VERIFIER_pbarrier();

  list->insert(1, 10, 7);

  done = list->insert(4, 10, 7);

  return 0;

}
