#pragma once

#include <stddef.h>

#include "incremental.hpp"

namespace incremental {

class IncrementalGC;

class Object {
protected:

  Object **objects = nullptr;

private:
  unsigned short size;
  unsigned char generation : 7;
  unsigned char color : 1;

protected:
  unsigned char num_objects;

  void setObjPtr(Object **objs, unsigned char n_obj) {
    objects = objs;
    num_objects = n_obj;
  }

  void gcWith(IncrementalGC *gc, bool newC) {
    if (color == newC) {
      return;
    }
    color = newC;
    // moving the objects in one complete breadth-first
    // swoop adds a lot of simplicity to the algorithm
    // in the desired use-case, where most references are small
    // this removes issues of a single objects reference-list changing mid compact
    // the allocation strategy itself also prevents new allocations from ever getting GC'd
    for (unsigned char i = 0; i < num_objects; i++) {
      gc->relocate(&objects[i]);
    }

    // yield

    for (unsigned char i = 0; i < num_objects; i++) {
      // must do a proper re-iteration here since objects may have been nullified
      Object *obj = objects[i];
      if (obj != nullptr) {
        // yields are contained in the gcWith call of each child
        obj->gcWith(gc, newC);
      }
    }
  }

  friend class IncrementalGC;

protected:
  Object(unsigned short sz) : size(sz)
};

}
