#include "core/circuit.hpp"

#include <cstddef>
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

using namespace syrec;

// TODO: Defined expected gate data instead of reusing returned data of function
// TODO: Compare returned gate data with expected gate data
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

// TODO: Control line deregistered in parent scope not added to gate?
// TODO: Control line deregistered in parent scope usable as target line?

// BEGIN AddXGate tests
TEST_F(CircuitTestsFixture, AddToffoliGate) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne     = 1;
    constexpr Gate::Line controlLineTwo     = 2;
    constexpr Gate::Line targetLine         = 0;
    const auto           createdToffoliGate = circuit->createAndAddToffoliGate(controlLineOne, controlLineTwo, targetLine);
    ASSERT_THAT(createdToffoliGate, testing::NotNull());

    auto expectedToffoliGate      = std::make_shared<Gate>();
    expectedToffoliGate->type     = Gate::Type::Toffoli;
    expectedToffoliGate->controls = {controlLineOne, controlLineTwo};
    expectedToffoliGate->targets.emplace(targetLine);
    assertThatGatesMatch(*expectedToffoliGate, *createdToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedToffoliGate});
}

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
    ASSERT_THAT(createdToffoliGate, testing::NotNull());

    auto expectedToffoliGate = std::make_shared<Gate>();
    expectedToffoliGate->type = Gate::Type::Toffoli;
    expectedToffoliGate->controls.emplace(controlLine);
    expectedToffoliGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedToffoliGate, *createdToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedToffoliGate});
}

TEST_F(CircuitTestsFixture, AddToffoliGateWithTargetLineBeingEqualToEitherControlLineNotPossible) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne = 0;
    constexpr Gate::Line controlLineTwo = 1;

    auto createdToffoliGate = circuit->createAndAddToffoliGate(controlLineOne, controlLineTwo, controlLineOne);
    ASSERT_THAT(createdToffoliGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});

    createdToffoliGate = circuit->createAndAddToffoliGate(controlLineOne, controlLineTwo, controlLineTwo);
    ASSERT_THAT(createdToffoliGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

TEST_F(CircuitTestsFixture, AddToffoliGateWithUnknownTargetLine) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne     = 1;
    constexpr Gate::Line controlLineTwo     = 2;
    constexpr Gate::Line unknownCircuitLine = numCircuitLines + 1;
    const auto           createdToffoliGate = circuit->createAndAddToffoliGate(controlLineOne, controlLineTwo, unknownCircuitLine);
    ASSERT_THAT(createdToffoliGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

TEST_F(CircuitTestsFixture, AddToffoliGateWithActiveControlLinesInParentControlLineScopes) {
    constexpr unsigned numCircuitLines = 6;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne   = 0;
    constexpr Gate::Line controlLineTwo   = 1;
    constexpr Gate::Line controlLineThree = 2;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineThree);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineTwo);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineThree);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineThree);

    constexpr Gate::Line gateControlLineOne = 3;
    constexpr Gate::Line gateControlLineTwo = 4;
    constexpr Gate::Line gateTargetLine     = 5;
    const auto           createdToffoliGate = circuit->createAndAddToffoliGate(gateControlLineOne, gateControlLineTwo, gateTargetLine);
    ASSERT_THAT(createdToffoliGate, testing::NotNull());

    auto expectedToffoliGate      = std::make_shared<Gate>();
    expectedToffoliGate->type     = Gate::Type::Toffoli;
    expectedToffoliGate->controls = {controlLineOne, gateControlLineOne, gateControlLineTwo};
    expectedToffoliGate->targets  = {gateTargetLine};

    assertThatGatesMatch(*expectedToffoliGate, *createdToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedToffoliGate});
}

TEST_F(CircuitTestsFixture, AddToffoliGateWithTargetLineMatchingActiveControlLineInAnyParentControlLineScope) {
    constexpr unsigned numCircuitLines = 4;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne = 0;
    constexpr Gate::Line controlLineTwo = 1;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);

    constexpr Gate::Line gateControlLineOne = 2;
    constexpr Gate::Line gateControlLineTwo = 3;
    constexpr Gate::Line targetLine         = controlLineTwo;
    const auto           createdToffoliGate = circuit->createAndAddToffoliGate(gateControlLineOne, gateControlLineTwo, targetLine);
    ASSERT_THAT(createdToffoliGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

TEST_F(CircuitTestsFixture, AddToffoliGateWithControlLinesBeingDisabledInCurrentControlLineScope) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne  = 0;
    constexpr Gate::Line controlLineTwo = 1;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineOne);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineTwo);

    constexpr Gate::Line gateControlLine = 2;
    constexpr Gate::Line targetLine      = controlLineTwo;
    // Both control lines of toffoli gate were deactived in propagation scope
    auto createdToffoliGate = circuit->createAndAddToffoliGate(controlLineOne, controlLineTwo, targetLine);
    ASSERT_THAT(createdToffoliGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});

    createdToffoliGate = circuit->createAndAddToffoliGate(controlLineOne, gateControlLine, targetLine);
    ASSERT_THAT(createdToffoliGate, testing::NotNull());

    auto firstExpectedToffoliGateWithOneControlLine  = std::make_shared<Gate>();
    firstExpectedToffoliGateWithOneControlLine->type = Gate::Type::Toffoli;
    firstExpectedToffoliGateWithOneControlLine->controls.emplace(gateControlLine);
    firstExpectedToffoliGateWithOneControlLine->targets.emplace(targetLine);

    assertThatGatesMatch(*createdToffoliGate, *firstExpectedToffoliGateWithOneControlLine);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {firstExpectedToffoliGateWithOneControlLine});

    createdToffoliGate = circuit->createAndAddToffoliGate(gateControlLine, controlLineOne, targetLine);
    ASSERT_THAT(createdToffoliGate, testing::NotNull());

    auto secondExpectedToffoliGateWithOneControlLine  = std::make_shared<Gate>();
    secondExpectedToffoliGateWithOneControlLine->type = Gate::Type::Toffoli;
    secondExpectedToffoliGateWithOneControlLine->controls.emplace(gateControlLine);
    secondExpectedToffoliGateWithOneControlLine->targets.emplace(targetLine);

    assertThatGatesMatch(*createdToffoliGate, *secondExpectedToffoliGateWithOneControlLine);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {firstExpectedToffoliGateWithOneControlLine, secondExpectedToffoliGateWithOneControlLine});

    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(gateControlLine);

    // While both user provided control lines are deactivated in aggregate of the control lines, the one control line
    // registered in the last activated control line propagation scope is active and thus used as the control line of the toffoli gate.
    createdToffoliGate = circuit->createAndAddToffoliGate(controlLineOne, controlLineTwo, targetLine);
    ASSERT_THAT(createdToffoliGate, testing::NotNull());

    auto expectedToffoliGate      = std::make_shared<Gate>();
    expectedToffoliGate->type     = Gate::Type::Toffoli;
    expectedToffoliGate->controls = {gateControlLine};
    expectedToffoliGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedToffoliGate, *createdToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {firstExpectedToffoliGateWithOneControlLine, secondExpectedToffoliGateWithOneControlLine, expectedToffoliGate});
}

TEST_F(CircuitTestsFixture, AddToffoliGateWithScopeActivatingDeactivatedControlLineOfParentScope) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne = 0;
    constexpr Gate::Line controlLineTwo = 1;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineTwo);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);

    constexpr Gate::Line targetLine         = 2;
    const auto           createdToffoliGate = circuit->createAndAddToffoliGate(controlLineOne, controlLineTwo, targetLine);
    ASSERT_THAT(createdToffoliGate, testing::NotNull());

    auto expectedToffoliGate      = std::make_shared<Gate>();
    expectedToffoliGate->type     = Gate::Type::Toffoli;
    expectedToffoliGate->controls = {controlLineOne, controlLineTwo};
    expectedToffoliGate->targets.emplace(targetLine);
    assertThatGatesMatch(*createdToffoliGate, *expectedToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedToffoliGate});
}

TEST_F(CircuitTestsFixture, AddToffoliGateWithDeactivationOfControlLinePropagationScope) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne = 0;
    constexpr Gate::Line controlLineTwo = 1;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineOne);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineTwo);
    circuit->deactivateControlLinePropagationScope();

    constexpr Gate::Line targetLine         = 2;
    const auto           createdToffoliGate = circuit->createAndAddToffoliGate(controlLineOne, controlLineTwo, targetLine);
    ASSERT_THAT(createdToffoliGate, testing::NotNull());

    auto expectedToffoliGate      = std::make_shared<Gate>();
    expectedToffoliGate->type     = Gate::Type::Toffoli;
    expectedToffoliGate->controls = {controlLineOne, controlLineTwo};
    expectedToffoliGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedToffoliGate, *createdToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedToffoliGate});
}

TEST_F(CircuitTestsFixture, AddToffoliGateWithTargetLineMatchingDeactivatedControlLineOfPropagationScope) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne   = 0;
    constexpr Gate::Line controlLineTwo   = 1;
    constexpr Gate::Line controlLineThree = 2;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineOne);

    constexpr Gate::Line gateControlLineOne = controlLineTwo;
    constexpr Gate::Line gateControlLineTwo = controlLineThree;
    constexpr Gate::Line targetLine         = controlLineOne;
    const auto           createdToffoliGate     = circuit->createAndAddToffoliGate(gateControlLineOne, gateControlLineTwo, targetLine);
    ASSERT_THAT(createdToffoliGate, testing::NotNull());

    auto expectedToffoliGate      = std::make_shared<Gate>();
    expectedToffoliGate->type     = Gate::Type::Toffoli;
    expectedToffoliGate->controls = {gateControlLineOne, gateControlLineTwo};
    expectedToffoliGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedToffoliGate, *createdToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedToffoliGate});
}

TEST_F(CircuitTestsFixture, AddToffoliGateWithCallerProvidedControlLinesMatchingDeregisteredControlLinesOfParentScope) {
    constexpr unsigned numCircuitLines = 4;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne   = 0;
    constexpr Gate::Line controlLineTwo   = 1;
    constexpr Gate::Line controlLineThree = 2;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineThree);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);

    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineOne);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineTwo);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineThree);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineThree);

    auto createdToffoliGate = circuit->createAndAddToffoliGate(controlLineOne, controlLineTwo, controlLineThree);
    ASSERT_THAT(createdToffoliGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});

    constexpr Gate::Line targetLine      = controlLineThree;
    constexpr Gate::Line controlLineFour = 3;
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    createdToffoliGate = circuit->createAndAddToffoliGate(controlLineFour, controlLineTwo, targetLine);

    auto expectedToffoliGate      = std::make_shared<Gate>();
    expectedToffoliGate->type     = Gate::Type::Toffoli;
    expectedToffoliGate->controls = {controlLineOne, controlLineFour};
    expectedToffoliGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedToffoliGate, *createdToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedToffoliGate});
}

TEST_F(CircuitTestsFixture, AddCnotGate) {
    constexpr unsigned numCircuitLines = 2;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLine     = 0;
    constexpr Gate::Line targetLine      = 1;
    const auto           createdCnotGate = circuit->createAndAddCnotGate(controlLine, targetLine);
    ASSERT_THAT(createdCnotGate, testing::NotNull());

    auto expectedCnotGate  = std::make_shared<Gate>();
    expectedCnotGate->type = Gate::Type::Toffoli;
    expectedCnotGate->controls.emplace(controlLine);
    expectedCnotGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedCnotGate, *createdCnotGate);
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

TEST_F(CircuitTestsFixture, AddCnotGateWithActiveControlLinesInParentControlLineScopes) {
    constexpr unsigned numCircuitLines = 6;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne   = 0;
    constexpr Gate::Line controlLineTwo   = 1;
    constexpr Gate::Line controlLineThree = 2;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineThree);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineTwo);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineThree);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineThree);

    constexpr Gate::Line controlLineFour = 3;
    constexpr Gate::Line targetLine      = 4;
    const auto           createdCnotGate = circuit->createAndAddCnotGate(controlLineFour, targetLine);
    ASSERT_THAT(createdCnotGate, testing::NotNull());

    auto expectedCnotGate      = std::make_shared<Gate>();
    expectedCnotGate->type     = Gate::Type::Toffoli;
    expectedCnotGate->controls = {controlLineOne, controlLineFour};
    expectedCnotGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedCnotGate, *createdCnotGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedCnotGate});
}

TEST_F(CircuitTestsFixture, AddCnotGateWithTargetLineMatchingActiveControlLineInAnyParentControlLineScope) {
    constexpr unsigned numCircuitLines = 2;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne = 0;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);

    circuit->activateControlLinePropagationScope();

    constexpr Gate::Line controlLineTwo  = 1;
    constexpr Gate::Line targetLine      = controlLineOne;
    const auto           createdCnotGate = circuit->createAndAddCnotGate(controlLineTwo, targetLine);
    ASSERT_THAT(createdCnotGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

TEST_F(CircuitTestsFixture, AddCnotGateWithControlLineBeingDeactivatedInCurrentControlLineScope) {
    constexpr unsigned numCircuitLines = 2;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLine = 0;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLine);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLine);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLine);

    constexpr Gate::Line targetLine      = 1;
    const auto           createdCnotGate = circuit->createAndAddCnotGate(controlLine, targetLine);
    ASSERT_THAT(createdCnotGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

TEST_F(CircuitTestsFixture, AddCnotGateWithDeactivationOfControlLinePropagationScope) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne = 0;
    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineOne);
    circuit->deactivateControlLinePropagationScope();

    constexpr Gate::Line controlLineTwo  = 1;
    constexpr Gate::Line targetLine      = 2;
    const auto           createdCnotGate = circuit->createAndAddCnotGate(controlLineTwo, targetLine);
    ASSERT_THAT(createdCnotGate, testing::NotNull());

    auto expectedCnotGate      = std::make_shared<Gate>();
    expectedCnotGate->type     = Gate::Type::Toffoli;
    expectedCnotGate->controls = {controlLineOne, controlLineTwo};
    expectedCnotGate->targets.emplace(targetLine);
    assertThatGatesMatch(*expectedCnotGate, *createdCnotGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedCnotGate});
}

TEST_F(CircuitTestsFixture, AddCnotGateWithTargetLineMatchingDeactivatedControlLineOfPropagationScope) {
    constexpr unsigned numCircuitLines = 2;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne = 0;
    constexpr Gate::Line controlLineTwo = 1;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineOne);

    constexpr Gate::Line gateControlLine = controlLineTwo;
    constexpr Gate::Line targetLine      = controlLineOne;
    const auto           createdNotGate  = circuit->createAndAddCnotGate(gateControlLine, targetLine);
    ASSERT_THAT(createdNotGate, testing::NotNull());

    auto expectedNotGate  = std::make_shared<Gate>();
    expectedNotGate->type = Gate::Type::Toffoli;
    expectedNotGate->controls.emplace(gateControlLine);
    expectedNotGate->targets.emplace(targetLine);
    assertThatGatesMatch(*expectedNotGate, *createdNotGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedNotGate});
}

TEST_F(CircuitTestsFixture, AddCnotGateWithCallerProvidedControlLinesMatchingDeregisteredControlLinesOfParentScope) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne = 0;
    constexpr Gate::Line controlLineTwo = 1;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);

    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineOne);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineTwo);

    circuit->activateControlLinePropagationScope();

    auto createdCnotGate = circuit->createAndAddCnotGate(controlLineOne, controlLineTwo);
    ASSERT_THAT(createdCnotGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});

    constexpr Gate::Line targetLine       = controlLineTwo;
    constexpr Gate::Line controlLineThree = 2;

    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    createdCnotGate = circuit->createAndAddCnotGate(controlLineThree, targetLine);

    auto expectedCnotGate      = std::make_shared<Gate>();
    expectedCnotGate->type     = Gate::Type::Toffoli;
    expectedCnotGate->controls = {controlLineOne, controlLineThree};
    expectedCnotGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedCnotGate, *createdCnotGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedCnotGate});
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

TEST_F(CircuitTestsFixture, AddNotGateWithActiveControlLinesInParentControlLineScopes) {
    constexpr unsigned numCircuitLines = 5;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne   = 0;
    constexpr Gate::Line controlLineTwo   = 1;
    constexpr Gate::Line controlLineThree = 2;
    constexpr Gate::Line controlLineFour  = 3;
    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineThree);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineTwo);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineFour);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineThree);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineThree);

    constexpr Gate::Line targetLine     = 4;
    const auto           createdNotGate = circuit->createAndAddNotGate(targetLine);
    ASSERT_THAT(createdNotGate, testing::NotNull());

    auto expectedNotGate      = std::make_shared<Gate>();
    expectedNotGate->type     = Gate::Type::Toffoli;
    expectedNotGate->controls = {controlLineOne, controlLineFour};
    expectedNotGate->targets  = {targetLine};

    assertThatGatesMatch(*expectedNotGate, *createdNotGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedNotGate});
}

TEST_F(CircuitTestsFixture, AddNotGateWithTargetLineMatchingActiveControlLineInAnyParentControlLineScope) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne = 0;
    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);

    circuit->activateControlLinePropagationScope();

    constexpr Gate::Line targetLine     = controlLineOne;
    const auto           createdNotGate = circuit->createAndAddNotGate(targetLine);
    ASSERT_THAT(createdNotGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

TEST_F(CircuitTestsFixture, AddNotGateWithTargetLineMatchingDeactivatedControlLineOfControlLinePropagationScope) {
    constexpr unsigned numCircuitLines = 1;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne = 0;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineOne);

    constexpr Gate::Line targetLine     = controlLineOne;
    const auto           createdNotGate = circuit->createAndAddNotGate(targetLine);
    ASSERT_THAT(createdNotGate, testing::NotNull());

    auto expectedNotGate  = std::make_shared<Gate>();
    expectedNotGate->type = Gate::Type::Toffoli;
    expectedNotGate->targets.emplace(targetLine);
    assertThatGatesMatch(*expectedNotGate, *createdNotGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedNotGate});
}

TEST_F(CircuitTestsFixture, AddMultiControlToffoliGate) {
    constexpr unsigned numCircuitLines = 4;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line    controlLineOne   = 1;
    constexpr Gate::Line    controlLineTwo   = 3;
    constexpr Gate::Line    controlLineThree = 2;
    constexpr Gate::Line    targetLine       = 0;
    const Gate::LinesLookup gateControlLines     = {controlLineOne, controlLineTwo, controlLineThree};

    const auto createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate(gateControlLines, targetLine);
    ASSERT_THAT(createdMultiControlToffoliGate, testing::NotNull());

    auto expectedMultiControlToffoliGate = std::make_shared<Gate>();
    expectedMultiControlToffoliGate->type = Gate::Type::Toffoli;
    expectedMultiControlToffoliGate->controls = gateControlLines;
    expectedMultiControlToffoliGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedMultiControlToffoliGate, *createdMultiControlToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {createdMultiControlToffoliGate});
}

TEST_F(CircuitTestsFixture, AddMultiControlToffoliGateWithUnknownControlLine) {
    constexpr unsigned numCircuitLines = 4;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line    controlLineOne     = 1;
    constexpr Gate::Line    unknownControlLine = numCircuitLines + 1;
    constexpr Gate::Line    controlLineThree   = 2;
    constexpr Gate::Line    targetLine         = 0;
    const Gate::LinesLookup gateControlLines       = {controlLineOne, unknownControlLine, controlLineThree};

    const auto createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate(gateControlLines, targetLine);
    ASSERT_THAT(createdMultiControlToffoliGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

TEST_F(CircuitTestsFixture, AddMultiControlToffoliGateWithUnknownTargetLine) {
    constexpr unsigned numCircuitLines = 4;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line    controlLineOne   = 1;
    constexpr Gate::Line    controlLineTwo   = 3;
    constexpr Gate::Line    controlLineThree = 2;
    constexpr Gate::Line    targetLine       = numCircuitLines + 1;
    const Gate::LinesLookup gateControlLines     = {controlLineOne, controlLineTwo, controlLineThree};

    const auto createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate(gateControlLines, targetLine);
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

TEST_F(CircuitTestsFixture, AddMultiControlToffoliGateWithActiveControlLinesInParentControlLineScopes) {
    constexpr unsigned numCircuitLines = 5;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineTwo   = 1;
    constexpr Gate::Line controlLineThree = 2;
    constexpr Gate::Line controlLineOne   = 0;
    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineThree);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineTwo);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineThree);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineThree);

    constexpr Gate::Line gateControlLine                = 3;
    constexpr Gate::Line targetLine                     = 4;
    const auto           createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate({gateControlLine}, targetLine);
    ASSERT_THAT(createdMultiControlToffoliGate, testing::NotNull());

    auto expectedMultiControlToffoliGate      = std::make_shared<Gate>();
    expectedMultiControlToffoliGate->type     = Gate::Type::Toffoli;
    expectedMultiControlToffoliGate->controls = {controlLineOne, gateControlLine};
    expectedMultiControlToffoliGate->targets  = {targetLine};

    assertThatGatesMatch(*expectedMultiControlToffoliGate, *createdMultiControlToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedMultiControlToffoliGate});
}

TEST_F(CircuitTestsFixture, AddMultiControlToffoliGateWithTargetLineMatchingActiveControlLinesOfAnyParentControlLineScopes) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne = 0;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineOne);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);

    const Gate::LinesLookup gateControlLines              = {1, 2};
    constexpr Gate::Line    targetLine                    = controlLineOne;
    const auto              createMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate(gateControlLines, targetLine);
    ASSERT_THAT(createMultiControlToffoliGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

TEST_F(CircuitTestsFixture, AddMultiControlToffoliGateWithTargetLineBeingEqualToUserProvidedControlLine) {
    constexpr unsigned numCircuitLines = 4;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line    controlLineOne   = 1;
    constexpr Gate::Line    controlLineTwo   = 3;
    constexpr Gate::Line    controlLineThree = 2;
    constexpr Gate::Line    targetLine       = controlLineTwo;
    const Gate::LinesLookup gateControlLines = {controlLineOne, controlLineTwo, controlLineThree};

    const auto createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate(gateControlLines, targetLine);
    ASSERT_THAT(createdMultiControlToffoliGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

TEST_F(CircuitTestsFixture, AddMultiControlToffoliGateWithAggregateOfUserProvidedAndActiveControlLinesOfParentControlLinesScopesIsEmpty) {
    constexpr unsigned numCircuitLines = 4;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne   = 0;
    constexpr Gate::Line controlLineTwo   = 1;
    constexpr Gate::Line controlLineThree = 2;
    constexpr Gate::Line targetLine       = 3;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineThree);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineThree);

    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineThree);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineTwo);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineOne);

    const Gate::LinesLookup gateControlLines               = {controlLineOne, controlLineThree};
    const auto              createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate(gateControlLines, targetLine);
    ASSERT_THAT(createdMultiControlToffoliGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

TEST_F(CircuitTestsFixture, AddMultiControlToffoliGateWithTargetLineMatchingDeactivatedControlLineOfParentScope) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne   = 0;
    constexpr Gate::Line controlLineTwo   = 1;
    constexpr Gate::Line controlLineThree = 2;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineOne);

    // The fredkin gate should be created due to the target line only overlapping a deactivated control line in the current control line propagation scope
    constexpr Gate::Line    targetLine                     = controlLineOne;
    const Gate::LinesLookup gateControlLines               = {controlLineThree, controlLineTwo};
    const auto              createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate(gateControlLines, targetLine);
    ASSERT_THAT(createdMultiControlToffoliGate, testing::NotNull());

    auto expectedMultiControlToffoliGate     = std::make_shared<Gate>();
    expectedMultiControlToffoliGate->type    = Gate::Type::Toffoli;
    expectedMultiControlToffoliGate->controls = gateControlLines;
    expectedMultiControlToffoliGate->targets.emplace(targetLine);
    assertThatGatesMatch(*expectedMultiControlToffoliGate, *createdMultiControlToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedMultiControlToffoliGate});
}

TEST_F(CircuitTestsFixture, AddMultiControlToffoliGateWithCallerProvidedControlLinesMatchingDeregisteredControlLinesOfParentScope) {
    constexpr unsigned numCircuitLines = 4;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne   = 0;
    constexpr Gate::Line controlLineTwo   = 1;
    constexpr Gate::Line controlLineThree = 2;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineThree);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);

    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineOne);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineTwo);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineThree);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineThree);

    auto createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate({controlLineOne, controlLineTwo}, controlLineThree);
    ASSERT_THAT(createdMultiControlToffoliGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});

    constexpr Gate::Line targetLine      = controlLineThree;
    constexpr Gate::Line controlLineFour = 3;
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate({controlLineFour}, targetLine);

    auto expectedMultiControlToffoliGate      = std::make_shared<Gate>();
    expectedMultiControlToffoliGate->type     = Gate::Type::Toffoli;
    expectedMultiControlToffoliGate->controls = {controlLineFour, controlLineOne};
    expectedMultiControlToffoliGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedMultiControlToffoliGate, *createdMultiControlToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedMultiControlToffoliGate});
}

TEST_F(CircuitTestsFixture, AddFredkinGate) {
    constexpr unsigned numCircuitLines = 2;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line targetLineOne  = 0;
    constexpr Gate::Line targetLineTwo = 1;

    const auto createdFredkinGate = circuit->createAndAddFredkinGate(targetLineOne, targetLineTwo);
    ASSERT_THAT(createdFredkinGate, testing::NotNull());

    auto expectedFredkinGate = std::make_shared<Gate>();
    expectedFredkinGate->type = Gate::Type::Fredkin;
    expectedFredkinGate->targets = {targetLineOne, targetLineTwo};
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

TEST_F(CircuitTestsFixture, AddFredkinGateWithTargetLineMatchingActiveControlLineOfAnyParentScope) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne = 0;
    constexpr Gate::Line controlLineTwo = 1;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineOne);

    constexpr Gate::Line notOverlappingTargetLine = 2;
    constexpr Gate::Line overlappingTargetLine    = controlLineTwo;
    auto                 createdFredkinGate       = circuit->createAndAddFredkinGate(notOverlappingTargetLine, overlappingTargetLine);
    ASSERT_THAT(createdFredkinGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});

    createdFredkinGate = circuit->createAndAddFredkinGate(overlappingTargetLine, notOverlappingTargetLine);
    ASSERT_THAT(createdFredkinGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
    
    createdFredkinGate = circuit->createAndAddFredkinGate(overlappingTargetLine, overlappingTargetLine);
    ASSERT_THAT(createdFredkinGate, testing::IsNull());
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {});
}

TEST_F(CircuitTestsFixture, AddFredkinGateWithTargetLineMatchingDeactivatedControlLineOfParentScope) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne = 0;
    constexpr Gate::Line controlLineTwo = 1;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineOne);

    // The fredkin gate should be created due to the target line only overlapping a deactivated control line in the current control line propagation scope
    constexpr Gate::Line overlappingTargetLine    = controlLineOne;
    constexpr Gate::Line notOverlappingTargetLine = 2;
    const auto           firstCreatedFredkinGate  = circuit->createAndAddFredkinGate(notOverlappingTargetLine, overlappingTargetLine);
    ASSERT_THAT(firstCreatedFredkinGate, testing::NotNull());

    auto expectedFirstFredkinGate     = std::make_shared<Gate>();
    expectedFirstFredkinGate->type    = Gate::Type::Fredkin;
    expectedFirstFredkinGate->controls.emplace(controlLineTwo);
    expectedFirstFredkinGate->targets = {notOverlappingTargetLine, overlappingTargetLine};
    assertThatGatesMatch(*expectedFirstFredkinGate, *firstCreatedFredkinGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstFredkinGate});

    const auto secondCreatedFredkinGate = circuit->createAndAddFredkinGate(overlappingTargetLine, notOverlappingTargetLine);
    ASSERT_THAT(secondCreatedFredkinGate, testing::NotNull());

    auto expectedSecondFredkinGate     = std::make_shared<Gate>();
    expectedSecondFredkinGate->type    = Gate::Type::Fredkin;
    expectedSecondFredkinGate->controls.emplace(controlLineTwo);
    expectedSecondFredkinGate->targets = {overlappingTargetLine, notOverlappingTargetLine};
    assertThatGatesMatch(*expectedSecondFredkinGate, *secondCreatedFredkinGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstFredkinGate, expectedSecondFredkinGate});
}
// END AddXGate tests

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

// BEGIN Control line propagation scopes tests
TEST_F(CircuitTestsFixture, RegisterDuplicateControlLineOfParentScopeInLocalControlLineScope) {
    constexpr unsigned numCircuitLines = 2;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line parentScopeControlLine = 0;
    constexpr Gate::Line targetLine             = 1;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(parentScopeControlLine);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(parentScopeControlLine);

    const auto createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate({}, targetLine);
    ASSERT_THAT(createdMultiControlToffoliGate, testing::NotNull());

    auto expectedMultiControlToffoliGate      = std::make_shared<Gate>();
    expectedMultiControlToffoliGate->type     = Gate::Type::Toffoli;
    expectedMultiControlToffoliGate->controls = {parentScopeControlLine};
    expectedMultiControlToffoliGate->targets  = {targetLine};

    assertThatGatesMatch(*expectedMultiControlToffoliGate, *createdMultiControlToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedMultiControlToffoliGate});
}

TEST_F(CircuitTestsFixture, RegisterDuplicateControlLineDeactivatedOfParentScopeInLocalScope) {
    constexpr unsigned numCircuitLines = 2;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne = 0;
    constexpr Gate::Line targetLine             = 1;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineOne);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);

    const auto createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate({}, targetLine);
    ASSERT_THAT(createdMultiControlToffoliGate, testing::NotNull());

    auto expectedMultiControlToffoliGate      = std::make_shared<Gate>();
    expectedMultiControlToffoliGate->type     = Gate::Type::Toffoli;
    expectedMultiControlToffoliGate->controls = {controlLineOne};
    expectedMultiControlToffoliGate->targets  = {targetLine};

    assertThatGatesMatch(*expectedMultiControlToffoliGate, *createdMultiControlToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedMultiControlToffoliGate});
}

TEST_F(CircuitTestsFixture, RegisterControlLineNotKnownInCircuit) {
    constexpr unsigned numCircuitLines = 2;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line knownControlLine   = 1;
    constexpr Gate::Line unknownControlLine = 2;
    constexpr Gate::Line targetLine         = 0;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(unknownControlLine);

    const Gate::LinesLookup gateControlLines                   = {knownControlLine};
    const auto              createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate(gateControlLines, targetLine);
    ASSERT_THAT(createdMultiControlToffoliGate, testing::NotNull());

    auto expectedMultiControlToffoliGate      = std::make_shared<Gate>();
    expectedMultiControlToffoliGate->type     = Gate::Type::Toffoli;
    expectedMultiControlToffoliGate->controls = gateControlLines;
    expectedMultiControlToffoliGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedMultiControlToffoliGate, *createdMultiControlToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedMultiControlToffoliGate});
}

TEST_F(CircuitTestsFixture, DeregisterControLineOfLocalControlLineScope) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line activateControlLine    = 1;
    constexpr Gate::Line deactivatedControlLine = 2;
    constexpr Gate::Line targetLine             = 0;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(deactivatedControlLine);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(activateControlLine);
    circuit->deregisterControlLineFromPropagationInCurrentScope(deactivatedControlLine);

    const Gate::LinesLookup gateControlLines                   = {deactivatedControlLine, activateControlLine};
    const auto              createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate(gateControlLines, targetLine);
    ASSERT_THAT(createdMultiControlToffoliGate, testing::NotNull());

    auto expectedMultiControlToffoliGate  = std::make_shared<Gate>();
    expectedMultiControlToffoliGate->type = Gate::Type::Toffoli;
    expectedMultiControlToffoliGate->controls.emplace(activateControlLine);
    expectedMultiControlToffoliGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedMultiControlToffoliGate, *createdMultiControlToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedMultiControlToffoliGate});
}

TEST_F(CircuitTestsFixture, DeregisterControLineOfParentScopeInLastActivateControlLineScope) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line activateControlLine    = 1;
    constexpr Gate::Line deactivatedControlLine = 2;
    constexpr Gate::Line targetLine             = 0;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(deactivatedControlLine);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(activateControlLine);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(deactivatedControlLine);
    circuit->deregisterControlLineFromPropagationInCurrentScope(deactivatedControlLine);

    const Gate::LinesLookup gateControlLines                   = {activateControlLine};
    const auto              createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate(gateControlLines, targetLine);
    ASSERT_THAT(createdMultiControlToffoliGate, testing::NotNull());

    auto expectedMultiControlToffoliGate      = std::make_shared<Gate>();
    expectedMultiControlToffoliGate->type     = Gate::Type::Toffoli;
    expectedMultiControlToffoliGate->controls = gateControlLines;
    expectedMultiControlToffoliGate->targets  = {targetLine};

    assertThatGatesMatch(*expectedMultiControlToffoliGate, *createdMultiControlToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedMultiControlToffoliGate});
}

TEST_F(CircuitTestsFixture, DeregisterControlLineNotKnownInCircuit) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line knownControlLine   = 1;
    constexpr Gate::Line unknownControlLine = 2;
    constexpr Gate::Line targetLine         = 0;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(unknownControlLine);
    circuit->deregisterControlLineFromPropagationInCurrentScope(unknownControlLine);

    const Gate::LinesLookup gateControlLines                   = {knownControlLine};
    const auto              createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate(gateControlLines, targetLine);
    ASSERT_THAT(createdMultiControlToffoliGate, testing::NotNull());

    auto expectedMultiControlToffoliGate      = std::make_shared<Gate>();
    expectedMultiControlToffoliGate->type     = Gate::Type::Toffoli;
    expectedMultiControlToffoliGate->controls = gateControlLines;
    expectedMultiControlToffoliGate->targets  = {targetLine};

    assertThatGatesMatch(*expectedMultiControlToffoliGate, *createdMultiControlToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedMultiControlToffoliGate});
}

TEST_F(CircuitTestsFixture, DeregisterControlLineOfParentPropagationScopeNotRegisteredInCurrentScope) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne = 0;
    constexpr Gate::Line controlLineTwo = 1;
    constexpr Gate::Line targetLine     = 2;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);

    // Deregistering a not registered control line should not modify the aggregate of all activate control lines
    circuit->activateControlLinePropagationScope();
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineTwo);

    const Gate::LinesLookup gateControlLines                   = {controlLineOne, controlLineTwo};
    const auto              createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate(gateControlLines, targetLine);
    ASSERT_THAT(createdMultiControlToffoliGate, testing::NotNull());

    auto expectedMultiControlToffoliGate      = std::make_shared<Gate>();
    expectedMultiControlToffoliGate->type     = Gate::Type::Toffoli;
    expectedMultiControlToffoliGate->controls = gateControlLines;
    expectedMultiControlToffoliGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedMultiControlToffoliGate, *createdMultiControlToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedMultiControlToffoliGate});
}

TEST_F(CircuitTestsFixture, RegisteringLocalControlLineDoesNotAddNewControlLinesToExistingGates) {
    constexpr unsigned numCircuitLines = 2;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne = 0;
    constexpr Gate::Line targetLine     = 1;

    circuit->activateControlLinePropagationScope();

    const auto createdNotGate = circuit->createAndAddNotGate(targetLine);
    ASSERT_THAT(createdNotGate, testing::NotNull());

    auto expectedNotGate  = std::make_shared<Gate>();
    expectedNotGate->type = Gate::Type::Toffoli;
    expectedNotGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedNotGate, *createdNotGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedNotGate});

    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedNotGate});
}

TEST_F(CircuitTestsFixture, DeactivatingLocalControlLineDoesNotAddNewControlLinesToExistingGates) {
    constexpr unsigned numCircuitLines = 2;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne = 0;
    constexpr Gate::Line targetLine     = 1;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);

    const auto createdNotGate = circuit->createAndAddNotGate(targetLine);
    ASSERT_THAT(createdNotGate, testing::NotNull());

    auto expectedNotGate  = std::make_shared<Gate>();
    expectedNotGate->type = Gate::Type::Toffoli;
    expectedNotGate->controls.emplace(controlLineOne);
    expectedNotGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedNotGate, *createdNotGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedNotGate});

    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineOne);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedNotGate});
}

TEST_F(CircuitTestsFixture, ActivatingControlLinePropagationScopeDoesNotAddNewControlLinesToExistingGates) {
    constexpr unsigned numCircuitLines = 2;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne = 0;
    constexpr Gate::Line targetLine     = 1;

    const auto createdNotGate = circuit->createAndAddNotGate(targetLine);
    ASSERT_THAT(createdNotGate, testing::NotNull());

    auto expectedNotGate  = std::make_shared<Gate>();
    expectedNotGate->type = Gate::Type::Toffoli;
    expectedNotGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedNotGate, *createdNotGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedNotGate});

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedNotGate});
}

TEST_F(CircuitTestsFixture, DeactivatingControlLinePropagationScopeDoesNotAddNewControlLinesToExistingGates) {
    constexpr unsigned numCircuitLines = 4;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne   = 0;
    constexpr Gate::Line controlLineTwo   = 1;
    constexpr Gate::Line controlLineThree = 2;
    constexpr Gate::Line targetLine       = 3;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);

    constexpr Gate::Line gateControlLine = controlLineThree;
    const auto           createdCnotGate  = circuit->createAndAddCnotGate(gateControlLine, targetLine);
    ASSERT_THAT(createdCnotGate, testing::NotNull());

    auto expectedCnotGate      = std::make_shared<Gate>();
    expectedCnotGate->type     = Gate::Type::Toffoli;
    expectedCnotGate->controls = {controlLineOne, controlLineTwo, gateControlLine};
    expectedCnotGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedCnotGate, *createdCnotGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedCnotGate});

    circuit->deactivateControlLinePropagationScope();
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedCnotGate});
}

TEST_F(CircuitTestsFixture, DeactivateControlLinePropagationScopeRegisteringControlLinesOfParentScope) {
    constexpr unsigned numCircuitLines = 4;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne   = 1;
    constexpr Gate::Line controlLineTwo   = 2;
    constexpr Gate::Line controlLineThree = 3;
    constexpr Gate::Line targetLine       = 0;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineThree);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineOne);
    circuit->deactivateControlLinePropagationScope();

    const auto createdNotGate = circuit->createAndAddNotGate(targetLine);
    ASSERT_THAT(createdNotGate, testing::NotNull());

    auto expectedNotGate      = std::make_shared<Gate>();
    expectedNotGate->type     = Gate::Type::Toffoli;
    expectedNotGate->controls = {controlLineOne, controlLineTwo};
    expectedNotGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedNotGate, *createdNotGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedNotGate});
}

TEST_F(CircuitTestsFixture, DeactivateControlLinePropagationScopeNotRegisteringControlLinesOfParentScope) {
    constexpr unsigned numCircuitLines = 4;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne   = 1;
    constexpr Gate::Line controlLineTwo   = 2;
    constexpr Gate::Line controlLineThree = 3;
    constexpr Gate::Line targetLine       = 0;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineThree);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineOne);
    circuit->deactivateControlLinePropagationScope();

    const auto createdNotGate = circuit->createAndAddNotGate(targetLine);
    ASSERT_THAT(createdNotGate, testing::NotNull());

    auto expectedNotGate      = std::make_shared<Gate>();
    expectedNotGate->type     = Gate::Type::Toffoli;
    expectedNotGate->controls = {controlLineOne, controlLineTwo};
    expectedNotGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedNotGate, *createdNotGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedNotGate});
}
// BEGIN Control line propagation scopes tests