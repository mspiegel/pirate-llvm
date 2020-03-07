#include "llvm/Object/PirateELFTypes.h"

namespace llvm {
namespace object {

template <> bool SafeArrayRef<char>::isNull(char c) {
  return c == '\0';
}

template <> bool SafeArrayRef<uint32_t>::isNull(uint32_t n) {
  return n == 0;
}

// TODO: specialize for Elf_Pirate_* to enable initial null checks

} // end namespace object.
} // end namespace llvm.
