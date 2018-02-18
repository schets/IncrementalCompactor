#pragma once

#include <stddef.h>
#include <stdint.h>

#include "incremental.hpp"

namespace incremental {

enum class ObjManage {
  Move,
};

class IncrementalGC;

class Object {
protected:

  unsigned char must_move : 1;
private:

  unsigned char is_forwarding : 1;
  union {
    struct {
      // references 8-byte units of memory
      unsigned short size : 12;
      unsigned char offsetable : 1;
      GCColor color : 2;
      // 2 bytes
      unsigned char offSet : 8;
      // 3 bytes
      unsigned char num_objects : 8;
      // 4 bytes
    };
    uint32_t forward_addr : 30;
  };

private:


  Object **load_objs() {
    return (Object **)((char *)this + offSet);
  }
protected:

    void set_offset(Object **o, unsigned char num_o) {
      num_objects = num_o;
      offSet = (char *)o - (char *)this;
    }

  friend class IncrementalGC;

  Object ** get_objects() { return load_objs(); }

  Object(unsigned short sz) : size(sz / 8) {
    color = gc.white;
    num_objects = 0;
    is_forwarding = 0;
  }


  static void write_reference(Object **to, Object *val, Object *parent) {
    *to = val;
    if (parent != nullptr && gc.needs_scan(val->color)) {
      // only need to dirty something if the move can cause a missed ref
      parent->color = GCColor::dirty;
    }
  }

  static void nullify_reference(Object **null) {
    write_reference(null, nullptr, nullptr);
  }

  static void write_new_reference(Object **to, Object *val) {
    write_reference(to, val, nullptr);
  }

  static Object *get_forwarded_pointer(Object *obj, IncrementalGC *gc) {
    if (__builtin_expect(obj->is_forwarding, 0)) {
      return gc->get_ptr_offset(obj->forward_addr);
    } else {
      return obj;
    }

  }
public:

  size_t get_bytes() {
    return size * 8;
  }
};

}
