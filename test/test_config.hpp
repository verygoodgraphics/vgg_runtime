#pragma once

#include <gtest/gtest.h>

#define SKIP_S3_DEPENDENT_TEST GTEST_SKIP() << "Skipping local http url test";
#define SKIP_SLOW_TEST GTEST_SKIP() << "Skipping slow test";
#define SKIP_DEBUG_TEST GTEST_SKIP() << "Skipping debug test";
#define SKIP_LOCAL_TEST GTEST_SKIP() << "Skipping test that requires local files";

constexpr auto design_doc_schema_file = "./testDataDir/vgg-format-1.json";