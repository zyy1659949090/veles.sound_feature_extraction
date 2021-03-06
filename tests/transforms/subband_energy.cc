/*! @file subband_energy.cc
 *  @brief Tests for sound_feature_extraction::transforms::SubbandEnergy.
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

#include "src/transforms/subband_energy.h"
#include "tests/transforms/transform_test.h"

using sound_feature_extraction::formats::ArrayFormatF;
using sound_feature_extraction::BuffersBase;
using sound_feature_extraction::transforms::SubbandEnergy;

class SubbandEnergyTest : public TransformTest<SubbandEnergy> {
 public:
  int Size;

  virtual void SetUp() {
    set_tree({ 3, 3, 2, 2, 3, 3 });
    Size = 512;
    SetUpTransform(1, Size, 16000);
    for (int i = 0; i < Size; i++) {
      (*Input)[0][i] = i + 1;
    }
  }
};

#define EPSILON 0.005f

#define ASSERT_EQF(a, b) ASSERT_NEAR(a, b, EPSILON)

float SumOfSquares(int max) {
  return max * (max + 1) * (2 * max + 1.0f) / 6;
}

TEST_F(SubbandEnergyTest, Do) {
  ASSERT_EQ(6U, output_format_->Size());
  Do((*Input)[0], (*Output)[0]);
  float* output = (*Output)[0];
  int quarter = Size / 8;
  ASSERT_EQF(SumOfSquares(quarter), output[0]);
  ASSERT_EQF(SumOfSquares(quarter * 2) - SumOfSquares(quarter), output[1]);
  ASSERT_EQF(SumOfSquares(quarter * 4) - SumOfSquares(quarter * 2),
             output[2]);
}
