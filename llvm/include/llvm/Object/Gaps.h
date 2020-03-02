//===- Gaps.h - Headers for processing GAPS information. --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Object/ELFTypes.h"
#include "llvm/Support/ErrorHandling.h"

namespace llvm {
namespace object {

using support::endianness;

template <typename ELFT>
class Elf_GAPS_Impl {
  LLVM_ELF_IMPORT_TYPES_ELFT(ELFT)
public:
  // TODO: Make these private
  ArrayRef<llvm::object::Elf_GAPS_enc<ELFT>> enclaves;
  ArrayRef<llvm::object::Elf_GAPS_cap<ELFT>> capabilities;
  ArrayRef<llvm::object::Elf_GAPS_req<ELFT>> symreqs;
  ArrayRef<uint32_t> captab;
private:
  ArrayRef<char> strtab;

public:
  Elf_GAPS_Impl(void) {}
  Elf_GAPS_Impl(const Elf_GAPS_Impl &other) = delete;

  // Set string table, with fatal error when checks fail.
  void setStrTab(ArrayRef<char> p) {
    if (this->strtab.data() != 0) {
      report_fatal_error("Multiple .gaps.strtab entries.");
    }
    // Check to ensure buffer ends with `0` so we are guaranteed
    // string table entries are null-terminated.
    if (p.size() > 0 && p[p.size()-1] != 0) {
      report_fatal_error("String table must end with 0.");
    }
    this->strtab = p;
  }

  void checkStrtabInitialized(void) const {
    if (strtab.data() == 0) {
      report_fatal_error("String table uninitialized.");
    }
  }

  // Get string table entry or throw fatal error if offset is invalid.
  const char* getStrtabEntry(typename ELFT::Addr offset) const {
    if (offset >= strtab.size()) {
      report_fatal_error("Illegal string table index.");
    }
    return strtab.data() + offset;
  }

  // Get null terminated list of capabilities or null pointer if index is invalid.
  ArrayRef<uint32_t> getCapabilityIndices(Elf_Word idx) const {
    if (idx >= captab.size()) return ArrayRef<uint32_t>();

    const uint32_t* s = captab.data() + idx;
    const uint32_t* p = s;
    while (*p != 0) ++p;
    return ArrayRef<uint32_t>(s,p-s);
  }

  void getSuppliedCaps(typename ELFT::Word offset, std::vector<StringRef> &out) const {
    out.clear();
    for (uint32_t i = offset; captab[i]; ++i)
      for (uint32_t j = captab[i]; j; j = capabilities[j].cap_parent)
        out.emplace_back(getStrtabEntry(capabilities[j].cap_name));
  }

  void getRequiredCaps(typename ELFT::Word offset, std::vector<StringRef> &out) const {
    out.clear();
    for (uint32_t i = offset; captab[i]; ++i)
      out.emplace_back(getStrtabEntry(capabilities[captab[i]].cap_name));
  }
};

} // end namespace object.
} // end namespace llvm.
