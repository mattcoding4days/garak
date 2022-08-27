#include <gtest/gtest.h>

#include <starter_core/version.hpp>
/**
 * @brief Test our library
 *
 * */
TEST(CoreVersionTest, BasicAssertions) {
  // Expect equality.
  EXPECT_EQ("0.1.0", starter::core::get_version());
}
