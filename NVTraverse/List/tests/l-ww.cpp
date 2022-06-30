#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

#include "../ListOriginal.h"

__VERIFIER_persistent_storage(static ListOriginal* list);

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
