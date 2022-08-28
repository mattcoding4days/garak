#include <gtest/gtest.h>

#include <garak/version.hpp>
/**
 * @brief Test our library
 *
 * */
TEST(CoreVersionTest, BasicAssertions) {
  // Expect equality.
  EXPECT_EQ("0.1.0", garak::get_version());
}
