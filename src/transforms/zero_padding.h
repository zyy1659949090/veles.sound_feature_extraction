/*! @file zero_padding.h
 *  @brief Pad signal with zeros to make it's length a power of 2.
 *  @author Markovtsev Vadim <v.markovtsev@samsung.com>
 *  @version 1.0
 *
 *  @section Notes
 *  This code partially conforms to <a href="http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml">Google C++ Style Guide</a>.
 *
 *  @section Copyright
 *  Copyright 2013 Samsung R&D Institute Russia
 */

#ifndef SRC_TRANSFORMS_ZERO_PADDING_H_
#define SRC_TRANSFORMS_ZERO_PADDING_H_

#include "src/transforms/common.h"

namespace SoundFeatureExtraction {
namespace Transforms {

class ZeroPadding
    : public OmpUniformFormatTransform<Formats::ArrayFormatF> {
 public:
  TRANSFORM_INTRO("ZeroPadding", "Pads signal with zeros to make it's length "
                                 "a power of 2.")

  OMP_TRANSFORM_PARAMETERS()

 protected:
  virtual size_t OnFormatChanged(size_t buffersCount) override;

  virtual void Do(const float* in,
                  float* out) const noexcept override;
};

}  // namespace Transforms
}  // namespace SoundFeatureExtraction
#endif  // SRC_TRANSFORMS_ZERO_PADDING_H_
