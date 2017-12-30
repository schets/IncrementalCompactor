#pragma once
#include <stddef.h>
#include <stdint.h>

namespace incremental {

class Object;

class IncrementalGC {

  bool cur_color;
  char *total_mem_block;

  void gcObject(Object *from);

public:

  const static size_t ForwardSize = 8;

  void *allocate(size_t sz);


  // initializes a GC over the roots of an object
  void gc_roots(Object** roots, int n_roots);

  // moves the object at the specfied pointer, and re-writes the pointer
  Object *relocate(Object **obj);

  Object *get_ptr_offset(uint32_t at) {
    return (Object *)(total_mem_block + ForwardSize * at);
  }

  uint32_t get_forward_addr(Object *val) {
    return ((char *)val - total_mem_block) / ForwardSize;
  }
};

}

