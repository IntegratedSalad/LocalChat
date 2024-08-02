#include <gtest/gtest.h>
#include <dns_sd.h>

TEST(DnsSdRegistration, CheckDaemonVersion)
{
  
}

// Successful registration
TEST(DnsSdRegistration, RegistrationSuccess)
{
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}

// Successful registration
TEST(DnsSdRegistration, RegistrationFailure)
{
    
}