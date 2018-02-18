#pragma once
#include <stddef.h>
#include <stdint.h>
#include <utility>
#include <stdlib.h>

namespace incremental {

class Object;

enum class GCColor {
  white = 0,
  dirty = 1, // has been written to since last gc finalization
  grey = 2,
  black = 3
};

struct IncrementalGC {

  char *total_mem_block;
  Object *total_object_block;
  char *old_region;
  char *cur_region;
  size_t size;
  size_t rg_sz;
  size_t alloc_pos;
  bool did_write;
  bool in_gc;
  GCColor white;
  GCColor black;

  void gc_from(Object *from);

  Object *relocate(Object **objp, Object *obj);

  void yield() {}

  bool needs_gc(GCColor col) {
    // here it's black since the colors swap
    // need to make this operate in a single color space
    return col == black || col == GCColor::dirty;
  }

  bool in_gc_region(Object *o) {
    char *co = (char *)o;
    return co >= old_region && co < old_region + rg_sz;
  }

  void swap_gc_regions() {
    alloc_pos = 0;
    std::swap(white, black);
    std::swap(cur_region, old_region);
  }

public:

  bool needs_scan(GCColor col) {
    bool needed_gc = (in_gc & needs_gc(col));
    did_write |= needed_gc;
    return needed_gc;
  }

  const static size_t ForwardSize = 8;

  void *allocate(size_t sz) {
    // assume rounded up for now
    if (alloc_pos + sz > rg_sz) {
      return nullptr;
    }
    void *rval = cur_region + alloc_pos;
    alloc_pos += sz;
    return rval;
  }


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

  IncrementalGC(size_t sz) {
    sz = sz + (sz & 1);
    total_mem_block = (char *)malloc(sizeof(sz));
    cur_region = total_mem_block;
    size = sz;
    rg_sz = size / 2;
    alloc_pos = 0;
    old_region = total_mem_block + rg_sz;
    white = GCColor::white;
    black = GCColor::black;
    in_gc = false;
    did_write = false;
  }
};

extern IncrementalGC gc;

}

