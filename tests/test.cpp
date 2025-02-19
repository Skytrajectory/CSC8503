#include <gtest/gtest.h>

// 一个简单的测试
TEST(MathTest, Add) {
    EXPECT_EQ(2 + 2, 4);
}

TEST(MathTest, Subtract) {
    EXPECT_EQ(5 - 3, 2);
}

// main 函数
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
