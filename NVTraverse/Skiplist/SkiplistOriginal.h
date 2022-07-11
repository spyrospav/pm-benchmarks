#include <atomic>
#include "rand_r_32.h"
#include "Utilities.h"
#include <new>

extern int levelmax;

#define FRASER_MAX_MAX_LEVEL 2
#define MAXNODES 1000
#define relaxed std::memory_order_relaxed

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

class Node {
public:
  int key;
  int val;
  unsigned char toplevel;
  std::atomic<Node*> next[FRASER_MAX_MAX_LEVEL];

  Node() {
    key = INT_MIN;
    val = INT_MIN;
    for (int i = 0; i < levelmax; i++) {
      next[i].store(nullptr);
    }
    toplevel = get_rand_level(7);
  }

  Node(int k, int v, int topl) {
    key = k;
    val = v;
    toplevel = topl;
  }

  Node(int k, int v, Node* n, int topl) {
    key = k;
    val = v;
    toplevel = topl;
    for (int i = 0; i < levelmax; i++)
    {
      next[i].store(n, relaxed);
    }
  }

  void set(int k, int v, Node* n, int topl) {
    key = k;
    val = v;
    toplevel = topl;
    for (int i = 0; i < levelmax; i++) {
      next[i].store(n, relaxed);
    }
  }

  bool CASNext(Node* exp, Node* n, int i) {
    Node* old = next[i].load(relaxed);
    if (exp != old) {
      return false;
    }
    bool ret = next[i].compare_exchange_strong(exp, n);
    return ret;
  }

  Node* getNext(int i) {
    Node* n = next[i].load(relaxed);
    return n;
  }
};

/*
 * All variables that are read during recovery should have been declared
 * as persistent. The function allocateNodes is triggered during the
 * initialization of the List and dynamically allocates these nodes with
 * palloc. Whenever a new node is needed, the function getNewNode() is
 * called in order to return a pointer to one of the preallocated nodes.
 * NOTE: Does fetch_add() affect the ordering?
 */
__VERIFIER_persistent_storage(Node* nodes[MAXNODES]);

static std::atomic_int node_idx;

void allocateNodes()
{

  node_idx.store(0);
  for (int i = 0; i < MAXNODES; i++) {
    nodes[i] = (Node *)__VERIFIER_palloc(sizeof(Node));
    new (nodes[i]) Node();
  }

}

Node* getNewNode()
{
  if (node_idx.load(relaxed) == 0) printf("3\n");
  return nodes[node_idx.fetch_add(1)];
}

class SkiplistOriginal {
public:

	SkiplistOriginal() {
    allocateNodes();
    Node *min, *max;
    max = getNewNode();
    max->set(INT_MAX, 0, nullptr, levelmax);
	  min = getNewNode();
    min->set(INT_MIN, 0, max, levelmax);
    this->head = min;
    MFENCE();
	}

	SkiplistOriginal(int val) {
    allocateNodes();
    Node *min, *max;
    max = getNewNode();
    max->set(INT_MAX, 0, nullptr, levelmax);
    min = getNewNode();
    min->set(INT_MIN, val, max, levelmax);
    this->head = min;
    MFENCE();
	}

	~SkiplistOriginal() {
    Node *node, *next;
    node = this->head;
    while (node != NULL)
    {
      next = node->getNext(0);
      node = next;
    }
	}

	int size() {
    int size = 0;
    Node *node;
    /* Here */
    node = (Node *)(getCleanReference(this->head->getNext(0)));
    while (node->getNext(0) != nullptr) {
      if (!isMarked(node->getNext(0))) {
        size++;
      }
      /* Here */
      node = (Node *)(getCleanReference(node->getNext(0)));
    }
    return size;
  }

	int get(int key) {
    Node *left = left_search(key);
    return left->val;
	}

	bool contains(int key) {
    return get(key) != 0;
	}

	bool remove(int key) {
    Node *succs[FRASER_MAX_MAX_LEVEL];
    bool found = search_no_cleanup_succs(key, succs);
      if (!found) {
        return false;
      }
      Node *node_del = succs[0];
      bool my_delete = mark_node_ptrs(node_del);
      if (my_delete) {
        search(key, nullptr, nullptr);
        return true;
      }
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
      return false;
  	}
    /* Here */
    newNode = getNewNode();
		newNode->set(key, val, nullptr, get_rand_level(id));

		for (int i = 0; i < newNode->toplevel; i++) {
      newNode->next[i] = succs[i];
    }

  	/* Node is visible once inserted at lowest level */
  	Node *before = (Node *)(getCleanReference(succs[0]));
  	if (!preds[0]->CASNext(before, newNode, 0)) {
      /* Here */
      free(newNode);
      goto retry;
  	}
  	for (int i = 1; i < newNode->toplevel; i++) {
      while (true) {
        pred = preds[i];
        succ = succs[i];
        //someone has already removed that node
        if (isMarked(newNode->getNext(i))) {
          return true;
        }
        if (pred->CASNext(succ, newNode, i))
          break;
        search(key, preds, succs);
      }
    }

  	return true;

  }


private:
	Node *head;

	bool search(int key, Node **left_list, Node **right_list) {
    Node *left, *left_next, *right = nullptr, *right_next;
  retry:
  	left = this->head;
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
          right = (Node *)(getCleanReference(right_next));
          right_next = right->getNext(i);
			    if (i == 0){
            /* Here */
		        //nodes[num_nodes++] = right;
			    }
        }
        if (right->key >= key) {
          break;
        }
        left = right;
        left_next = right_next;
		    num_nodes = 0;
      }
	    bool cas = false;
      /* Ensure left and right nodes are adjacent */
      if (left_next != right) {
		    bool cas = left->CASNext(left_next, right, i);
		    if (!cas) {
          goto retry;
		    }
      }
	    if (i == 0 && cas) {
        for(int j = 0; j < num_nodes-1; j++) {
          /* Here */
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
    left = this->head;
    for (int i = levelmax - 1; i >= 0; i--) {
      left_next = (Node *)(getCleanReference(left->getNext(i)));
      right = left_next;
      while (true) {
        if (!isMarked(right->getNext(i))) {
          if (right->key >= key) {
            break;
          }
          left = right;
        }
        right = (Node *)(getCleanReference(right->getNext(i)));
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
      left_next = (Node *)(getCleanReference(left->getNext(i)));
      right = left_next;
      while (true) {
        if (!isMarked(right->getNext(i))) {
          if (right->key >= key) {
            break;
          }
          left = right;
        }
        right = (Node *)(getCleanReference(right->getNext(i)));
      }
      right_list[i] = right;
    }

    return (right->key == key);

  }

	Node* left_search(int key) {
    Node *left = nullptr, *left_prev;
    left_prev = this->head;
    for (int lvl = levelmax - 1; lvl >= 0; lvl--) {
      left = (Node *)(getCleanReference(left_prev->getNext(lvl)));
      while (left->key < key || isMarked(left->getNext(lvl))) {
        if (!isMarked(left->getNext(lvl))) {
          left_prev = left;
        }
        left = (Node *)(getCleanReference(left->getNext(lvl)));
      }
      if (left->key == key) {
        return left;
      }
    }

    return left_prev;

  }

	inline bool mark_node_ptrs(Node *n) {

    bool cas = false;
    Node *n_next;
    for (int i = n->toplevel - 1; i >= 0; i--) {
      do {
        n_next = n->getNext(i);
        if (isMarked(n_next)) {
          cas = false;
          break;
        }
        Node *before = (Node *)(getCleanReference(n_next));
        Node *after = (Node *)(getMarkedReference(n_next));
        cas = n->CASNext(before, after, i);
      } while (!cas);
    }

    return (cas);

  }

};
