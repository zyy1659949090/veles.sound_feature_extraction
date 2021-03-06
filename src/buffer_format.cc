/*! @file buffer_format.cc
 *  @brief Buffer format interface class.
 *  @author Markovtsev Vadim <v.markovtsev@samsung.com>
 *  @version 1.0
 *
 *  @section Notes
 *  This code partially conforms to <a href="http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml">Google C++ Style Guide</a>.
 *
 *  @section Copyright
 *  Copyright © 2013 Samsung R&D Institute Russia
 *
 *  @section License
 *  Licensed to the Apache Software Foundation (ASF) under one
 *  or more contributor license agreements.  See the NOTICE file
 *  distributed with this work for additional information
 *  regarding copyright ownership.  The ASF licenses this file
 *  to you under the Apache License, Version 2.0 (the
 *  "License"); you may not use this file except in compliance
 *  with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing,
 *  software distributed under the License is distributed on an
 *  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *  KIND, either express or implied.  See the License for the
 *  specific language governing permissions and limitations
 *  under the License.
 */

#include "src/buffer_format.h"
#include <cassert>

namespace sound_feature_extraction {

constexpr const char* BufferFormat::kIdentityID;

BufferFormat::BufferFormat(const std::string& id, int samplingRate)
    : id_(id),
      samplingRate_(samplingRate) {
  ValidateSamplingRate(samplingRate_);
}

BufferFormat::BufferFormat(const std::string& id)
    : id_(id),
      samplingRate_(0) {
}

const std::string& BufferFormat::Id() const noexcept {
  return id_;
}

bool BufferFormat::operator==(const BufferFormat& other) const noexcept {
  return id_ == kIdentityID || other.id_ == kIdentityID || id_ == other.id_;
}

bool BufferFormat::operator!=(const BufferFormat& other) const noexcept {
  return id_ != other.id_ && id_ != kIdentityID && other.id_ != kIdentityID;
}

void BufferFormat::CopySourceDetailsFrom(const BufferFormat& format) noexcept {
  SetSamplingRate(format.SamplingRate());
}

size_t BufferFormat::SizeInBytes() const noexcept {
  return Aligned(UnalignedSizeInBytes());
}

int BufferFormat::SamplingRate() const noexcept {
  assert(samplingRate_ > 0);
  return samplingRate_;
}

void BufferFormat::SetSamplingRate(int value) {
  ValidateSamplingRate(value);
  samplingRate_ = value;
}

void BufferFormat::ValidateSamplingRate(int value) {
  if (value < kMinSamplingRate || value > kMaxSamplingRate) {
    throw formats::InvalidSamplingRateException(value);
  }
}

IdentityFormat::IdentityFormat() : BufferFormat(kIdentityID) {
}

IdentityFormat::IdentityFormat(int samplingRate)
    : BufferFormat(kIdentityID, samplingRate) {
}

size_t IdentityFormat::UnalignedSizeInBytes() const noexcept {
  return 0;
}

void IdentityFormat::Validate(const Buffers&) const {
}

std::string IdentityFormat::Dump(const Buffers&, size_t) const {
  return "<empty>";
}

std::string IdentityFormat::ToString() const noexcept {
  return Id();
}

}  // namespace sound_feature_extraction
