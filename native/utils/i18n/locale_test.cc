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

#include "utils/i18n/locale.h"

#include "gtest/gtest.h"

namespace libtextclassifier3 {
namespace {

TEST(LocaleTest, ParseUnknown) {
  Locale locale = Locale::Invalid();
  EXPECT_FALSE(locale.IsValid());
}

TEST(LocaleTest, ParseSwissEnglish) {
  Locale locale = Locale::FromBCP47("en-CH");
  EXPECT_TRUE(locale.IsValid());
  EXPECT_EQ(locale.Language(), "en");
  EXPECT_EQ(locale.Script(), "");
  EXPECT_EQ(locale.Region(), "CH");
}

TEST(LocaleTest, ParseChineseChina) {
  Locale locale = Locale::FromBCP47("zh-CN");
  EXPECT_TRUE(locale.IsValid());
  EXPECT_EQ(locale.Language(), "zh");
  EXPECT_EQ(locale.Script(), "");
  EXPECT_EQ(locale.Region(), "CN");
}

TEST(LocaleTest, ParseChineseTaiwan) {
  Locale locale = Locale::FromBCP47("zh-Hant-TW");
  EXPECT_TRUE(locale.IsValid());
  EXPECT_EQ(locale.Language(), "zh");
  EXPECT_EQ(locale.Script(), "Hant");
  EXPECT_EQ(locale.Region(), "TW");
}

TEST(LocaleTest, ParseEnglish) {
  Locale locale = Locale::FromBCP47("en");
  EXPECT_TRUE(locale.IsValid());
  EXPECT_EQ(locale.Language(), "en");
  EXPECT_EQ(locale.Script(), "");
  EXPECT_EQ(locale.Region(), "");
}

TEST(LocaleTest, ParseCineseTraditional) {
  Locale locale = Locale::FromBCP47("zh-Hant");
  EXPECT_TRUE(locale.IsValid());
  EXPECT_EQ(locale.Language(), "zh");
  EXPECT_EQ(locale.Script(), "Hant");
  EXPECT_EQ(locale.Region(), "");
}

TEST(LocaleTest, IsAnyLocaleSupportedMatch) {
  std::vector<Locale> locales = {Locale::FromBCP47("zh-HK"),
                                 Locale::FromBCP47("en-UK")};
  std::vector<Locale> supported_locales = {Locale::FromBCP47("en")};

  EXPECT_TRUE(Locale::IsAnyLocaleSupported(locales, supported_locales,
                                           /*default_value=*/false));
}

TEST(LocaleTest, IsAnyLocaleSupportedNotMatch) {
  std::vector<Locale> locales = {Locale::FromBCP47("zh-tw")};
  std::vector<Locale> supported_locales = {Locale::FromBCP47("en"),
                                           Locale::FromBCP47("fr")};

  EXPECT_FALSE(Locale::IsAnyLocaleSupported(locales, supported_locales,
                                            /*default_value=*/false));
}

TEST(LocaleTest, IsAnyLocaleSupportedAnyLocale) {
  std::vector<Locale> locales = {Locale::FromBCP47("zh-tw")};
  std::vector<Locale> supported_locales = {Locale::FromBCP47("*")};

  EXPECT_TRUE(Locale::IsAnyLocaleSupported(locales, supported_locales,
                                           /*default_value=*/false));
}

TEST(LocaleTest, IsAnyLocaleSupportedEmptyLocales) {
  std::vector<Locale> supported_locales = {Locale::FromBCP47("en")};

  EXPECT_TRUE(Locale::IsAnyLocaleSupported({}, supported_locales,
                                           /*default_value=*/true));
}

}  // namespace
}  // namespace libtextclassifier3
