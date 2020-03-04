//===- PirateELFTypes.h - Pirate types for ELF ------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_OBJECT_PIRATEELFTYPES_H
#define LLVM_OBJECT_PIRATEELFTYPES_H

#include "llvm/Object/ELFTypes.h"

namespace llvm {
namespace object {

template <class ELFT> struct Elf_Pirate_enc {
  LLVM_ELF_IMPORT_TYPES_ELFT(ELFT)
  Elf_Addr enc_name;
  Elf_Word enc_cap;
  Elf_Half enc_main;
  Elf_Half enc_padding;
};

template <class ELFT> struct Elf_Pirate_req {
  LLVM_ELF_IMPORT_TYPES_ELFT(ELFT)
  Elf_Word req_cap;
  Elf_Word req_enc;
  Elf_Half req_sym;
  Elf_Half req_padding;
};

template <class ELFT> struct Elf_Pirate_cap {
  LLVM_ELF_IMPORT_TYPES_ELFT(ELFT)
  Elf_Addr cap_name;
  Elf_Word cap_parent;
  Elf_Word cap_padding;
};

template <class ELFT> struct Elf_Pirate_res {
  LLVM_ELF_IMPORT_TYPES_ELFT(ELFT)
  Elf_Addr gr_name;
  Elf_Addr gr_obj;
  Elf_Addr gr_params;
  Elf_Word gr_size;
  Elf_Half gr_align;
  Elf_Half gr_sym;
};

template <typename T> class SafeArrayRef : public ArrayRef<T> {
private:
  StringRef name = "unnamed SafeArrayRef";
  bool null_initial = false;
  bool null_terminated = false;

  // This can be overridden to enable null-checking for a type
  bool isNull(T) { return true; }

public:
  SafeArrayRef(StringRef n, bool i = false, bool t = false)
    : name(n), null_initial(i), null_terminated(t) {}

  // TODO: Overload constructors to prevent unintentional unsafe initial
  //       state

  // Assign the array once after checking that the source array is non-null
  // and performing checks for null-termination and initiation if requested
  SafeArrayRef<T> &operator=(const ArrayRef<T> &src) {
    if (this->data() != nullptr)
      report_fatal_error(name + " is multiply defined");
    if (src.data() == nullptr)
      report_fatal_error(name + " contains null data");
    if (null_initial && (src.size() < 1 || !isNull(src.front())))
      report_fatal_error(name + " does not have a null initial element");
    if (null_terminated && (src.size() < 1 || !isNull(src.back())))
      report_fatal_error(name + " does not have a null final element");

    this->ArrayRef<T>::operator=(src);
    return *this;
  }

  // Access the array after checking that the array has been initialized and
  // that the index is in bounds
  const T &operator[](size_t ix) const {
    if (this->data() == nullptr)
      report_fatal_error("uninitialized access to " + name);
    if (ix >= this->size())
      report_fatal_error("out of bounds " + name + " access");
    return ArrayRef<T>::operator[](ix);
  }
};

template <> bool SafeArrayRef<char>::isNull(char c);

template <> bool SafeArrayRef<uint32_t>::isNull(uint32_t n);

template <typename ELFT> struct Elf_Pirate_Impl {
  SafeArrayRef<Elf_Pirate_enc<ELFT>> enclaves;
  SafeArrayRef<Elf_Pirate_cap<ELFT>> capabilities;
  SafeArrayRef<Elf_Pirate_req<ELFT>> symreqs;
  SafeArrayRef<uint32_t> captab;
  SafeArrayRef<char> strtab;

  Elf_Pirate_Impl() :
    enclaves(".pirate.enclaves", true),
    capabilities(".pirate.capabilities", true),
    symreqs(".pirate.symreqs"),
    captab(".pirate.captab", true, true),
    strtab(".pirate.strtab", true, true)
    {};

  StringRef getStrtabEntry(typename ELFT::Addr offset) const {
    return { &strtab[offset] };
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

#endif
