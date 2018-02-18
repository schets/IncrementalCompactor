#include <incremental.hpp>
#include <string.h>
#include <object.hpp>

namespace incremental {

IncrementalGC gc(100000);

void IncrementalGC::gc_roots(Object** roots, int n_roots) {
  swap_gc_regions();
  for (int i = 0; i < n_roots; i++) {
    gc_from(roots[i]);
  }
}

// Is the problem here trying to mix
// fixing forward references and GC in the same loop?
void IncrementalGC::gc_from(Object *from) {
  if (from == nullptr) {
    return;
  }
  bool needed_gc = true;
  from->color = GCColor::grey;
  // if this loop is depth-first, no need to have two loops?
  while (needed_gc || from->color == GCColor::dirty) {
    needed_gc = false;
    for (int i = 0; i < from->num_objects; i++) {
      // this code is reused across the initial GC and cleanup stage
      // theoretically, this could change across yields...
      Object **objs = from->load_objs();
      if (objs[i] == nullptr) {
        continue;
      }
      Object *obj = Object::get_forwarded_pointer(objs[i], this);

      // some redundancies here? Need to double-check the algo+color states
      if (needs_gc(obj->color) &&
          in_gc_region(obj)) // roots are allocated outside of the GC region
      {
        needed_gc = true;
        obj->color = GCColor::grey;
        Object *new_obj = relocate(&objs[i], obj);
        if (new_obj != nullptr) {
          obj->is_forwarding = true;
          obj->forward_addr = get_forward_addr(new_obj);
          obj = new_obj;
        } else {
          // something's fucky, this is an error
          // malloc and go with it? user provided function?
          // run a full GC?
          return;
        }
        yield();
      }
      objs[i] = obj;
    }
  }

  // the final iteration of this loop will not have yielded
  // since yields only can occur inside a movement operation
  // mark color as black
  from->color = black;

  for (int i = 0; i < from->num_objects; i++) {
    Object **objs = from->load_objs();
    if (objs[i] == nullptr) {
      continue;
    }
    Object *obj = Object::get_forwarded_pointer(objs[i], this);
    gc_from(obj);
  }
}

Object *IncrementalGC::relocate(Object **objp, Object *obj) {

  Object *new_obj = (Object *)allocate(obj->size);
  if (new_obj == nullptr) {
    return nullptr;
  }
  memcpy((char *)new_obj, (char *)obj, obj->get_bytes());
  *objp = new_obj;
  return new_obj;
}

}
