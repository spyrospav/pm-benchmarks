## NVTraverse

The implementations of the various data structures are taken from [NVTraverse repository](https://github.com/michalfman/NVTraverse)[1].

## Data Structures

We are testing the following lock-free data structures:

  - [Linked-List](/List)
  - [Skiplist](/Skiplist)

Each data structure has 3 implementations:

  - Original, which does not use any PM primitive and therefore it is not durably linearizable
  - Izraelevitz construction[2], which is a very naive approach to ensure durability
  - NVTraverse transformation,

## Resources

[1] Michal Friedman, Naama Ben-David, Yuanhao Wei, Guy E. Blelloch, and Erez Petrank. 2020. NVTraverse: in NVRAM data structures, the destination is more important than the journey. In Proceedings of the 41st ACM SIGPLAN Conference on Programming Language Design and Implementation (PLDI 2020). Association for Computing Machinery, New York, NY, USA, 377–392. https://doi.org/10.1145/3385412.3386031

[2] Izraelevitz, Joseph, Hammurabi das Chagas Mendes and Michael L. Scott. “Linearizability of Persistent Memory Objects Under a Full-System-Crash Failure Model.” DISC (2016).
