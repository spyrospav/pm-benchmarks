#ifndef _SKIPLIST_TRAVERSE_H_
#define _SKIPLIST_TRAVERSE_H_

#include <atomic>
#include "rand_r_32.h"
#include "Utilities.h"
#include <new>

#define MAXNODES 15

extern int levelmax;
/*
 * All variables that are read during recovery should have been declared
 * as persistent. The function allocate node is triggered during the
 * initialization of the List and dynamically allocates these nodes with
 * palloc. Whenever a new node is needed, the function getNewNode() is
 * called in order to return a pointer to one of the preallocated nodes.
 * NOTE: Does fetch_add() affect the ordering?
 */
__VERIFIER_persistent_storage(Node* nodes[MAXNODES]);

int node_idx;

void allocateNodes()
{

  node_idx = 0;
  for (int i = 0; i < MAXNODES; i++) {
    nodes[i] = (Node *)__VERIFIER_palloc(sizeof(Node));
    new (nodes[i]) Node();
  }

}

Node* getNewNode()
{
  return nodes[node_idx++];
}

class SkiplistTraverse {
public:

  SkiplistTraverse() {
    allocateNodes();
    Node *min, *max;
    max = getNewNode();
    max->set(INT_MAX, 0, nullptr, levelmax);
    BARRIER(max);
    min = getNewNode();
    min->set(INT_MIN, 0, max, levelmax);
    BARRIER(min);
    head = min;
    __VERIFIER_clflush(&head);
    MFENCE();
  }

	int size() {
    int size = 0;
    Node *node;
    node = (Node*)(getCleanReference(head->getNext(0)));
    while (node->getNext(0) != nullptr) {
      if (!isMarked(node->getNext(0))) {
        size++;
      }
      node = (Node*)(getCleanReference(node->getNext(0)));
    }
    return size;
  }

	int get(int key) {
    Node *succs[FRASER_MAX_MAX_LEVEL], *preds[FRASER_MAX_MAX_LEVEL];
    bool exists = search_no_cleanup(key, preds, succs);
    // FLUSH(preds[0]);
    // FLUSH(succs[0]);
    SFENCE();
    if (exists) {
      return succs[0]->val;
    }
    else {
      return 0;
    }
	}

	bool contains(int key) {
    return get(key) != 0;
	}

	bool remove(int key) {
    Node *succs[FRASER_MAX_MAX_LEVEL];
    bool found = search(key, nullptr, succs);
    if (!found) {
      SFENCE();
      return false;
    }
    Node *node_del = succs[0];
    bool my_delete = mark_node_ptrs(node_del);
    if (my_delete) {
      search(key, nullptr, nullptr);
      SFENCE();
      return true;
    }
    SFENCE();
    return false;
	}

	bool insert(int key, int val, int id) {
    Node *newNode, *pred, *succ;
    Node *succs[FRASER_MAX_MAX_LEVEL], *preds[FRASER_MAX_MAX_LEVEL];
    int i;
    bool found;

  retry:
    found = search(key, preds, succs);
  	if (found) {
	    SFENCE();
      return false;
  	}
    newNode = getNewNode();
		newNode->set(key, val, nullptr, get_rand_level(id));

		for (int i = 0; i < newNode->toplevel; i++) {
      newNode->next[i] = succs[i];
    }
		FLUSH(newNode);

  	/* Node is visible once inserted at lowest level */
  	Node *before = (Node*)(getCleanReference(succs[0]));
  	if (!preds[0]->CASNextF(before, newNode, 0)) {
      //ssmem_free(alloc, newNode);
      goto retry;
  	}

  	for (int i = 1; i < newNode->toplevel; i++) {
      while (true) {
        pred = preds[i];
        succ = succs[i];
        //someone has already removed that node
        if (isMarked(newNode->getNextF(i))) {
          SFENCE();
          return true;
        }
        if (pred->CASNextF(succ, newNode, i))
          break;
        search(key, preds, succs);
      }
    }
    SFENCE();
 		return true;
	}


private:
	Node *head;

	bool search(int key, Node **left_list, Node **right_list) {
    Node *left_parent, *left, *left_next, *right = nullptr, *right_next;
    Node* traverseNodes[MAXNODES*5];
  retry:
    left_parent = head;
  	left = head;
		int num_nodes = 0;
  	for (int i = levelmax - 1; i >= 0; i--) {
      left_next = left->getNext(i);

      if (isMarked(left_next)) {
        goto retry;
      }

      /* Find unmarked node pair at this level - left and right */
      for (right = left_next;; right = right_next) {
        /* Skip a sequence of marked nodes */
        right_next = right->getNext(i);
      	while (isMarked(right_next)) {
          right = (Node*)(getCleanReference(right_next));
          right_next = right->getNext(i);
          if (i == 0) {
  	        traverseNodes[num_nodes++] = right;
          }
        }
        if (right->key >= key) {
          break;
        }
        left_parent = left;
        left = right;
        left_next = right_next;
        num_nodes = 0;
      }
      if (i == 0) {
        traverseNodes[num_nodes++] = left_parent;
        for (int j = 0; j < num_nodes; j++) {
         FLUSH(traverseNodes[j]);
        }
      }
      bool cas = false;
      /* Ensure left and right nodes are adjacent */
      if (left_next != right) {
        bool cas = left->CASNextF(left_next, right, i);
        if (!cas) {
          goto retry;
        }
      }

      if (i == 0 && cas) {
        for (int j = 0; j < num_nodes-2; j++) {
           //ssmem_free(alloc, nodes[j]);
        }
      }
      if (left_list != nullptr) {
          left_list[i] = left;
      }
      if (right_list != nullptr) {
          right_list[i] = right;
      }
    }
    return (right->key == key);
  }

	bool search_no_cleanup(int key, Node **left_list, Node **right_list) {
    Node *left, *left_next, *right = nullptr;
    left = head;
    for (int i = levelmax - 1; i >= 0; i--) {
      left_next = (Node*)(getCleanReference(left->getNext(i)));
      right = left_next;
      while (true) {
        if (!isMarked(right->getNext(i))) {
          if (right->key >= key) {
            break;
          }
          left = right;
        }
        right = (Node*)(getCleanReference(right->getNext(i)));
      }

      if (left_list != nullptr) {
        left_list[i] = left;
      }
      if (right_list != nullptr) {
        right_list[i] = right;
      }
    }
    return (right->key == key);
  }

	bool search_no_cleanup_succs(int key, Node **right_list) {
    Node *left, *left_next, *right = nullptr;
    left = this->head;
    for (int i = levelmax - 1; i >= 0; i--) {
      left_next = (Node*)(getCleanReference(left->getNext(i)));
      right = left_next;
      while (true) {
        if (!isMarked(right->getNext(i))) {
          if (right->key >= key) {
            break;
          }
          left = right;
        }
        right = (Node*)(getCleanReference(right->getNext(i)));
      }
      right_list[i] = right;
    }
    return (right->key == key);
  }

	inline bool mark_node_ptrs(Node *n) {
    bool cas = false;
    Node *n_next;
    for (int i = n->toplevel - 1; i >= 0; i--) {
      do {
        n_next = n->getNextF(i);
        if (isMarked(n_next)) {
          cas = false;
          break;
        }
        Node *before = (Node*)(getCleanReference(n_next));
        Node *after = (Node*)(getMarkedReference(n_next));
        cas = n->CASNextF(before, after, i);
      } while (!cas);
    }
    return (cas); /* if I was the one that marked lvl 0 */
  }

	int get_rand_level(int seed) {
    int level = 1;
    for (int i = 0; i < levelmax - 1; i++) {
      if ((rand_r_32((unsigned int *)&seed) % 101) < 50)
        level++;
      else
        break;
    }
    return level;
  }

} __attribute__((aligned((64))));




#endif /* _SKIPLIST_TRAVERSE_H_ */
