#include <stdio.h>
#include <stdlib.h>
#include <atomic>
#include <pthread.h>
#include <assert.h>
#include <genmc.h>

#define relaxed std::memory_order_relaxed

__VERIFIER_persistent_storage(std::atomic_int x);
__VERIFIER_persistent_storage(std::atomic_int y);

extern "C"{
 void __VERIFIER_clflush(void*);
}

void __VERIFIER_recovery_routine(void)
{

  if (y.load(relaxed) == 1)
    assert(x.load(relaxed) == 1);

  return;

}

int main() {

  x.store(0, relaxed);
  y.store(0, relaxed);

  __VERIFIER_pbarrier();

  x.store(1, relaxed);
  y.store(1, relaxed);

  return 0;

}
