// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under both the GPLv2 (found in the
// COPYING file in the root directory) and Apache 2.0 License
// (found in the LICENSE.Apache file in the root directory).
#include <cassert>
#include <cstdint>
#include <cstring>
#include <map>
#include <new>
#include <utility>
#include <cstdio>
#include "util/cast_util.h"
namespace rocksdb {
constexpr uint64_t kInvalidBlobFileNumber = 0;
struct MutableScalars {
  // Whether there are invalid new files or invalid deletions on levels larger
  // than num_levels_.
  bool has_invalid_levels_ = false;

  // The fields below are only meaningful when track_found_and_missing_files_ is
  // enabled.

  // The highest file number among all missing blob files, or
  // kInvalidBlobFileNumber if none are missing. Useful to check whether a
  // complete Version is available.
  uint64_t missing_blob_files_high_ = kInvalidBlobFileNumber;
  // Cached result of the last validity check: true if all the files making up
  // the Version can be found. Or, when allow_incomplete_valid_version_ is true
  // and the version was never edited in an atomic group, true if only a suffix
  // of L0 SST files and their associated blob files are missing.
  bool valid_version_available_ = false;
  // True if the version was ever edited in an atomic group.
  bool edited_in_atomic_group_ = false;
  // True if the Version was updated since the last validity check, so the
  // cached `valid_version_available_` must be recomputed on the next check
  // rather than reused.
  bool version_updated_since_last_check_ = false;
};


struct Undo {
 MutableScalars scalars;
 bool rolled_back = false;
 std::map<int, bool> entries;
};
void Copy(MutableScalars& a, const MutableScalars& b) { a = b; }
void Swap(MutableScalars& a, MutableScalars& b) { std::swap(a, b); }
}
int main() {
 using namespace rocksdb;
 alignas(MutableScalars) unsigned char storage[sizeof(MutableScalars)];
 for (unsigned byte = 0; byte != 256; ++byte) {
  std::memset(storage, byte, sizeof storage);
  auto* live = new (storage) MutableScalars;
  Undo undo;
  Copy(undo.scalars, *live);
  assert(!undo.rolled_back);
  Swap(*live, undo.scalars);
  assert(!undo.rolled_back);
  undo.rolled_back = true;
  Swap(*live, undo.scalars);
  assert(undo.rolled_back);
  live->~MutableScalars();
 }
 std::puts("scalar copy and swap preserve the derived undo flag");
}
