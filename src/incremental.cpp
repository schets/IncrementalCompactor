#include <incremental.hpp>
#include <string.h>
#include <object.hpp>

namespace incremental {

IncrementalGC gc(100000);

void IncrementalGC::gc_roots(Object** roots, int n_roots) {
  swap_gc_regions();
  do {
    did_write = false;
    for (int i = 0; i < n_roots; i++) {
      gc_from(roots[i]);
    }
  } while (did_write);
}

// TODO use a slightly more complex allocator
// but only compacts in regions which are empty enough
// TODO message GC when 'bad' writes happen during collection
// so that algorithm usually completes with one pass
void IncrementalGC::gc_from(Object *from) {
  if (from == nullptr) {
    return;
  }
  from->color = GCColor::grey;
  // TODO ensure fix color and re-iteration scheme
  // don't yield during object iteration - not sure if that matters?
  for (int i = 0; i < from->num_objects;) {
    // This can  be lifted past the yield
    Object **objs = from->load_objs();
    for (int j = 0; j < num_iter && i < from->num_objects; j++,i++) {
        auto rval = Object::get_forwarded_pointer(&objs[i], this);
        if (!rval.first) {
            continue;
        }
        Object *obj = rval.second;

        if (needs_gc(obj->color) &&
            in_gc_region(obj)) // roots are allocated outside of the GC region
            {
                obj->color = GCColor::grey;
                Object *new_obj = relocate(&objs[i], obj);
                if (new_obj != nullptr) {
                    obj->is_orig = false;
                    obj->set_forward_addr(get_forward_addr(new_obj));
                    obj = new_obj;
                } else {
                    // something's fucky, this is an error
                    // malloc and go with it? user provided function?
                    // run a full GC?
                    return;
                }
            }
        objs[i] = obj;
    }
    yield();
  }
  from->color = GCColor::black;

  // TODO add a distinct recoloring traversal!!!

  // problem: Current algo only yield on moves
  // could yield on a full tree traversal
  // TODO add a yield_points system
  for (int i = 0; i < from->num_objects; i++) {
      Object **objs = from->load_objs();
      auto rval = Object::get_forwarded_pointer(&objs[i], this);
      if (!rval.first) {
          continue;
      }
      Object *obj = rval.second;

      gc_from(obj);
  }
}

void IncrementalGC::recolor_from(Object *from) {
    if (from == nullptr) {
        return;
    }
    from->color = GCColor::white;
    Object **objs = from->load_objs();
    for (int i = 0; i < from->num_objects; i++) {
        // all pointers should be forwarded by this point
        if (objs[i] != nullptr) {
            objs[i]->color = GCColor::white;
        }
    }
    yield();
    objs = from->load_objs();
    for (int i = 0; i < from->num_objects; i++) {
        recolor_from(objs[i]);
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
