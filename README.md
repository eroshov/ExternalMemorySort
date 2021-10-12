# External Memory k-way Merge Sort

An exercise-implementation of classic external memory sort algorithm with few optimizations.
The classic algorithm is rather straightforward:
1. Split input on chunks, so that each chunk occupy whole main memory.
2. Sort each chunk using basic sort algorithm and write corresponding output files.
3. Recursively merge this files using k-way merge until only one left, 
   where k is equal to main memory capacity divided by file stream buffer size.
4. When the previous step is completed only 1 file is remained and this is the answer.

## Key notes
* Input file is assumed to be a binary file, containing unsigned integers. 
  Output produced by the algorithm is a binary file too, and it is the sorted input
* For k-way merge we use, of course, heap. 
  Initially, it is built in linear time using std::make_heap (that is called by heap constructor)
* We parallelized the 2nd step: we read input file and sort chunks of size ```main_memory / num_threads``` in parallel.
  We use no more than 4 threads here as IO from both HDD and SSD do not allow higher order of parallelization.
* We pre-allocated enough memory for containers where possible and use resize(), instead of creating new containers
* To run tests ```make test```

Algorithm implementation is in ```sort_performer.cpp```
