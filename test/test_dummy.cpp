#include "Dummy.hpp"

#include "gtest/gtest.h"

namespace dum {
    class DummyTest: public ::testing::Test {
    protected:
        // any objects needed by all tests
        double dummyVal;

        void SetUp() override {
            // setup all the individual objects before each test
            dummyVal = 42.;
        }
    };

    TEST_F(DummyTest, GenericTest) {
        auto dummy = Dummy();
        dummy.setVal(dummyVal);

        EXPECT_EQ(dummy.getVal(), dummyVal);
    }

    TEST_F(DummyTest, GenericTest2) {
        auto dummy = Dummy(dummyVal);

        EXPECT_EQ(dummy.getVal(), dummyVal);
    }
} // namespace dum
