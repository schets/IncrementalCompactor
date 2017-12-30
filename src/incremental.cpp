#include <incremental.hpp>
#include <string.h>
#include <object.hpp>

namespace incremental {

void *IncrementalGC::allocate(size_t sz) {
  return nullptr;
}

void IncrementalGC::gc_roots(Object** roots, int n_roots) {
  for (int i = 0; i < n_roots; i++) {
    roots[i]->color = 1; // prevents all further investigation / relocation of them
  }
  // yield
  for (int i = 0; i < n_roots; i++) {
    gc_from(roots[i]);
  }
}

void IncrementalGC::gc_from(Object *from) {
  if (from == nullptr) {
    return;
  }
  Object **objs = from->load_objs();
  bool needed_gc = false;
  while (needed_gc) {
    needed_gc = false;
    for (int i = 0; i < from->num_objects; i++) {
      Object *obj = objs[i];
      // at this stage, this is an easy test for color
      // that doesn't require locating a forwarded reference
      if (!obj->is_forwarding) {
        needed_gc = true;
        Object *new_obj = relocate(&objs[i], obj);
        if (new_obj != nullptr) {
          obj->is_forwarding = true;
          obj->forward_addr = get_forward_addr(new_obj);
        } else {
          // something's fucky, this is an error
          // malloc and go with it? user provided function?
          // run a full GC?
          return;
        }
      }
    }
  }
}

Object *IncrementalGC::relocate(Object **objp, Object *obj) {

  Object *new_obj = (Object *)allocate(obj->size);
  if (new_obj == nullptr) {
    return nullptr;
  }
  if (obj->must_move) {
    obj->move_to(new_obj);
  } else {
    // suuuuper spooky b/c overwriting vptrs, moving data around
    // but classes can control this with the traits. Not a super big deal otherwise
    memcpy((char *)new_obj, (char *)obj, obj->size);
  }
  *objp = new_obj;
  return new_obj;
}

}
