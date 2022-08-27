#include <gtest/gtest.h>

#include <starter_app/version.hpp>

using namespace starter;

TEST(Version, BasicAssertion) { EXPECT_EQ("0.1.0", app::get_version()); }
