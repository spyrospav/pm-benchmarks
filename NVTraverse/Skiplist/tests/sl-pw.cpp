#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

#include "../SkiplistOriginal2.h"

int levelmax = floor_log_2(4);

__VERIFIER_persistent_storage(SkiplistOriginal* list);

void __VERIFIER_recovery_routine(void)
{

  assert(list->contains(1));

  return;

}

int main() {

  list = (SkiplistOriginal*)__VERIFIER_palloc(sizeof(SkiplistOriginal));
  new (list) SkiplistOriginal();

  list->insert(1, 10, 1);

  __VERIFIER_pbarrier();

  // list->insert(4, 10, 1);

  return 0;

}
