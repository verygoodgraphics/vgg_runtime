#pragma once

#include <gtest/gtest.h>

#define SKIP_S3_DEPENDENT_TEST GTEST_SKIP() << "Skipping local http url test";