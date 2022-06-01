#ifndef LIST_TRAVERSE_H_
#define LIST_TRAVERSE_H_

#include "../Utilities.h"
#include <assert.h>
#include <genmc.h>
#include <new>

#define MAXNODES 10

class Node{
public:
	int value;
	int key;
	Node* volatile next;

	Node(int val, int k, Node* n) {
		value = val;
		key = k;
		next = n;
	}

	Node(int val, int k) {
		value = val;
		key = k;
		next = NULL;
	}

	Node() {
		value = int();
		key = 0;
		next = NULL;
	}

	void set(int k, int val, Node* n) {
		key = k;
		value = val;
		next = n;
	}

	Node* getNextF() {
		Node* n = next;
		FLUSH(&next);
		return n;
	}

	Node* getNext() {
		Node* n = next;
		return n;
	}

	bool CAS_nextF(Node* exp, Node* n) {
		Node* old = next;
		if(exp != old) {
			FLUSH(&next);
			return false;
		}
		bool ret = CAS(&next, old, n);
		FLUSH(&next);
		return ret;
	}

	bool CAS_next(Node* exp, Node* n) {
		Node* old = next;
		if(exp != old) return false;
		bool ret = CAS(&next, old, n);
		return ret;
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

class ListTraverse {
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

	ListTraverse() {
    allocateNodes();
    head = getNewNode();
		head->set(INT_MIN, INT_MIN, NULL);
	}

  Node* getAdd(Node* n) {
    long node = (long)n;
    return (Node*)(node & ~(0x1L)); // clear bit to get the real address
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

	Window* find(Node* head, int key/*, EpochThread epoch*/) {
    Node* leftParent = head;
    Node* left = head;
    Node* leftNext = head->getNext();
    Node* right = NULL;

    Node* pred = NULL;
    Node* curr = NULL;
    Node* currAdd = NULL;
    Node* succ = NULL;
    bool marked = false;
    int numNodes = 0;
    while (true) {
      numNodes = 0;
      pred = head;
      curr = head;
      currAdd = curr;
      succ = currAdd->getNext();
      marked = getMark(succ);
      /* 1: Find left and right */
      while (marked || currAdd->key < key) {
        if (!marked) {
          leftParent = pred;
          left = currAdd;
          leftNext = succ;
          numNodes = 0;
          //nodes[numNodes++] = leftNext;
        }
        //nodes[numNodes++] = currAdd;
        pred = currAdd;
        curr = succ;
        currAdd = getAdd(curr);
        if (currAdd == NULL) {
            break;
        }
        succ = currAdd->getNext();
        marked = getMark(succ);
    	}
      right = currAdd;
      //nodes[numNodes++] = right;
        /* 2: Check nodes are adjacent */
      if (leftNext == right) {
        if ((right != NULL) && getMark(right->getNext())) {
            continue;
        }
				else {
          // nodes[numNodes++] = leftParent;
          // for (int i = 0; i < numNodes; i++) {
          //     if (nodes[i]) FLUSH(nodes[i]);
          // }
					Window* w = new Window(left, right);
          return w;
        }
      }
      // nodes[numNodes++] = leftParent;
      // for (int i = 0; i < numNodes; i++) {
      //   if (nodes[i]) {
      //     FLUSH(nodes[i]);
      //   }
      // }
      /* 3: Remove one or more marked nodes */
      if (left->CAS_nextF(leftNext, right)) {
        // for (int i = 1; i <= numNodes-3; i++) {
        //   if (nodes[i]) {
        //     ssmem_free(alloc, nodes[i]);}
        // }
        if ((right != NULL) && getMark(right->getNextF())) {
          continue;
        }
				else {
					Window* w = new Window(left, right);
          return w;
        }
      }
    }
  }

//=========================================

	bool insert(int k, int item) {
    while (true) {
      Window* window = find(head, k/*, epoch*/);
      Node* pred = window->pred;
      Node* curr = window->curr;
      if (curr && curr->key == k) {
        SFENCE();
        return false;
      }
			Node* node = getNewNode();
      node->set(k, item, curr);
      FLUSH(node);
      bool res = pred->CAS_nextF(curr, node);
      if (res) {
        SFENCE();
        return true;
      }
			// ssmem_free(alloc, node)
    }
  }


//========================================

	bool remove(int key) {
		bool snip = false;
		while (true) {
			Window* window = find(head, key/*, epoch*/);
			Node* pred = window->pred;
			Node* curr = window->curr;
			//ssmem_free(allocW, window);
			if (!curr || curr->key != key) {
	      SFENCE();
				return false;
			}
			else {
        Node* succ = curr->getNextF();
        Node* succAndMark = mark(succ);
        if (succ == succAndMark) {
            continue;
        }
        snip = curr->CAS_nextF(succ, succAndMark);
				if (!snip)
					continue;
        // if (pred->CAS_nextF(curr, succ)){
        //   ssmem_free(alloc, curr);
				// }
	      SFENCE();
		    return true;
			}
		}
	}

    //========================================

	bool contains(int key) {
    Node* pred = head;
    Node* curr = head;
    bool marked = getMark(curr->getNext());
    while (curr->key < key) {
      pred = curr;
      curr = getAdd(curr->getNext());
      if (!curr) {
        FLUSH(pred);
        SFENCE();
        return false;
      }
      marked = getMark(curr->getNext());
    }
    FLUSH(pred);
    FLUSH(curr);
    if(curr->key == key && !marked){
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

#endif /* LIST_TRAVERSE_H_ */
