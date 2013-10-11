/*! @file diffrect.cc
 *  @brief Find the difference from one sample to the next.
 *  @author Shapichev Alexey <a.shapichev@samsung.com>
 *  @version 1.0
 *
 *  @section Notes
 *  This code partially conforms to <a href="http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml">Google C++ Style Guide</a>.
 *
 *  @section Copyright
 *  Copyright 2013 Samsung R&D Institute Russia
 */

#include "src/transforms/diff.h"
#ifdef __AVX__
#include <immintrin.h>
#elif defined(__ARM_NEON__)
#include <arm_neon.h>
#endif
#include <simd/wavelet.h>
#include <src/make_unique.h>

namespace SoundFeatureExtraction {
namespace Transforms {

Diff::Diff()
    : rectify_(false), swt_(kNoSWT) {
  RegisterSetter("rectify", [&](const std::string& value) {
    rectify_ = Parse<bool>("rectify", value);
    return true;
  });
  RegisterSetter("swt", [&](const std::string& value) {
    int pv = Parse<int>("swt", value);
    if (pv < 1 && pv != kNoSWT) {
      return false;
    }
    swt_ = pv;
    return true;
  });
}

void Diff::Initialize() const noexcept {
  if (swt_ != kNoSWT) {
    int count = MaxThreadsNumber();
    swt_buffers_.resize(count);
    for (int i = 0; i < count; i++) {
      swt_buffers_[i].first = std::uniquify(mallocf(inputFormat_->Size()),
                                            std::free);
      swt_buffers_[i].second = std::uniquify(mallocf(inputFormat_->Size()),
                                             std::free);
    }
  }
}

void Diff::Do(const float* in, float* out) const noexcept {
  if (swt_ != kNoSWT) {
    bool executed = false;
    while (!executed) {
      for (auto& bufs : swt_buffers_) {
        if (bufs.mutex.try_lock()) {
          DifferentiateUsingSWT(swt_, in, inputFormat_->Size(), &bufs, out);
          bufs.mutex.unlock();
          if (rectify_) {
            Rectify(UseSimd(), out, inputFormat_->Size(), out);
          }
          executed = true;
          break;
        }
      }
    }
    return;
  }
  if (rectify_) {
    DoRectify(UseSimd(), in, inputFormat_->Size(), out);
  } else {
    Do(UseSimd(), in, inputFormat_->Size(), out);
  }
}

void Diff::Do(bool simd, const float* input, int length,
              float* output) noexcept {
  if (simd) {
#ifdef __AVX__
    for (int i = 1; i < length - 7; i += 8) {
      __m256 vec1 = _mm256_loadu_ps(input + i);
      __m256 vec2 = _mm256_load_ps(input + i - 1);
      __m256 result = _mm256_sub_ps(vec1, vec2);
      _mm256_store_ps(output + i - 1, result);
    }
    for (int i = (length & ~0x7); i < length - 1; i++) {
      output[i] = input[i + 1] - input[i];
    }
  } else {
#elif defined(__ARM_NEON__)
    for (int i = 1; i < length - 3; i += 4) {
      float32x4_t vec1 = vld1q_f32(input + i);
      float32x4_t vec2 = vld1q_f32(input + i - 1);
      float32x4_t result = vsubq_f32(vec1, vec2);
      vst1q_f32(output + i - 1, result);
    }
    for (int i = (length & ~0x3); i < length - 1; i++) {
      output[i] = input[i + 1] - input[i];
    }
  } else {
#else
  } {
#endif
    for (int i = 0; i < length - 1; i++) {
      output[i] = input[i + 1] - input[i];
    }
  }
  output[length - 1] = input[0] - input[length - 1];
}

void Diff::DoRectify(bool simd, const float* input, int length,
                     float* output) noexcept {
  if (simd) {
#ifdef __AVX__
    for (int i = 1; i < length - 7; i += 8) {
      __m256 vec1 = _mm256_loadu_ps(input + i);
      __m256 vec2 = _mm256_load_ps(input + i - 1);
      __m256 result = _mm256_sub_ps(vec1, vec2);
      result = _mm256_max_ps(result, _mm256_setzero_ps());
      _mm256_store_ps(output + i - 1, result);
    }
    for (int i = (length & ~0x7); i < length - 1; i++) {
      output[i] = input[i + 1] - input[i];
      if (output[i] < 0) {
        output[i] = 0;
      }
    }
  } else {
#elif defined(__ARM_NEON__)
    for (int i = 1; i < length - 3; i += 4) {
      float32x4_t vec1 = vld1q_f32(input + i);
      float32x4_t vec2 = vld1q_f32(input + i - 1);
      float32x4_t result = vsubq_f32(vec1, vec2);
      result = vmaxq_f32(result, vdupq_n_f32(0));
      vst1q_f32(output + i - 1, result);
    }
    for (int i = (length & ~0x3); i < length - 1; i++) {
      output[i] = input[i + 1] - input[i];
      if (output[i] < 0) {
        output[i] = 0;
      }
    }
  } else {
#else
  } {
#endif
    for (int i = 0; i < length - 1; i++) {
      output[i] = input[i + 1] - input[i];
      if (output[i] < 0) {
        output[i] = 0;
      }
    }
  }
  float last = input[0] - input[length - 1];
  output[length - 1] = last < 0? 0 : last;
}

void Diff::Rectify(bool simd, const float* input, int length,
                   float* output) noexcept {
  if (simd) {
#ifdef __AVX__
    for (int i = 0; i < length - 15; i += 16) {
      __m256 vec1 = _mm256_load_ps(input + i);
      __m256 vec2 = _mm256_load_ps(input + i + 8);
      __m256 result1 = _mm256_max_ps(vec1, _mm256_setzero_ps());
      __m256 result2 = _mm256_max_ps(vec2, _mm256_setzero_ps());
      _mm256_store_ps(output + i, result1);
      _mm256_store_ps(output + i + 8, result2);
    }
    for (int i = (length & ~0x7); i < length; i++) {
      float value = input[i];
      if (value < 0) {
        output[i] = 0;
      } else {
        output[i] = value;
      }
    }
  } else {
#elif defined(__ARM_NEON__)
    for (int i = 0; i < length - 7; i += 8) {
      float32x4_t vec1 = vld1q_f32(input + i);
      float32x4_t vec2 = vld1q_f32(input + i + 4);
      float32x4_t result1 = vmaxq_f32(vec1, vdupq_n_f32(0));
      float32x4_t result2 = vmaxq_f32(vec2, vdupq_n_f32(0));
      vst1q_f32(output + i, result1);
      vst1q_f32(output + i + 4, result2);
    }
    for (int i = (length & ~0x3); i < length; i++) {
      float value = input[i];
      if (value < 0) {
        output[i] = 0;
      } else {
        output[i] = value;
      }
    }
  } else {
#else
  } {
#endif
    for (int i = 0; i < length; i++) {
      float value = input[i];
      if (value < 0) {
        output[i] = 0;
      } else {
        output[i] = value;
      }
    }
  }
}

void Diff::DifferentiateUsingSWT(int level, const float* input, int length,
                                 SWTBuffers* auxBuffers,
                                 float* output) noexcept {
  stationary_wavelet_apply(WAVELET_TYPE_DAUBECHIES, 2, 1,
                           input, length,
                           level == 1? output : auxBuffers->second.get(),
                           auxBuffers->first.get());
  for (int i = 2; i <= level; i++) {
    stationary_wavelet_apply(
        WAVELET_TYPE_DAUBECHIES, 2, i,
        i % 2 == 0? auxBuffers->first.get() : auxBuffers->second.get(),
        length, output,
        i % 2 == 0? auxBuffers->second.get() : auxBuffers->first.get());
  }
}

REGISTER_TRANSFORM(Diff);

}  // namespace Transforms
}  // namespace SoundFeatureExtraction
