
#ifndef DURABLE_QUEUE_H_
#define DURABLE_QUEUE_H_

// #include <atomic>
#include <new>
#include <assert.h>
#include <genmc.h>
#include "Utilities.h"

#define MAXNODES 10
#define MAX_THREADS 3

#define relaxed std::memory_order_relaxed

/*
 * All variables that are read during recovery should have been declared
 * as persistent. The function allocate node is triggered during the
 * initialization of the List and dynamically allocates these nodes with
 * palloc. Whenever a new node is needed, the function getNewNode() is
 * called in order to return a pointer to one of the preallocated nodes.
 * NOTE: Does fetch_add() affect the ordering?
 */
__VERIFIER_persistent_storage(NodeWithID* nodes[MAXNODES]);

static int node_idx;

void allocateNodes()
{

  node_idx = 0;
  for (int i = 0; i < MAXNODES; i++) {
    nodes[i] = (NodeWithID *)__VERIFIER_palloc(sizeof(NodeWithID));
    new (nodes[i]) NodeWithID(INT_MAX);
  }
  MFENCE();

}

NodeWithID* getNewNode()
{
  return nodes[node_idx++];
}

/* This queue preserves the durable linearizability definitions. This version
 * does NOT contain any memory management. Every returned value from a dequeue
 * operation is saved within the returned values array in case there is a crash
 * after ther dequeue and before the value was returned to the caller. However,
 * this array is not necessaty for satisfying durable inearizability.
 */
class DurableQueue {
public:
  // The removedValues array. Each thread has an entrance where it saves
  // the value of the last node it managed to dequeue. Relevant in case
  // there is a crash after the value was removed and before the value
  // was returned to the caller.
  int* removedValues[MAX_THREADS * PADDING];

  DurableQueue() {
    allocateNodes();
    tail = getNewNode();
    head = tail;
    FLUSH(tail);
    FLUSH(head);
    // BARRIER(&tail);
    // BARRIER(&head);
    for (int i = 0; i < MAX_THREADS; i++) {
      removedValues[i * PADDING] = nullptr;
      FLUSH(removedValues[i * PADDING]);
    }
  }

  /* Enqueues a node to the queue with the given value. */
  bool enq(int value) {
    NodeWithID* node = getNewNode();
    node->threadID = value;
    FLUSH(node);
    while (true) {
      NodeWithID* last = tail; //tail.load(relaxed);
      NodeWithID* next = last->next;//.load(relaxed);
      if (last == tail/*.load()*/) {
        if (next == nullptr) {
          // if (last->next.compare_exchange_strong(next, node)) {
          if (CAS(&(last->next), next, node)) {
            // BARRIER_OPT(&last->next);
            FLUSH(last->next);
            // tail.compare_exchange_strong(last, node);
            CAS(&tail, last, node);
            FLUSH(tail);
            return true;
          }
        }
        else {
          FLUSH(last->next);
          // BARRIER_OPT(&last->next);
          // tail.compare_exchange_strong(last, next);
          CAS(&tail, last, next);
          FLUSH(tail);
          continue;
        }
      }
    }
  }

  /* Tries to dequeue a node. Returns the value of the removed node. If the
   * queue is empty, it returns INT_MIN which symbols an empty queue. In
   * addition, it saves the returned value in the thread's location at the
   * returnedValues arreay. In order to remove the value, it first stamps the
   * value with the threadID - this is what indicates that the node was
   * removed.
   */
  // int deq(int threadID) {
  //   int* newRemovedValue = new int(INT_MAX);
  //   FLUSH(newRemovedValue);
  //   // BARRIER(newRemovedValue);
  //   removedValues[threadID * PADDING] = newRemovedValue;
  //   FLUSH(removedValues[threadID * PADDING]);
  //   // BARRIER(&removedValues[threadID * PADDING]);
  //   while (true) {
  //     NodeWithID* first = head.load(relaxed);
  //     NodeWithID* last = tail.load(relaxed);
  //     NodeWithID* next = first->next.load(relaxed);
  //     if (first == head.load()) {
  //       if (first == last) {
  //         if (next == nullptr) {
  //           *removedValues[threadID * PADDING] = INT_MIN;
  //           FLUSH(removedValues[threadID * PADDING]);
  //           // BARRIER(removedValues[threadID * PADDING]);
  //           return INT_MIN;
  //         }
  //         FLUSH(last->next);
  //         // BARRIER_OPT(&last->next);
  //         tail.compare_exchange_strong(last, next);
  //       }
  //       else {
  //         int value = next->value;
  //         // Mark the node as removed by changing the threadID field
  //         int valid = -1;
  //         if (next->threadID.compare_exchange_strong(valid, threadID)) {
  //             // BARRIER(&next->threadID);
  //             __VERIFIER_clflush(&next->threadID);
  //             *removedValues[threadID * PADDING] = value;
  //             // BARRIER_OPT(removedValues[threadID * PADDING]);
  //             FLUSH(removedValues[threadID * PADDING]);
  //             head.compare_exchange_strong(first, next); // Update head
  //             return value;
  //         }
  //         else {
  //           int* address = removedValues[next->threadID * PADDING];
  //           if (head.load(relaxed) == first){
  //               // BARRIER(&next->threadID);
  //               __VERIFIER_clflush(&next->threadID);
  //               *address = value;
  //               FLUSH(address);
  //               // BARRIER_OPT(address);
  //               head.compare_exchange_strong(first, next);
  //           }
  //         }
  //       }
  //     }
  //   }
  // }

  bool isEmpty() {
    return (head == tail);
  }

  int getSize() {
    int size = 0;
    NodeWithID *aux = tail;
    while (aux != head) {
      size++;
      aux = aux->next;
    }
    return size;
  }

  private:
    // std::atomic<NodeWithID*> head;
    NodeWithID* head;
    int padding[PADDING];
    // std::atomic<NodeWithID*> tail;
    NodeWithID* tail;

};

#endif /* DURABLE_QUEUE_H_ */
