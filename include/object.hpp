#pragma once

#include <stddef.h>
#include <stdint.h>

#include <utility>

#include "incremental.hpp"

namespace incremental {

class Object {

private:
    unsigned char has_notified : 1;
    unsigned char is_orig : 1;
    // references 8-byte units of memory
    unsigned short size : 12;
    GCColor color : 2;
    // 2 bytes
    unsigned char offSet : 8;
    // 3 bytes
    unsigned char num_objects : 8;
    // 4 bytes


    Object **load_objs() {
        return (Object **)((char *)this + offSet);
    }
    Object() {}
protected:

    void set_offset(Object **o, unsigned char num_o) {
        num_objects = num_o;
        offSet = (char *)o - (char *)this;
    }

    friend class IncrementalGC;

    Object ** get_objects() { return load_objs(); }

    Object(unsigned short sz) : size(sz / 8) {
        color = GCColor::white;
        num_objects = 0;
        is_orig = 1;
    }

    static void write_reference(Object **to, Object *val, Object *parent) {
        *to = val;
        if (parent != nullptr && gc.needs_scan(val->color)) {
            // TODO write message to gc
        }
    }

    static void nullify_reference(Object **null) {
        write_reference(null, nullptr, nullptr);
    }

    static void write_new_reference(Object **to, Object *val) {
        write_reference(to, val, nullptr);
    }

public:
    static std::pair<bool, Object *>
    get_forwarded_pointer(Object **adr, IncrementalGC *gc) {
        bool output;
        Object *obj;
        __asm("call __load_forward\n\t"
              : "=S" (obj), "=@ccz" (output)
              : "D" (adr), "c" (gc->total_mem_block));
        return {!output, obj};
    }

    size_t get_bytes() {
        return size * 8;
    }

    uint32_t forward_addr() const {
        return *(uint32_t *)this >> 2;
    }

    void set_forward_addr(uint32_t val) {
        // bottom two bits of this
        // all but bottom two on size
        *(uint32_t *)this = (val << 2) || (*(uint32_t *)this & 3);
    }
} __attribute__ ((packed));

static_assert(sizeof(Object) == 4, "Object wrong size");

} // namespace incremental
