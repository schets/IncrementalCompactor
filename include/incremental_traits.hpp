#pragma once

namespace incremental {

template<class T>
struct IncrementalTraits {
  constexpr static bool single_reference = false;
  constexpr static bool forward_reads = false;
};

}
