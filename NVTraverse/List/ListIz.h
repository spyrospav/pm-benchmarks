#ifndef LIST_IZ_H_
#define LIST_IZ_H_

#include "../Utilities.h"
#include <assert.h>
#include <genmc.h>
#include <new>

#define MAXNODES 10

class Node{
public:
	int key;
	int value;
	Node* volatile next;

	Node(int k, int val) {
		key = k;
		value = val;
		next = NULL;
	}

	Node(int k, int val, Node* n) {
		key = k;
		value = val;
		next = n;
	}

	Node() {
		key = INT_MIN;
		value = int();
		next = NULL;
	}

	void set(int k, int val, Node* n) {
		key = k;
		value = val;
		next = n;
	}

	bool CAS_next(Node* exp, Node* n) {
		Node* old = next;
		if (exp != old) {
			BARRIER(&next);
			return false;
		}
		bool ret = CAS(&next, exp, n);
		BARRIER(&next);
		return ret;
	}

	Node* getNext() {
		Node* n = next;
		BARRIER(&next);
		return n;
	}
};

/*
 * All variables that are read during recovery should have been declared
 * as persistent. The function allocate node is triggered during the
 * initialization of the List and dynamically allocates these nodes with
 * palloc. Whenever a new node is needed, the function getNewNode() is
 * called in order to return a pointer to one of the preallocated nodes.
 * NOTE: Does fetch_add() affect the ordering?
 */
__VERIFIER_persistent_storage(Node * nodes[MAXNODES]);

static std::atomic_int node_idx;

void allocateNodes()
{
  node_idx.store(0);
  for (int i = 0; i < MAXNODES; i++) {
    nodes[i] = (Node *)__VERIFIER_palloc(sizeof(Node));
    nodes[i]->key = INT_MIN;
    nodes[i]->value = int();
    nodes[i]->next = NULL;
  }

}

Node* getNewNode()
{
  return nodes[node_idx.fetch_add(1)];
}

class ListIz{
public:

	class Window {
	public:
		Node* pred;
		Node* curr;
		Window(Node* myPred, Node* myCurr) {
			pred = myPred;
			curr = myCurr;
		}
  };

	ListIz() {
		allocateNodes();
    head = getNewNode();
		head->set(INT_MIN, INT_MIN, NULL);
	}

  Node* getAdd(Node* n) {
    long node = (long)n;
    return (Node*)(node & ~(0x1L));
  }

  bool getMark(Node* n) {
    long node = (long)n;
    return (bool)(node & 0x1L);
  }

  Node* mark(Node* n) {
    long node = (long)n;
    node |= 0x1L;
    return (Node*)node;
  }

	Window* find(Node* head, int key) {
    Node* left = head;
    Node* leftNext = head->getNext();
    Node* right = NULL;
    Node* curr = NULL;
    Node* currAdd = NULL;
    Node* succ = NULL;
    bool marked = false;
    int numNodes = 0;
    while (true) {
      numNodes = 0;
      curr = head;
      currAdd = curr;
      succ = currAdd->getNext();
      marked = getMark(succ);
      /* 1: Find left and right */
      while (marked || currAdd->key < key) {
        if (!marked) {
          left = currAdd;
          leftNext = succ;
          numNodes = 0;
        }
        //nodes[numNodes++] = currAdd;
        curr = succ;
        currAdd = getAdd(curr);
        if (currAdd == NULL) {
          break;
        }
        succ = currAdd->getNext();
        marked = getMark(succ);
      }

      right = currAdd;

			/* 2: Check nodes are adjacent */
      if (leftNext == right) {
        if ((right != NULL) && getMark(right->getNext())) {
          continue;
        }
				else {
					Window* w = new Window(left, right);
					return w;
        }
      }

			/* 3: Remove one or more marked nodes */
      if (left->CAS_next(leftNext, right)) {
        for (int i = 1; i < numNodes; i++) {
          // if (nodes[i]) {
          //   ssmem_free(alloc, nodes[i]);
          // }
        }
        if ((right != NULL) && getMark(right->getNext())) {
          continue;
        }
				else {
					Window* w = new Window(left, right);
					return w;
        }
      }
    }
  }

	bool insert(int k, int item) {
		while (true) {
			Window* window = find(head, k);
      Node* curr = window->curr;
      Node* pred = window->pred;
			// delete Window();=
    	if (curr && curr->key == k) {
        	return false;
    	}
    	Node* node = getNewNode();
			node->set(k, item, curr);
      FLUSH(node);
      if (pred->CAS_next(curr, node)) {
        SFENCE();
        return true;
      }
			else {
				// We don't care about GC now
				// ssmem_free(alloc, node);
			}
		}
	}

	bool remove(int k) {
		int key = k;
		bool snip;
		while (true) {
			Window* window = find(head, key);
			Node* pred = window->pred;
			Node* curr = window->curr;
			// delete Window();
			if (!curr || curr->key != key) { //load - immutable
				SFENCE();
				return false;
			}
			else {
        Node* succ = curr->getNext();
        Node* succAndMark = mark(succ);
        if (succ == succAndMark) {
            continue;
        }
        snip = curr->CAS_next(succ, succAndMark);
        if (!snip)
          continue;
				if (pred->CAS_next(curr, succ)) {
					// We don't care about GC now
					// ssmem_free(alloc, curr);
				}
				SFENCE();
        return true;
			}
		}
	}

	bool contains(int k) {
    int key = k;
    Node* curr = head;
    bool marked = getMark(curr->getNext());
    while (curr->key < key) {
      curr = getAdd(curr->getNext());
      if (!curr) {
        SFENCE();
        return false;
      }
      marked = getMark(curr->getNext());
    }
		if (curr->key == key && !marked){
      SFENCE();
      return true;
		}
		else {
      SFENCE();
      return false;
		}
	}

	private:
		Node* volatile head;

};
#endif /* LIST_IZ_H_ */
