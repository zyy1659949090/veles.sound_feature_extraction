/*! @file energy.cc
 *  @brief Sound energy calculation.
 *  @author Markovtsev Vadim <v.markovtsev@samsung.com>
 *  @version 1.0
 *
 *  @section Notes
 *  This code partially conforms to <a href="http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml">Google C++ Style Guide</a>.
 *
 *  @section Copyright
 *  Copyright 2013 Samsung R&D Institute Russia
 */

#include "src/transforms/energy.h"
#include <cmath>
#include "src/primitives/energy.h"

namespace sound_feature_extraction {
namespace transforms {

void Energy::Do(const float* in, float* out) const noexcept {
  int length = input_format_->Size();
  *out = calculate_energy(use_simd(), true, in, length);
}

REGISTER_TRANSFORM(Energy);

}  // namespace transforms
}  // namespace sound_feature_extraction
