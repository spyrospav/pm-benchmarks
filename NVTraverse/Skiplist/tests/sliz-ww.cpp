#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

#include "../SkiplistIz.h"

__VERIFIER_persistent_storage(int levelmax = floor_log_2(4));

__VERIFIER_persistent_storage(static SkiplistIz* list);
__VERIFIER_persistent_storage(bool done = false);

void __VERIFIER_recovery_routine(void)
{

  if (done)
    assert(list->contains(1));
  return;

}

int main() {

  list = (SkiplistIz*)__VERIFIER_palloc(sizeof(SkiplistIz));
  new (list) SkiplistIz();

  list->insert(2, 10, 7);

  __VERIFIER_pbarrier();

  //assert(list->contains(2));

  list->insert(1, 10, 7);

  done = list->insert(4, 10, 7);

  return 0;

}
