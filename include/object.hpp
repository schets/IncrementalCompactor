#pragma once

#include <stddef.h>
#include <stdint.h>

#include "incremental.hpp"

namespace incremental {

class IncrementalGC;

class Object {
protected:

private:

  // the GC must always load forwarding addresses
  unsigned char is_forwarding : 1;
  union {
    struct {
      // references 8-byte units of memory
      unsigned short size : 12;
      unsigned char offsetable : 1;
      unsigned char color : 2;
      // 2 bytes
      unsigned char offSet : 8;
      // 3 bytes
      unsigned char num_objects : 8;
      // 4 bytes
    };
    uint32_t forward_addr : 31;
  };

private:

  static Object *get_forwarded_pointer(Object *obj, IncrementalGC *gc) {
    if (obj->is_forwarding) {
      return gc->get_ptr_offset(obj->forward_addr);
    } else {
      return obj;
    }
  }

  Object **loadObjs() {
    if (offsetable) {
      return (Object **)((char *)this + offSet);
    } else {
      return getObjects();
    }
  }
protected:

  friend class IncrementalGC;

  virtual Object ** getObjects() { return nullptr; }

protected:
  virtual ~Object() {}
  Object(unsigned short sz) : size(sz / 8) {}
};

}
