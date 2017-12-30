#include <incremental.hpp>
#include <object.hpp>

namespace incremental {

void *IncrementalGC::allocate(std::size_t sz) {
  return nullptr;
}

void IncrementalGC::gcRoots(Object** roots, int n_roots) {
  for (int i = 0; i < n_roots; i++) {
    roots[i]->gcWith(this, newC);
  }
}

}
