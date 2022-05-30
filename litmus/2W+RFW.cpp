#include <stdio.h>
#include <stdlib.h>
#include <atomic>
#include <pthread.h>
#include <assert.h>
#include <genmc.h>

#define relaxed std::memory_order_relaxed

static pthread_t threads[2];
static int param[2] = {0, 1};

__VERIFIER_persistent_storage(std::atomic_int x);
__VERIFIER_persistent_storage(std::atomic_int y);
__VERIFIER_persistent_storage(std::atomic_int z);
int a = 0;

extern "C"{
 void __VERIFIER_clflush(void*);
}


void *thread1(void *param)
{

  x.store(42, relaxed);
  y.store(7, relaxed);

  return NULL;

}

void *thread2(void *param)
{

  a = y.load();
  if (a != 0) {
    __VERIFIER_clflush(&x);
    z.store(1, relaxed);
  }

  return NULL;

}

void __VERIFIER_recovery_routine(void)
{

  if (z.load(relaxed) == 1)
    assert(x.load(relaxed) == 42);

  return;

}

int main()
{

  x.store(0, relaxed);
  y.store(0, relaxed);
  z.store(0, relaxed);

  __VERIFIER_pbarrier();

  pthread_create(&threads[0], NULL, thread1, &param[0]);
  pthread_create(&threads[1], NULL, thread2, &param[1]);

  pthread_join(threads[0], NULL);
  pthread_join(threads[1], NULL);

  return 0;

}
