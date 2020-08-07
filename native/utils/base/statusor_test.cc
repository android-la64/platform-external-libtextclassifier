/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "utils/base/statusor.h"

#include "utils/base/logging.h"
#include "utils/base/status.h"
#include "gtest/gtest.h"

namespace libtextclassifier3 {
namespace {

TEST(StatusOrTest, DoesntDieWhenOK) {
  StatusOr<std::string> status_or_string = std::string("Hello World");
  EXPECT_TRUE(status_or_string.ok());
  EXPECT_EQ(status_or_string.ValueOrDie(), "Hello World");
}

TEST(StatusOrTest, DiesWhenNotOK) {
  StatusOr<std::string> status_or_string = {Status::UNKNOWN};
  EXPECT_FALSE(status_or_string.ok());
  // Android does not print the error message to stderr, so we are not checking
  // the error message here.
  EXPECT_DEATH(status_or_string.ValueOrDie(), "");
}

// Foo is NOT default constructible and can be implicitly converted to from int.
class Foo {
 public:
  // Copy value conversion
  Foo(int i) : i_(i) {}  // NOLINT
  int i() const { return i_; }

 private:
  int i_;
};

TEST(StatusOrTest, HandlesNonDefaultConstructibleValues) {
  StatusOr<Foo> foo_or(Foo(7));
  EXPECT_TRUE(foo_or.ok());
  EXPECT_EQ(foo_or.ValueOrDie().i(), 7);

  StatusOr<Foo> error_or(Status::UNKNOWN);
  EXPECT_FALSE(error_or.ok());
  EXPECT_EQ(error_or.status().CanonicalCode(), StatusCode::UNKNOWN);
}

class Bar {
 public:
  // Move value conversion
  Bar(Foo&& f) : i_(2 * f.i()) {}  // NOLINT

  // Movable, but not copyable.
  Bar(const Bar& other) = delete;
  Bar& operator=(const Bar& rhs) = delete;
  Bar(Bar&& other) = default;
  Bar& operator=(Bar&& rhs) = default;

  int i() const { return i_; }

 private:
  int i_;
};

TEST(StatusOrTest, HandlesValueConversion) {
  // Copy value conversion constructor : StatusOr<Foo>(const int&)
  StatusOr<Foo> foo_status(19);
  EXPECT_TRUE(foo_status.ok());
  EXPECT_EQ(foo_status.ValueOrDie().i(), 19);

  // Move value conversion constructor : StatusOr<Bar>(Foo&&)
  StatusOr<Bar> bar_status(std::move(foo_status));
  EXPECT_TRUE(bar_status.ok());
  EXPECT_EQ(bar_status.ValueOrDie().i(), 38);

  StatusOr<int> int_status(19);
  // Copy conversion constructor : StatusOr<Foo>(const StatusOr<int>&)
  StatusOr<Foo> copied_status(int_status);
  EXPECT_TRUE(copied_status.ok());
  EXPECT_EQ(copied_status.ValueOrDie().i(), 19);

  // Move conversion constructor : StatusOr<Bar>(StatusOr<Foo>&&)
  StatusOr<Bar> moved_status(std::move(copied_status));
  EXPECT_TRUE(moved_status.ok());
  EXPECT_EQ(moved_status.ValueOrDie().i(), 38);

  // Move conversion constructor with error : StatusOr<Bar>(StatusOr<Foo>&&)
  StatusOr<Foo> error_status(Status::UNKNOWN);
  StatusOr<Bar> moved_error_status(std::move(error_status));
  EXPECT_FALSE(moved_error_status.ok());
}

// Create a class that has validly defined copy and move operators, but will
// cause a crash if assignment operators are invoked on an instance that was
// never initialized.
class Baz {
 public:
  Baz() : i_(new int), invalid_(false) {}
  Baz(const Baz& other) {
    i_ = new int;
    *i_ = *other.i_;
    invalid_ = false;
  }
  Baz(const Foo& other) {  // NOLINT
    i_ = new int;
    *i_ = other.i();
    invalid_ = false;
  }
  Baz(Baz&& other) {
    // Copy other.i_ into this so that this holds it now. Mark other as invalid
    // so that it doesn't destroy the int that this now owns when other is
    // destroyed.
    i_ = other.i_;
    other.invalid_ = true;
    invalid_ = false;
  }
  Baz& operator=(const Baz& rhs) {
    // Copy rhs.i_ into tmp. Then swap this with tmp so that this no has the
    // value that rhs had and tmp will destroy the value that this used to hold.
    Baz tmp(rhs);
    std::swap(i_, tmp.i_);
    return *this;
  }
  Baz& operator=(Baz&& rhs) {
    std::swap(i_, rhs.i_);
    return *this;
  }
  ~Baz() {
    if (!invalid_) delete i_;
  }

 private:
  int* i_;
  bool invalid_;
};

TEST(StatusOrTest, CopyAssignment) {
  StatusOr<Baz> baz_or;
  EXPECT_FALSE(baz_or.ok());
  Baz b;
  StatusOr<Baz> other(b);
  baz_or = other;
  EXPECT_TRUE(baz_or.ok());
  EXPECT_TRUE(other.ok());
}

TEST(StatusOrTest, MoveAssignment) {
  StatusOr<Baz> baz_or;
  EXPECT_FALSE(baz_or.ok());
  baz_or = StatusOr<Baz>(Baz());
  EXPECT_TRUE(baz_or.ok());
}

TEST(StatusOrTest, CopyConversionAssignment) {
  StatusOr<Baz> baz_or;
  EXPECT_FALSE(baz_or.ok());
  StatusOr<Foo> foo_or(Foo(12));
  baz_or = foo_or;
  EXPECT_TRUE(baz_or.ok());
  EXPECT_TRUE(foo_or.ok());
}

TEST(StatusOrTest, MoveConversionAssignment) {
  StatusOr<Baz> baz_or;
  EXPECT_FALSE(baz_or.ok());
  StatusOr<Foo> foo_or(Foo(12));
  baz_or = std::move(foo_or);
  EXPECT_TRUE(baz_or.ok());
}

struct OkFn {
  StatusOr<int> operator()() { return 42; }
};
TEST(StatusOrTest, AssignOrReturnValOk) {
  auto lambda = []() {
    TC3_ASSIGN_OR_RETURN(int i, OkFn()(), -1);
    return i;
  };

  // OkFn() should return a valid integer, so lambda should return that integer.
  EXPECT_EQ(lambda(), 42);
}

struct FailFn {
  StatusOr<int> operator()() { return Status::UNKNOWN; }
};
TEST(StatusOrTest, AssignOrReturnValError) {
  auto lambda = []() {
    TC3_ASSIGN_OR_RETURN(int i, FailFn()(), -1);
    return i;
  };

  // FailFn() should return an error, so lambda should return -1.
  EXPECT_EQ(lambda(), -1);
}

}  // namespace
}  // namespace libtextclassifier3
