/*! @file beat.cc
 *  @brief Beat detection.
 *  @author Markovtsev Vadim <v.markovtsev@samsung.com>, Shapichev Alexey <a.shapichev@samsung.com>
 *  @version 1.0
 *
 *  @section Notes
 *  This code partially conforms to <a href="http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml">Google C++ Style Guide</a>.
 *
 *  @section Copyright
 *  Copyright 2013 Samsung R&D Institute Russia
 */

#include "src/transforms/beat.h"
#include <algorithm>
#include <cmath>
#include <simd/detect_peaks.h>
#include "src/make_unique.h"
#include "src/primitives/energy.h"

namespace SoundFeatureExtraction {
namespace Transforms {

Beat::Beat()
    : buffer_(nullptr, std::free),
      bands_(1),
      pulses_(kDefaultPulses),
      min_bpm_(kDefaultMinBPM),
      max_bpm_(kDefaultMaxBPM),
      step1_(kDefaultResolution1),
      step2_(kDefaultResolution2),
      peaks_(kDefaultPeaks),
      debug_(false) {
  RegisterSetter("bands", [&](const std::string& value) {
    int iv = Parse<int>("bands", value);
    if (iv < 1) {
      return false;
    }
    bands_ = iv;
    return true;
  });
  RegisterSetter("pulses", [&](const std::string& value) {
    int iv = Parse<int>("pulses", value);
    if (iv < 1) {
      return false;
    }
    pulses_ = iv;
    return true;
  });
  RegisterSetter("min_bpm", [&](const std::string& value) {
    int iv = Parse<int>("min_bpm", value);
    if (iv < 1 || iv > 600) {
      return false;
    }
    min_bpm_ = iv;
    return true;
  });
  RegisterSetter("max_bpm", [&](const std::string& value) {
    int iv = Parse<int>("max_bpm", value);
    if (iv < 1 || iv > 600) {
      return false;
    }
    max_bpm_ = iv;
    return true;
  });
  RegisterSetter("resolution1", [&](const std::string& value) {
    int fv = Parse<float>("resolution1", value);
    if (fv <= 0) {
      return false;
    }
    step1_ = fv;
    return true;
  });
  RegisterSetter("resolution2", [&](const std::string& value) {
    int fv = Parse<float>("resolution2", value);
    if (fv <= 0) {
      return false;
    }
    step2_ = fv;
    return true;
  });
  RegisterSetter("max_peaks", [&](const std::string& value) {
    int iv = Parse<int>("max_peaks", value);
    if (iv < 1 || iv > 10) {
      return false;
    }
    peaks_ = iv;
    return true;
  });
  RegisterSetter("debug", [&](const std::string& value) {
    debug_ = Parse<bool>("debug", value);
    return true;
  });
}

size_t Beat::PulsesLength(int pulses_count, int period) noexcept {
  return (pulses_count - 1) * period + 1;
}

size_t Beat::OnInputFormatChanged(size_t buffersCount) {
  outputFormat_->SetSize(peaks_);
  return buffersCount / bands_;
}

void Beat::Initialize() const {
  float max_period = floorf(60 * inputFormat_->SamplingRate() / min_bpm_);
  size_t max_pulses_length = PulsesLength(pulses_, max_period);
  buffer_ = std::uniquify(mallocf(
      (this->inputFormat_->Size() + max_pulses_length - 1) * sizeof(float)),
      std::free);
}

void Beat::CombConvolve(const float* in, size_t size, int pulses,
                        int period, float* out) noexcept {
  int pp = PulsesLength(pulses, period);
  for (int i = 0; i < pp - 1; i++) {
    float res = 0;
    for (int j = 0; j <= i; j += period) {
      res += in[i - j];
    }
    out[i] = res;
  }

  for (int i = pp - 1; i < static_cast<int>(size); i++) {
    float res = 0;
    for (int j = 0; j < pp; j += period) {
      res += in[i - j];
    }
    out[i] = res;
  }

  for (int i = size; i < static_cast<int>(size + pp - 1); i++) {
    float res = 0;
    for (int j = ((i - size) / period + 1) * period; j < pp; j += period) {
      res += in[i - j];
    }
    out[i] = res;
  }
}

void Beat::Do(const BuffersBase<float*>& in,
              BuffersBase<Formats::FixedArray<2>*>* out)
    const noexcept {
  for (size_t ini = 0; ini < in.Count(); ini += bands_) {
    std::vector<float> energies;

    // First pass - rough peaks estimation
    CalculateBeatEnergies(in, ini, min_bpm_, max_bpm_, step1_, &energies);

    // Output the energies for the reference
    if (debug_) {
      std::string dump("----Energies----\n");
      for (size_t i = 0; i < energies.size(); i++) {
        dump += std::to_string(energies[i]) + "    ";
        if (i % 10 == 0 && i > 0) {
          dump += '\n';
        }
      }
      INF("%s\n----\n", dump.c_str());
    }

    // Find maximums and sort them
    ExtremumPoint* results;
    size_t found_peaks_count;
    detect_peaks(UseSimd(), energies.data(), energies.size(),
                 kExtremumTypeMaximum, &results, &found_peaks_count);
    if (results == nullptr) {
      for (int i = 0; i < peaks_; i++) {
        (*out)[ini / bands_][i][0] = 0;
        (*out)[ini / bands_][i][1] = 0;
      }
      continue;
    }
    std::sort(results, results + found_peaks_count,
              [](const ExtremumPoint& f, const ExtremumPoint& s) {
                return f.value > s.value;
              });
    int rcount = static_cast<int>(found_peaks_count) > peaks_?
        peaks_ : found_peaks_count;
    std::sort(results, results + rcount,
              [](const ExtremumPoint& f, const ExtremumPoint& s) {
                return f.position < s.position;
              });

    // Second pass - increase peaks precision
    for (int pind = 0; pind < rcount; pind++) {
      CalculateBeatEnergies(in, ini,
                            min_bpm_ + (results[pind].position - 1) * step1_,
                            min_bpm_ + (results[pind].position + 1) * step1_,
                            step2_, &energies, &(*out)[ini / bands_][pind][0],
                            &(*out)[ini / bands_][pind][1]);
    }
    for (int pind = rcount; pind < peaks_; pind++) {
      (*out)[ini / bands_][pind][0] = 0;
      (*out)[ini / bands_][pind][1] = 0;
    }
    free(results);
  }
}

void Beat::CalculateBeatEnergies(const BuffersBase<float*>& in, size_t inIndex,
                                 float min_bpm, float max_bpm, float step,
                                 std::vector<float>* energies,
                                 float* max_energy_bpm_found,
                                 float* max_energy_found) const noexcept {
  auto size = this->inputFormat_->Size();
  auto buffer = buffer_.get();
  int search_size = floorf((max_bpm - min_bpm) / step);
  energies->resize(search_size);
  float max_energy = 0;
  float max_energy_bpm = min_bpm;
  for (int i = 0; i < search_size; i++) {
    float bpm = min_bpm + step * i;
    // 60 is the number of seconds in one minute
    int period = floorf(60 * inputFormat_->SamplingRate() / bpm);
    float current_energy = 0;
    size_t conv_length = size + PulsesLength(pulses_, period) - 1;
    for (size_t i = inIndex; i < inIndex + bands_ && i < in.Count(); i++) {
      CombConvolve(in[i], size, pulses_, period, buffer);
      current_energy += calculate_energy(Beat::UseSimd(), buffer,
                                         conv_length) * conv_length;
    }
    (*energies)[i] = current_energy;
    if (current_energy > max_energy) {
      max_energy = current_energy;
      max_energy_bpm = bpm;
    }
  }
  if (max_energy_bpm_found != nullptr) {
    *max_energy_bpm_found = max_energy_bpm;
  }
  if (max_energy_found != nullptr) {
    *max_energy_found = max_energy;
  }
}

REGISTER_TRANSFORM(Beat);

}  // namespace Transforms
}  // namespace SoundFeatureExtraction
