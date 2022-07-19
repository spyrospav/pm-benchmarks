#ifndef DURABLE_QUEUE_H_
#define DURABLE_QUEUE_H_

#include "Utilities.h"
#include <assert.h>
#include <genmc.h>
#include <atomic>
#include <new>

#define MAXNODES 5
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

std::atomic<int> node_idx;

void allocateNodes()
{

  node_idx.store(0);
  for (int i = 0; i < MAXNODES; i++) {
    nodes[i] = (NodeWithID *)__VERIFIER_palloc(sizeof(NodeWithID));
    new (nodes[i]) NodeWithID(INT_MAX);
  }
  MFENCE();

}

NodeWithID* getNewNode()
{
  return nodes[node_idx.fetch_add(1)];
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
  int removedValues[MAX_THREADS + 1];

  DurableQueue() {
    allocateNodes();
    // NodeWithID* dummy = getNewNode();
    // NodeWithID* dummy = getNewNode();
    // FLUSH(dummy);
    tail = getNewNode();
    FLUSH(tail);
    head = tail;
    __VERIFIER_clflush(&tail);
    __VERIFIER_clflush(&head);
    for (int i = 0; i < MAX_THREADS + 1; i++) {
      removedValues[i] = INT_MIN;
      __VERIFIER_clflush(&removedValues[i]);
    }
    MFENCE();
  }

  /* Enqueues a node to the queue with the given value. */
  bool enq(int value) {
    NodeWithID* node = getNewNode();
    node->value = value;
    FLUSH(node);
    NodeWithID* last = tail;
    NodeWithID* next = last->next;
    while (true) {
      last = tail;
      next = last->next;
      if (last == tail) {
        if (next == nullptr) {
          if (CAS(&(last->next), next, node)) {
            __VERIFIER_clflush(&(last->next));
            CAS(&tail, last, node);
            // __VERIFIER_clflush(&tail);
            return true;
          }
        }
        else {
          __VERIFIER_clflush(&(last->next));
          CAS(&tail, last, next);
          // __VERIFIER_clflush(&tail);
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
  int deq(int threadID) {
    int* newRemovedValue = new int(INT_MAX);
    __VERIFIER_clflush(&newRemovedValue);
    removedValues[threadID] = *newRemovedValue;
    __VERIFIER_clflush(&removedValues[threadID]);
    NodeWithID* first = head;
    NodeWithID* last = tail;
    while (true) {
      first = head;
      last = tail;
      NodeWithID* next = first->next;
      if (first == head) {
        if (first == last) {
          if (next == nullptr) {
            removedValues[threadID] = INT_MIN;
            __VERIFIER_clflush(&removedValues[threadID]);
            return INT_MIN;
          }
          __VERIFIER_clflush(&(last->next));
          CAS(&tail, last, next);
        }
        else {
          int value = next->value;
          // Mark the node as removed by changing the threadID field
          int valid = -1;
          if (CAS(&(next->threadID), 10, threadID)) {
              __VERIFIER_clflush(&(first->next->threadID));
              removedValues[threadID] = value;
              __VERIFIER_clflush(&removedValues[threadID]);
              CAS(&head, first, next);
              return value;
          }
          else {
            assert(removedValues[next->threadID] > -1);
            int address = removedValues[next->threadID];
            if (head == first){
                __VERIFIER_clflush(&(first->next->threadID));
                address = value;
                __VERIFIER_clflush(&address);
                CAS(&head, first, next);
            }
          }
        }
      }
    }
  }

  bool isEmpty() {
    return (head == tail);
  }

  int getSize() {
    int size = 0;
    NodeWithID *aux = head;
    do{
      aux = aux->next;
      size++;
    } while(aux->next);
    return size;
  }

  void runRecovery() {
    NodeWithID *aux = head;
    do{
      aux = aux->next;
    } while(aux->next);
    tail = aux;
    return;
  }

private:
  NodeWithID* head;
  NodeWithID* tail;

};

#endif /* DURABLE_QUEUE_H_ */
