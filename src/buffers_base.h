/*! @file buffers_base.h
 *  @brief Base class for buffer formats which implements BufferFormat.
 *  @author Markovtsev Vadim <v.markovtsev@samsung.com>
 *  @version 1.0
 *
 *  @section Notes
 *  This code partially conforms to <a href="http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml">Google C++ Style Guide</a>.
 *
 *  @section Copyright
 *  Copyright 2013 Samsung R&D Institute Russia
 */

#ifndef SRC_BUFFERS_BASE_H_
#define SRC_BUFFERS_BASE_H_

#include <assert.h>
#include "src/config.h"
#include "src/buffer_format.h"
#include "src/buffers.h"
#include "src/demangle.h"
#include "src/logger.h"

namespace SoundFeatureExtraction {

class InvalidBuffersException : public ExceptionBase {
 public:
  InvalidBuffersException(const std::string& format,
                          size_t index, const std::string& value)
  : ExceptionBase("Buffers[" + std::to_string(index) +
                  "] is invalid (" + value + "). Format is " +
                  format + ".") {}
};

template <typename T>
class BuffersBase;

template <typename T>
class BufferFormatBase : public BufferFormat {
 public:
  typedef T BufferType;

  BufferFormatBase() noexcept
      : BufferFormat(CutNamespaces(std::demangle(typeid(T).name()))),
        samplingRate_(0),
        incompatible_(false) {
  }

  BufferFormatBase(int samplingRate)
      : BufferFormat(CutNamespaces(std::demangle(typeid(T).name()))),
        samplingRate_(samplingRate),
        incompatible_(false) {
    ValidateSamplingRate(samplingRate_);
  }

  virtual std::function<void(void*)> Destructor()  // NOLINT(*)
      const noexcept override final {
    return [](void* ptr) {  // NOLINT(whitespace/braces)
      auto instance = reinterpret_cast<T*>(ptr);
      delete instance;
    };
  }

  virtual bool MustReallocate(const BufferFormat& other)
      const noexcept override final {
    if (*this != other || Incompatible()) {
      return true;
    }
    return MustReallocate(
        reinterpret_cast<const BufferFormatBase<T>&>(other));
  }

  virtual size_t PayloadSizeInBytes() const noexcept override {
    return 0;
  }

  virtual const void* PayloadPointer(const void* buffer)
      const noexcept override final {
    return PayloadPointer(*reinterpret_cast<const T*>(buffer));
  }

  virtual void Validate(const Buffers& buffers) const override final {
    if (*this != *buffers.Format()) {
      throw InvalidFormatException(Id(), buffers.Format()->Id());
    }
    Validate(reinterpret_cast<const BuffersBase<T>&>(buffers));
  }

  virtual std::string Dump(const Buffers& buffers)
      const noexcept override final {
    if (*this != *buffers.Format()) {
      throw InvalidFormatException(Id(), buffers.Format()->Id());
    }
    std::string ret("Buffers count: ");
    ret += std::to_string(buffers.Size());
    ret += "\n";
    ret += Dump(reinterpret_cast<const BuffersBase<T>&>(buffers));
    return std::move(ret);
  }

  virtual int SamplingRate() const noexcept override final {
    assert(samplingRate_ > 0);
    return samplingRate_;
  }

  virtual void SetSamplingRate(int value) override final {
    ValidateSamplingRate(value);
    samplingRate_ = value;
  }

  bool Incompatible() const noexcept {
    return incompatible_;
  }

  void SetIncompatible(bool value) noexcept {
    incompatible_ = value;
  }

 protected:
  std::string CutNamespaces(std::string&& str) {
    return str.substr(str.find_last_of(':') + 1, std::string::npos);
  }

  virtual bool MustReallocate(const BufferFormatBase<T>& other) const noexcept
      = 0;

  virtual const void* PayloadPointer(const T& item) const noexcept = 0;

  virtual void Validate(const BuffersBase<T>& buffers) const = 0;

  virtual std::string Dump(const BuffersBase<T>& buffers) const noexcept = 0;

  static void ValidateSamplingRate(int value) {
    if (value < MIN_SAMPLING_RATE || value > MAX_SAMPLING_RATE) {
      throw Formats::InvalidSamplingRateException(value);
    }
  }

 private:
  int samplingRate_;
  bool incompatible_;
};

namespace Validation {
  template <class TE>
  struct Validator {
    static bool Validate(const TE&) noexcept {
      return true;
    }

    static bool Validate(TE&&) noexcept {
      return true;
    }
  };

  template <>
  struct Validator<float> {
    static bool Validate(float value) noexcept {
      return value == value && value != __builtin_inff() &&
        value != -__builtin_inff();
    }
  };
}  // namespace Validation

template <typename T>
class BuffersBase : public Buffers {
 public:
  typedef T ElementType;

  explicit BuffersBase(
      const std::shared_ptr<BufferFormatBase<T>>& format) noexcept
      : Buffers(0, format),
        initialized_(false) {
  }

  template <typename... TArgs>
  void Initialize(size_t size, TArgs... args) noexcept {
    assert(!initialized_ && "Already initialized");
    Buffers::SetSize(size);
    for (size_t i = 0; i < size; i++) {
      Set(i, new T(args...));
    }
    initialized_ = true;
  }

  T& operator[](int index) noexcept {
    return *reinterpret_cast<T*>(Buffers::operator [](index));  // NOLINT(*)
  }

  const T& operator[](int index) const noexcept {
    return *reinterpret_cast<const T*>(Buffers::operator [](index));  // NOLINT(*)
  }

  const std::shared_ptr<BufferFormatBase<T>> CastedFormat() const noexcept {
    return std::static_pointer_cast<BufferFormatBase<T>>(Format());
  }

private:
  BuffersBase(const BuffersBase<T>& other) = delete;
  BuffersBase& operator=(const BuffersBase<T>& other) = delete;

  bool initialized_;
};

template<class T>
class FormatLogger : public Logger {
 public:
  FormatLogger() noexcept : Logger(std::demangle(typeid(T).name()),
                                   EINA_COLOR_ORANGE) {
  }
};

}  // namespace SoundFeatureExtraction

#endif  // SRC_BUFFERS_BASE_H_
