#include <garak/utils/module.hpp>

#include <gtest/gtest.h>
/**
 * @brief Test our library
 * * */

using namespace garak;
TEST(GarakVersionTest, BasicAssertions) {
  // Expect equality.
  EXPECT_EQ("0.1.0", utils::get_version());
}
