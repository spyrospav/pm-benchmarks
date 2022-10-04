#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

#include "../SkiplistOriginal.h"

__VERIFIER_persistent_storage(int levelmax = floor_log_2(8));

__VERIFIER_persistent_storage(SkiplistOriginal* list);
__VERIFIER_persistent_storage(bool done);

void __VERIFIER_recovery_routine(void)
{

  if (done)
    assert(list->contains(4));

  return;

}

int main() {

  list = (SkiplistOriginal*)__VERIFIER_palloc(sizeof(SkiplistOriginal));
  new (list) SkiplistOriginal();

  list->insert(1, 10, 1);
  done = false;

  __VERIFIER_pbarrier();

  assert(list->contains(1));

  done = list->insert(4, 10, 1);

  return 0;

}
