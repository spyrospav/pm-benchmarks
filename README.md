# Benchmarks for verifying Persistent Memory programs

This repository contains various benchmarks for Persistent Memory. For the verification process we are using an extension of PerSeVerE [1] (a tool based on [GenMC](https://github.com/MPI-SWS/genmc) [2]), which supports the Px86 memory persistency model [3].

# Resources

[1] Michalis Kokologiannakis, Ilya Kaysin, Azalea Raad, and Viktor Vafeiadis. 2021. PerSeVerE: persistency semantics for verification under ext4. Proc. ACM Program. Lang. 5, POPL, Article 43 (January 2021), 29 pages. https://doi.org/10.1145/3434324

[2] Michalis Kokologiannakis and Viktor Vafeiadis. 2021. GenMC: A Model Checker for Weak Memory Models. In Computer Aided Verification: 33rd International Conference, CAV 2021, Virtual Event, July 20–23, 2021, Proceedings, Part I. Springer-Verlag, Berlin, Heidelberg, 427–440. https://doi.org/10.1007/978-3-030-81685-8_20

[3] Azalea Raad, John Wickerson, Gil Neiger, and Viktor Vafeiadis. 2019. Persistency semantics of the Intel-x86 architecture. Proc. ACM Program. Lang. 4, POPL, Article 11 (January 2020), 31 pages. https://doi.org/10.1145/3371079
