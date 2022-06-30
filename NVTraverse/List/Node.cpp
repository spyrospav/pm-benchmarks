#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>
#include <new>

#include "Utilities.h"

__VERIFIER_persistent_storage(Node * mynodes[2]);

void __VERIFIER_recovery_routine(void)
{

  if (mynodes[1]->key == 2)
    assert(mynodes[0]->next == mynodes[1]);
  return;

}

int main() {

  mynodes[0] = (Node*)__VERIFIER_palloc(sizeof(Node));
  mynodes[1] = (Node*)__VERIFIER_palloc(sizeof(Node));

  new (mynodes[0]) Node(0, 10, NULL);
  new (mynodes[1]) Node(1, 10, NULL);

  __VERIFIER_pbarrier();

  mynodes[0]->next = mynodes[1];

  FLUSH(mynodes[0]);

  mynodes[1]->key = 2;

  return 0;

}
