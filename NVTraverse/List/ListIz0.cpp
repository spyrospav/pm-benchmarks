#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

#include "ListIz.h"

__VERIFIER_persistent_storage(static ListIz* list);

void __VERIFIER_recovery_routine(void)
{

  if (list->contains(3))
    assert(list->contains(0));

  return;

}

int main() {

  list = (ListIz*)__VERIFIER_palloc(sizeof(ListIz));
  new (list) ListIz();

  __VERIFIER_pbarrier();

  list->insert(0,10);
  list->insert(3,10);

  return 0;

}
