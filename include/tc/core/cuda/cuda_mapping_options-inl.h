/**
 * Copyright (c) 2017-present, Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

namespace tc {

// CudaDimView & CudaDim
//
CudaDim::CudaDim(std::vector<uint64_t> il) : ownedProto_(), view(ownedProto_) {
  CHECK_GT(il.size(), 0) << "list of values in CudaDimView must be non-empty";
  CHECK_LE(il.size(), 3) << "at most 3 values allowed in CudaDimView";

  switch (il.size()) {
    case 3:
      ownedProto_.set_z(*(il.begin() + 2));
    case 2:
      ownedProto_.set_y(*(il.begin() + 1));
    case 1:
      ownedProto_.set_x(*il.begin());
      break;
    default:
      CHECK(false) << "unreachable";
  }
}

CudaDim::CudaDim(std::initializer_list<uint64_t> il)
    : CudaDim(std::vector<uint64_t>(il)) {}

CudaDim::CudaDim(uint64_t x, uint64_t y, uint64_t z)
    : ownedProto_(), view(ownedProto_) {
  ownedProto_.set_x(x);
  if (y != CudaDimView::defaultDim || z != CudaDimView::defaultDim) {
    ownedProto_.set_y(y);
  }
  if (z != CudaDimView::defaultDim) {
    ownedProto_.set_z(z);
  }
}

size_t CudaDimView::size() const {
  CHECK(!(!proto.has_y() && proto.has_z())) << "CudaDimView has z but not y";

  if (proto.has_z() && proto.has_y()) {
    return 3;
  } else if (proto.has_y()) {
    return 2;
  }
  return 1;
}

std::vector<uint64_t> CudaDimView::extractVector() const {
  CHECK(!(!proto.has_y() && proto.has_z())) << "CudaDimView has z but not y";

  std::vector<uint64_t> result;
  result.push_back(proto.x());
  if (proto.has_y()) {
    result.push_back(proto.y());
  }
  if (proto.has_z()) {
    result.push_back(proto.z());
  }
  return result;
}

std::array<uint64_t, 3> CudaDimView::extractDefaultedArray() const {
  std::array<uint64_t, 3> arr{CudaDimView::defaultDim,
                              CudaDimView::defaultDim,
                              CudaDimView::defaultDim};
  auto v = extractVector();
  CHECK_LE(v.size(), 3);
  std::copy(v.begin(), v.end(), arr.begin());
  return arr;
}

ValueAccessor<uint64_t> CudaDimView::operator[](size_t i) {
  CHECK_LT(i, 3) << "index overflow";
  if (i == 0) {
    return ValueAccessor<uint64_t>(
        [this](uint64_t u) { this->proto.set_x(u); },
        [this]() { return this->proto.x(); });
  } else if (i == 1) {
    return ValueAccessor<uint64_t>(
        [this](uint64_t u) { this->proto.set_y(u); },
        [this]() {
          return this->proto.has_y() ? this->proto.y()
                                     : CudaDimView::defaultDim;
        });
  } else {
    return ValueAccessor<uint64_t>(
        [this](uint64_t u) { this->proto.set_z(u); },
        [this]() {
          return this->proto.has_z() ? this->proto.z()
                                     : CudaDimView::defaultDim;
        });
  }
}

uint64_t CudaDimView::operator[](size_t i) const {
  CHECK_LT(i, 3) << "index overflow";
  if (i == 0) {
    return proto.x();
  } else if (i == 1) {
    return proto.has_y() ? proto.y() : CudaDimView::defaultDim;
  } else {
    return proto.has_z() ? proto.z() : CudaDimView::defaultDim;
  }
}

CudaDimView& CudaDimView::operator=(const CudaDimView& view) {
  proto = view.proto;
  return *this;
}

bool CudaDimView::operator==(const CudaDimView& view) const {
  return proto.SerializeAsString() == view.proto.SerializeAsString();
}

bool CudaDimView::operator!=(const CudaDimView& view) const {
  return !(*this == view);
}

//
// CudaMappingOptions
//
CudaMappingOptions::CudaMappingOptions()
    : ownedProto_(),
      generic(*ownedProto_.mutable_generic_mapping_options()),
      block(*ownedProto_.mutable_block()),
      grid(*ownedProto_.mutable_grid()) {}

CudaMappingOptions::CudaMappingOptions(const CudaMappingOptions& options)
    : ownedProto_(options.ownedProto_),
      generic(*ownedProto_.mutable_generic_mapping_options()),
      block(*ownedProto_.mutable_block()),
      grid(*ownedProto_.mutable_grid()) {}

CudaMappingOptions::CudaMappingOptions(const CudaMappingOptionsProto& buf)
    : ownedProto_(buf),
      generic(*ownedProto_.mutable_generic_mapping_options()),
      block(*ownedProto_.mutable_block()),
      grid(*ownedProto_.mutable_grid()) {}

CudaMappingOptions::CudaMappingOptions(const std::string& str)
    : CudaMappingOptions() {
  generic = MappingOptionsView(*ownedProto_.mutable_generic_mapping_options());
  block = CudaDimView(*ownedProto_.mutable_block());
  grid = CudaDimView(*ownedProto_.mutable_grid());
  bool parsed = ownedProto_.ParseFromString(str);
  CHECK(parsed) << "could not parse protobuf string";
}

CudaMappingOptions& CudaMappingOptions::operator=(
    const CudaMappingOptions& options) {
  ownedProto_ = options.ownedProto_; // views already point to the proper place
  return *this;
}

bool CudaMappingOptions::operator==(const CudaMappingOptions& options) const {
  return ownedProto_.SerializeAsString() ==
      options.ownedProto_.SerializeAsString();
}

bool CudaMappingOptions::operator!=(const CudaMappingOptions& options) const {
  return ownedProto_.SerializeAsString() !=
      options.ownedProto_.SerializeAsString();
}

CudaMappingOptions& CudaMappingOptions::mapToThreads(
    std::initializer_list<uint64_t> threads) {
  block = CudaDim(threads).view; // tmp CudaDim, copy, delete
  return *this;
}

CudaMappingOptions&
CudaMappingOptions::mapToThreads(uint64_t x, uint64_t y, uint64_t z) {
  block = CudaDim(x, y, z).view; // tmp CudaDim, copy, delete
  return *this;
}

CudaMappingOptions& CudaMappingOptions::mapToThreads(
    const std::vector<uint64_t>& threads) {
  CHECK_GT(threads.size(), 0) << "expected at least one thread size";
  CHECK_LE(threads.size(), 3) << "expected at most three thread sizes";

  uint64_t x = threads[0];
  uint64_t y = threads.size() > 1 ? threads[1] : CudaDimView::defaultDim;
  uint64_t z = threads.size() > 2 ? threads[2] : CudaDimView::defaultDim;
  block = CudaDim(x, y, z).view; // tmp CudaDim, copy, delete
  return *this;
}

CudaMappingOptions& CudaMappingOptions::mapToBlocks(
    std::initializer_list<uint64_t> blocks) {
  grid = CudaDim(blocks).view; // tmp CudaDim, copy, delete
  return *this;
}

CudaMappingOptions&
CudaMappingOptions::mapToBlocks(uint64_t x, uint64_t y, uint64_t z) {
  grid = CudaDim(x, y, z).view; // tmp CudaDim, copy, delete
  return *this;
}

CudaMappingOptions& CudaMappingOptions::mapToBlocks(
    const std::vector<uint64_t>& blocks) {
  CHECK_GT(blocks.size(), 0) << "expected at least one thread size";
  CHECK_LE(blocks.size(), 3) << "expected at most three thread sizes";

  uint64_t x = blocks[0];
  uint64_t y = blocks.size() > 1 ? blocks[1] : CudaDimView::defaultDim;
  uint64_t z = blocks.size() > 2 ? blocks[2] : CudaDimView::defaultDim;
  grid = CudaDim(x, y, z).view; // tmp CudaDim, copy, delete
  return *this;
}

CudaMappingOptions& CudaMappingOptions::genericMappingOptions(
    const MappingOptions& options) {
  *(ownedProto_.mutable_generic_mapping_options()) = options.view.proto;
  return *this;
}

CudaMappingOptions& CudaMappingOptions::useSharedMemory(bool b) {
  ownedProto_.set_use_shared_memory(b);
  return *this;
}

CudaMappingOptions& CudaMappingOptions::usePrivateMemory(bool b) {
  ownedProto_.set_use_private_memory(b);
  return *this;
}

CudaMappingOptions& CudaMappingOptions::maxSharedMemory(uint64_t size) {
  ownedProto_.set_max_shared_memory(size);
  return *this;
}

CudaMappingOptions& CudaMappingOptions::unrollCopyShared(bool b) {
  ownedProto_.set_unroll_copy_shared(b);
  return *this;
}

} // namespace tc
