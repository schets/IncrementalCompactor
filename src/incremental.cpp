#include <incremental.hpp>
#include <object.hpp>

namespace incremental {

void *IncrementalGC::allocate(size_t sz) {
  return nullptr;
}

void IncrementalGC::gcRoots(Object** roots, int n_roots) {
  for (int i = 0; i < n_roots; i++) {
    roots[i].color = 1; // prevents all further investigation / relocation of them
  }
  // yield
  for (int i = 0; i < n_roots; i++) {
    gcFrom(roots[i]);
  }
}

void IncrementalGC::gcFrom(Object *from) {
  bool needed_gc = false;
  while (needed_gc) {
    for (int i = 0; i < n_roots; i++) {
      // at this stage, this is an easy test for color
      // that doesn't require locating a forwarded reference
      if (!from->is_forwarding) {
        Object *new_obj = reloate(from);
        if (new_obj != nullptr) {
          from->is_forwarding = true;
          from->forward_addr = get_forward_addr(new_obj);
        } else {
          // something's fucky
        }
      }
    }
  }
}

Object *IncrementalGC::relocate(Object **objp) {
  Object *obj = *objp;
  /*
  void *newObj = allocate(obj->size);
  if (newObj == nullptr) {
    return;
  }
  if (obj->mustMove) {
    obj->moveTo(newObj);
  } else {
    memcpy(newObj, obj, obj->size);
  }
  completeAlloc(obj);
  *objp = newObj;
  */
}

}
