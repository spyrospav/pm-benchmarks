#ifndef MS_QUEUE_H_
#define MS_QUEUE_H_

#include <atomic>
#include <new>
#include <genmc.h>
#include "Utilities.h"

#define MAXNODES 10
#define relaxed std::memory_order_relaxed
//=============================Start MSQueue Class==========================//
/* This queue is Michael and Scott's queue from DISC 1996 which is the baseline
 * of the java.util.concurrent librraty. It is not-persistent and is the
 * baseline of all its durable versions. This version DOES NOT contain any
 * memory management.
 */

 /* Node is the type of the elements that will be in the queue.
  * It contains the following fields:
  * value     - can be of any type. It holds the data of the element.
  * next      - a pointer to the next element in the queue.
  */
 class Node {
   public:
     int value;
     Node* next;
     Node(int val) : value(val), next(nullptr) {}
     Node() : value(int()), next(nullptr) {}
 };

 /*
  * All variables that are read during recovery should have been declared
  * as persistent. The function allocate node is triggered during the
  * initialization of the List and dynamically allocates these nodes with
  * palloc. Whenever a new node is needed, the function getNewNode() is
  * called in order to return a pointer to one of the preallocated nodes.
  * NOTE: Does fetch_add() affect the ordering?
  */
 __VERIFIER_persistent_storage(Node* nodes[MAXNODES]);

 static int node_idx;

 void allocateNodes()
 {

   node_idx = 0;
   for (int i = 0; i < MAXNODES; i++) {
     nodes[i] = (Node *)__VERIFIER_palloc(sizeof(Node));
     new (nodes[i]) Node(INT_MAX);
   }

 }

 Node* getNewNode()
 {
   return nodes[node_idx++];
 }

class MSQueue {
public:

  MSQueue() {
    allocateNodes();
    // Node* dummy = new Node(INT_MAX);
    Node* dummy = getNewNode();
    dummy->value = INT_MAX;
    tail = dummy;
    head = dummy;
    MFENCE();
  }

  /* Enqueues a node to the queue with the given value. */
  bool enq(int value) {
    Node* node = new Node(value);
    node->next = NULL;
    // Node* node = getNewNode();
    node->value = value;
    while (true) {
      Node* last = tail;
      Node* next = last->next;
      if (last == tail) {
        //not necessary but checks again before try
        if (next == NULL) {
          if (CAS(&last->next, next, node)) {
          // if (last->next.compare_exchange_strong(next, node)) {
            // tail.compare_exchange_strong(last, node);
            CAS(&tail, last, node);
            return true;
          }
        }
        else {
          CAS(&tail, last, next);
          // tail.compare_exchange_strong(last, next);
        }
      }
    }
  }

  //-------------------------------------------------------------------------

  /* Tries to dequeue a node. Returns the value of the removed node.
   * If the queue is empty, it returns INT_MIN which symbols an
   * empty queue.
   */
  int deq(){
    while (true) {
      Node* first = head;
      Node* last = tail;
      Node* next = first->next;
      if (first == head) {
        if (first == last) {
          if (next == nullptr) {
            return INT_MIN;
          }
          CAS(&tail, last, next);
          // tail.compare_exchange_strong(last, next);
        }
        else {
          int value = next->value;
          if (CAS(&head, first, next)) {
          // if (head.compare_exchange_strong(first, next)) {
            return value;
          }
        }
      }
    }
  }

  bool isEmpty() {
    return (head == tail);
  }

  int getSize() {
    if (head == tail) return 0;
    assert(head != tail);
    int size = 0;
    Node *aux = head;
    do{
      aux = aux->next;
      size++;
    } while(aux->next);

    tail = aux;
    return size;
  }

private:
  Node* head;
  Node* tail;
};

#endif /* MS_QUEUE_H_ */
