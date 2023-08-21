#pragma once

#include <gtest/gtest.h>

#define SKIP_S3_DEPENDENT_TEST GTEST_SKIP() << "Skipping local http url test";
#define SKIP_SLOW_TEST GTEST_SKIP() << "Skipping slow test";

constexpr auto design_doc_schema_file = "./testDataDir/vgg-format-1.json";