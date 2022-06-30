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

  if (y.load(relaxed) == 5)
    assert(y.load(relaxed) >= 4);

  return;

}

int main() {

  x.store(0, relaxed);
  y.store(0, relaxed);

  __VERIFIER_pbarrier();

  y.store(1, relaxed);
  x.store(2, relaxed);

  y.store(3, relaxed);
  x.store(4, relaxed);

  __VERIFIER_clflush(&x);
  __VERIFIER_clflush(&y);

  y.store(5, relaxed);
  x.store(6, relaxed);

  return 0;

}
