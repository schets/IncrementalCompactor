#pragma once
#include <cstddef>

namespace incremental {

class Object;

class IncrementalGC {

  bool curColor;

public:

  void *allocate(std::size_t sz);

  bool advanceAllocator();

  bool setModified();


  // initializes a GC over the roots of an object
  void gcRoots(Object** roots, int n_roots);

  // moves the object at the specfied pointer, and re-writes the pointer
  void relocate(Object **obj);
};

}

