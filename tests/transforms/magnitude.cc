/*! @file magnitude.cc
 *  @brief Tests for SpeechFeatureExtraction::Transforms::Magnitude.
 *  @author Markovtsev Vadim <v.markovtsev@samsung.com>
 *  @version 1.0
 *
 *  @section Notes
 *  This code partially conforms to <a href="http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml">Google C++ Style Guide</a>.
 *
 *  @section Copyright
 *  Copyright 2013 Samsung R&D Institute Russia
 */


#include <gtest/gtest.h>
#include <math.h>
#include "src/transforms/magnitude.h"

using namespace SpeechFeatureExtraction::Formats;
using SpeechFeatureExtraction::BuffersBase;
using SpeechFeatureExtraction::Transforms::Magnitude;

class MagnitudeTest : public Magnitude, public testing::Test {
 public:
  BuffersBase<WindowF> Input;
  BuffersBase<WindowF> Output;
  int Size;

  virtual void SetUp() {
    Size = 378;
    Input.Initialize(1, Size);
    for (int i = 0; i < Size; i++) {
      Input[0]->Data.get()[i] = i;
    }
    WindowFormatF format(Size * 1000 / 18000, 18000);
    SetInputFormat(format);
    TypeSafeInitializeBuffers(Input, &Output);
  }
};

#define EPSILON 0.000075f

#define ASSERT_EQF(a, b) do { \
  ASSERT_GT(a + EPSILON, b); \
  ASSERT_LT(a - EPSILON, b); \
} while(0)

TEST_F(MagnitudeTest, Do) {
  TypeSafeDo(Input, &Output);
  for (int i = 0; i < Size / 2; i++) {
    float m = Output[0]->Data.get()[i];
    float re = i * 2;
    float im = i * 2 + 1;
    ASSERT_EQF(sqrtf(re * re + im * im), m);
  }
}

#include "tests/google/src/gtest_main.cc"