#include "core/circuit.hpp"

#include <cstddef>
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

using namespace syrec;

class CircuitTestsFixture: public testing::Test {
protected:
    std::unique_ptr<Circuit> circuit;

    void SetUp() override {
        circuit = std::make_unique<Circuit>();
    }

    static void assertThatGatesMatch(const Gate& expected, const Gate& actual) {
        ASSERT_EQ(expected.type, actual.type);
        ASSERT_THAT(actual.controls, testing::UnorderedElementsAreArray(expected.controls.cbegin(), expected.controls.cend()));
        ASSERT_THAT(actual.targets, testing::UnorderedElementsAreArray(expected.targets.cbegin(), expected.targets.cend()));
    }

    static void assertThatGatesOfCircuitAreEqualToSequence(const Circuit& circuit, const std::initializer_list<Gate::ptr>& expectedCircuitGates) {
        const std::size_t numGatesInCircuit = circuit.numGates();
        ASSERT_EQ(expectedCircuitGates.size(), numGatesInCircuit) << "Expected that circuit contains " << std::to_string(expectedCircuitGates.size()) << " gates but actually contained " << std::to_string(numGatesInCircuit) << " gates";
        const std::vector<Gate::ptr> gatesOfCircuit = {circuit.cbegin(), circuit.cend()};

        auto expectedCircuitGatesIterator = expectedCircuitGates.begin();
        auto actualCircuitGatesIterator   = circuit.begin();
        for (std::size_t i = 0; i < numGatesInCircuit; ++i) {
            ASSERT_THAT(*expectedCircuitGatesIterator, testing::NotNull());
            ASSERT_THAT(*actualCircuitGatesIterator, testing::NotNull());
            assertThatGatesMatch(**expectedCircuitGatesIterator, **actualCircuitGatesIterator);
            ++expectedCircuitGatesIterator;
            ++actualCircuitGatesIterator;
        }
    }
};

// GATE functionality
TEST_F(CircuitTestsFixture, AddToffoliGate) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne     = 1;
    constexpr Gate::Line controlLineTwo     = 2;
    constexpr Gate::Line targetLine         = 0;
    const auto           createdToffoliGate = circuit->createAndAddToffoliGate(controlLineOne, controlLineTwo, targetLine);
    ASSERT_THAT(createdToffoliGate, testing::NotNull());

    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {createdToffoliGate});
}

// TODO: Control lines cannot be equal
// TOOD: Target line cannot equal any of the two control lines
TEST_F(CircuitTestsFixture, AddToffoliGateWithUnknownControlLine) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line unknownControlLine = numCircuitLines + 1;
    constexpr Gate::Line knownControlLine   = 1;
    constexpr Gate::Line targetLine         = 2;
    auto                 createdToffoliGate = circuit->createAndAddToffoliGate(unknownControlLine, knownControlLine, targetLine);
    ASSERT_THAT(createdToffoliGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});

    createdToffoliGate = circuit->createAndAddToffoliGate(knownControlLine, unknownControlLine, targetLine);
    ASSERT_THAT(createdToffoliGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

TEST_F(CircuitTestsFixture, AddToffoliGateWithDuplicateControlLineNotPossible) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLine        = 1;
    constexpr Gate::Line targetLine         = 0;
    const auto           createdToffoliGate = circuit->createAndAddToffoliGate(controlLine, controlLine, targetLine);
    ASSERT_THAT(createdToffoliGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

TEST_F(CircuitTestsFixture, AddToffoliGateWithTargetLineBeingEqualToEitherControlLineNotPossible) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line firstControlLine  = 0;
    constexpr Gate::Line secondControlLine = 1;

    auto createdToffoliGate = circuit->createAndAddToffoliGate(firstControlLine, secondControlLine, firstControlLine);
    ASSERT_THAT(createdToffoliGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});

    createdToffoliGate = circuit->createAndAddToffoliGate(firstControlLine, secondControlLine, secondControlLine);
    ASSERT_THAT(createdToffoliGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

TEST_F(CircuitTestsFixture, AddToffoliGateWithUnknownTargetLine) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line firstControlLine   = 1;
    constexpr Gate::Line secondControlLine  = 2;
    constexpr Gate::Line unknownCircuitLine = numCircuitLines + 1;
    const auto           createdToffoliGate = circuit->createAndAddToffoliGate(firstControlLine, secondControlLine, unknownCircuitLine);
    ASSERT_THAT(createdToffoliGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

TEST_F(CircuitTestsFixture, AddCnotGate) {
    constexpr unsigned numCircuitLines = 2;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLine     = 0;
    constexpr Gate::Line targetLine      = 1;
    const auto           createdCnotGate = circuit->createAndAddCnotGate(controlLine, targetLine);
    ASSERT_THAT(createdCnotGate, testing::NotNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {createdCnotGate});
}

TEST_F(CircuitTestsFixture, AddCnotGateWithUnknownControlLine) {
    constexpr unsigned numCircuitLines = 2;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLine     = numCircuitLines + 1;
    constexpr Gate::Line targetLine      = 1;
    const auto           createdCnotGate = circuit->createAndAddCnotGate(controlLine, targetLine);
    ASSERT_THAT(createdCnotGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

TEST_F(CircuitTestsFixture, AddCnotGateWithUnknownTargetLine) {
    constexpr unsigned numCircuitLines = 2;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLine     = 1;
    constexpr Gate::Line targetLine      = numCircuitLines + 1;
    const auto           createdCnotGate = circuit->createAndAddCnotGate(controlLine, targetLine);
    ASSERT_THAT(createdCnotGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

TEST_F(CircuitTestsFixture, AddCnotGateWithControlAndTargetLineBeingSameLine) {
    constexpr unsigned numCircuitLines = 1;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLine     = 0;
    const auto           createdCnotGate = circuit->createAndAddCnotGate(controlLine, controlLine);
    ASSERT_THAT(createdCnotGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

TEST_F(CircuitTestsFixture, AddNotGate) {
    constexpr unsigned numCircuitLines = 1;
    circuit->setLines(numCircuitLines);

    const auto createdNotGate = circuit->createAndAddNotGate(0);
    ASSERT_THAT(createdNotGate, testing::NotNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {createdNotGate});
}

TEST_F(CircuitTestsFixture, AddNotGateWithUnknownTargetLine) {
    constexpr unsigned numCircuitLines = 1;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line unknownTargetLine = numCircuitLines + 1;
    const auto           createdNotGate    = circuit->createAndAddNotGate(unknownTargetLine);
    ASSERT_THAT(createdNotGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

TEST_F(CircuitTestsFixture, AddMultiControlToffoliGate) {
    constexpr unsigned numCircuitLines = 4;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line    firstControlLine  = 1;
    constexpr Gate::Line    secondControlLine = 3;
    constexpr Gate::Line    thirdControlLine  = 2;
    constexpr Gate::Line    targetLine        = 0;
    const Gate::LinesLookup controlLines      = {firstControlLine, secondControlLine, thirdControlLine};

    const auto createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate(controlLines, targetLine);
    ASSERT_THAT(createdMultiControlToffoliGate, testing::NotNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {createdMultiControlToffoliGate});
}

TEST_F(CircuitTestsFixture, AddMultiControlToffoliGateWithUnknownControlLine) {
    constexpr unsigned numCircuitLines = 4;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line    firstControlLine   = 1;
    constexpr Gate::Line    unknownControlLine = numCircuitLines + 1;
    constexpr Gate::Line    thirdControlLine   = 2;
    constexpr Gate::Line    targetLine         = 0;
    const Gate::LinesLookup controlLines       = {firstControlLine, unknownControlLine, thirdControlLine};

    const auto createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate(controlLines, targetLine);
    ASSERT_THAT(createdMultiControlToffoliGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

TEST_F(CircuitTestsFixture, AddMultiControlToffoliGateWithUnknownTargetLine) {
    constexpr unsigned numCircuitLines = 4;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line    firstControlLine  = 1;
    constexpr Gate::Line    secondControlLine = 3;
    constexpr Gate::Line    thirdControlLine  = 2;
    constexpr Gate::Line    targetLine        = numCircuitLines + 1;
    const Gate::LinesLookup controlLines      = {firstControlLine, secondControlLine, thirdControlLine};

    const auto createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate(controlLines, targetLine);
    ASSERT_THAT(createdMultiControlToffoliGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

TEST_F(CircuitTestsFixture, AddMultiControlToffoliGateWithoutControlLinesAndNoActiveLocalControlLineScopes) {
    constexpr unsigned numCircuitLines = 1;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line targetLine                     = 0;
    const auto           createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate({}, targetLine);
    ASSERT_THAT(createdMultiControlToffoliGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

// TODO: Target line equal to control line of parent control line
TEST_F(CircuitTestsFixture, AddMultiControlToffoliGateWithTargetLineBeingEqualToControlLine) {
    constexpr unsigned numCircuitLines = 4;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line    firstControlLine  = 1;
    constexpr Gate::Line    secondControlLine = 3;
    constexpr Gate::Line    thirdControlLine  = 2;
    constexpr Gate::Line    targetLine        = secondControlLine;
    const Gate::LinesLookup controlLines      = {firstControlLine, secondControlLine, thirdControlLine};

    const auto createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate(controlLines, targetLine);
    ASSERT_THAT(createdMultiControlToffoliGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

TEST_F(CircuitTestsFixture, AddFredkinGate) {
    constexpr unsigned numCircuitLines = 2;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line firstTargetLine  = 0;
    constexpr Gate::Line secondTargetLine = 1;

    const auto createdFredkinGate = circuit->createAndAddFredkinGate(firstTargetLine, secondTargetLine);
    ASSERT_THAT(createdFredkinGate, testing::NotNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {createdFredkinGate});
}

TEST_F(CircuitTestsFixture, AddFredkinGateWithUnknownTargetLine) {
    constexpr unsigned numCircuitLines = 2;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line knownTargetLine   = 1;
    constexpr Gate::Line unknownTargetLine = numCircuitLines + 1;

    auto createdFredkinGate = circuit->createAndAddFredkinGate(knownTargetLine, unknownTargetLine);
    ASSERT_THAT(createdFredkinGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});

    createdFredkinGate = circuit->createAndAddFredkinGate(unknownTargetLine, knownTargetLine);
    ASSERT_THAT(createdFredkinGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

TEST_F(CircuitTestsFixture, AddFredkinGateWithTargetLinesTargettingSameLine) {
    constexpr unsigned numCircuitLines = 1;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line targetLine = 0;

    const auto createdFredkinGate = circuit->createAndAddFredkinGate(targetLine, targetLine);
    ASSERT_THAT(createdFredkinGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}


// TODO: Stringify function might use custom non-QASM conforming format that is only accepted by MQT::Core?
TEST_F(CircuitTestsFixture, StringifyToffoliGate) {
    GTEST_SKIP();
}

TEST_F(CircuitTestsFixture, StringifyCnotGate) {
    GTEST_SKIP();
}

TEST_F(CircuitTestsFixture, StringifyMultiControlToffoliGate) {
    GTEST_SKIP();
}

TEST_F(CircuitTestsFixture, StringifyNotGate) {
    GTEST_SKIP();
}

TEST_F(CircuitTestsFixture, StringifyFredkinGate) {
    GTEST_SKIP();
}

// Local control line scope functionality
TEST_F(CircuitTestsFixture, RegisterControlLineInLocalControlLineScope) {
    GTEST_SKIP();
}

TEST_F(CircuitTestsFixture, RegisterDuplicateControlLineInLocalControlLineScope) {
    GTEST_SKIP();
}

TEST_F(CircuitTestsFixture, RegisterDuplicateControlLineOfParentNotExistingInLocalControlLineScope) {
    GTEST_SKIP();
}

TEST_F(CircuitTestsFixture, RegisterControlLineNotKnownInCircuit) {
    GTEST_SKIP();
}

TEST_F(CircuitTestsFixture, DeregisterControLineOfLocalControlLineScope) {
    GTEST_SKIP();
}

TEST_F(CircuitTestsFixture, DeregisterControLineOfParentScopeInLastActivateControlLineScope) {
    GTEST_SKIP();
}

TEST_F(CircuitTestsFixture, DeregisterControlLineNotRegisteredForLocalControlLineScope) {
    GTEST_SKIP();
}

TEST_F(CircuitTestsFixture, DeregisterControlLineNotKnownInCircuit) {
    GTEST_SKIP();
}

TEST_F(CircuitTestsFixture, ActivatingLocalControlLineScopeDoesNotAddNewControlLines) {
    GTEST_SKIP();
}

TEST_F(CircuitTestsFixture, DeactivateLocalControlLineScopeWithNoControlLinesSharedWithAnyParentScope) {
    GTEST_SKIP();
}

TEST_F(CircuitTestsFixture, DeactivateLocalControlLineScopeWithControlLinesSharedWithAnyParentScope) {
    GTEST_SKIP();
}