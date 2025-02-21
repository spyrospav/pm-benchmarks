#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <stddef.h>
#include <genmc.h>
#include <atomic>

#define INT_MIN -1
#define INT_MAX 100
#define MAX_THREAD 10
#define CAS __sync_bool_compare_and_swap

inline void MFENCE()
{
  __sync_synchronize();
}

extern "C"{
 void __VERIFIER_clflush(void*);
}

/* NodeWithID is the type of the elements that will be in the queue.
 * It contains the following fields:
 * value     - can be of any type. It holds the data of the element.
 * next      - a pointer to the next element in the queue.
 * threadID  - holds the id of the thread that manages to dequeue this
 *             node. Helps for saving the returned value before a crash.
 */
class NodeWithID {
public:
  int value;
  NodeWithID* next;
  int threadID;
  NodeWithID(int val) : value(val), next(nullptr), threadID(10) {}
};

void FLUSH(NodeWithID* node) {
  // __VERIFIER_clflush(&node);
  __VERIFIER_clflush(&(node->value));
  __VERIFIER_clflush(&(node->next));
  __VERIFIER_clflush(&(node->threadID));
}

// void FLUSH(void *p) {
//   __VERIFIER_clflush(&p);
// }

void SFENCE() {
    // asm volatile ("sfence" ::: "memory");
}


#endif /* UTILITIES_H_ */
