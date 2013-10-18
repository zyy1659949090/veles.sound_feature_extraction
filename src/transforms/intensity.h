/*! @file intensity.h
 *  @brief Sound intensity calculation.
 *  @author Markovtsev Vadim <v.markovtsev@samsung.com>
 *  @version 1.0
 *
 *  @section Notes
 *  This code partially conforms to <a href="http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml">Google C++ Style Guide</a>.
 *
 *  @section Copyright
 *  Copyright 2013 Samsung R&D Institute Russia
 */

#ifndef SRC_TRANSFORMS_INTENSITY_H_
#define SRC_TRANSFORMS_INTENSITY_H_

#include "src/formats/single_format.h"
#include "src/transforms/common.h"

namespace SoundFeatureExtraction {
namespace Transforms {

class Intensity
    : public OmpTransformBase<Formats::ArrayFormatF, Formats::SingleFormatF> {
 public:
  TRANSFORM_INTRO("Intensity", "Unnormalized sound intensity calculation.")

  OMP_TRANSFORM_PARAMETERS()

 protected:
  virtual void Do(const float* in,
                  float* out) const noexcept override;
};

}  // namespace Transforms
}  // namespace SoundFeatureExtraction
#endif  // SRC_TRANSFORMS_INTENSITY_H_
