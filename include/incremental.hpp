#pragma once
#include <stddef.h>
#include <stdint.h>

namespace incremental {

class Object;

enum class GCColor {
  white = 0,
  dirty = 1, // has been written to since last gc finalization
  grey = 2,
  black = 3
};

class IncrementalGC {

  char *total_mem_block;
  bool cur_color;
  bool did_write;
  bool in_gc;
  GCColor white;
  GCColor black;

  void gc_from(Object *from);

  Object *relocate(Object **objp, Object *obj);

  void yield() {}

  bool needs_gc(GCColor col) {
    return col == white || col == GCColor::dirty;
  }

  bool in_gc_region(Object *) {
    return true;
  }

public:

  bool needs_scan(GCColor col) {
    bool needed_gc = (in_gc & needs_gc(col));
    did_write |= needed_gc;
    return needed_gc;
  }

  const static size_t ForwardSize = 8;

  void *allocate(size_t sz);


  // initializes a GC over the roots of an object
  void gc_roots(Object** roots, int n_roots);


  Object *get_ptr_offset(uint32_t at) {
    return (Object *)(total_mem_block + ForwardSize * at);
  }

  uint32_t get_forward_addr(Object *val) {
    return ((char *)val - total_mem_block) / ForwardSize;
  }

  void set_write_happened() {
    did_write = true;
  }
};

IncrementalGC gc;

}

