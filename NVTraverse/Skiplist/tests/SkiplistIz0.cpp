#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

#include "../SkiplistIz.h"

int levelmax = floor_log_2(4);

__VERIFIER_persistent_storage(static SkiplistIz* list);

void __VERIFIER_recovery_routine(void)
{

  if (list->contains(4))
    assert(list->contains(1));

  return;

}

int main() {

  list = (SkiplistIz*)__VERIFIER_palloc(sizeof(SkiplistIz));
  new (list) SkiplistIz();

  __VERIFIER_pbarrier();

  list->insert(1, 10, 7);

  list->insert(4, 10, 7);

  return 0;

}
