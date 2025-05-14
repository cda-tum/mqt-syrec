/*
 * Copyright (c) 2023 - 2025 Chair for Design Automation, TUM
 * Copyright (c) 2025 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#include "core/circuit.hpp"
#include "core/gate.hpp"

#include <cstddef>
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <initializer_list>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

// The current tests do not cover the functionality:
// * set- and get constant/garbage/input/output lines
// * adding and getting lines of the circuit
// * the stringification of the supported gate types
// ** (Gate::toQasm() will generate outputs that are not supported by the QASM standard without extra definitions and only supported by MQT::Core)
// * the stringification of the whole circuit to either a string or file

using namespace syrec;

class CircuitTestsFixture: public testing::Test {
protected:
    struct GeneratedAndExpectedGatePair {
        Gate::ptr generatedGate;
        Gate::ptr expectedGate;
    };

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

        const auto* expectedCircuitGatesIterator = expectedCircuitGates.begin();
        auto        actualCircuitGatesIterator   = circuit.cbegin();
        for (std::size_t i = 0; i < numGatesInCircuit; ++i) {
            ASSERT_THAT(*expectedCircuitGatesIterator, testing::NotNull());
            ASSERT_THAT(*actualCircuitGatesIterator, testing::NotNull());
            assertThatGatesMatch(**expectedCircuitGatesIterator, **actualCircuitGatesIterator);
            ++expectedCircuitGatesIterator; // NOLINT (cppcoreguidelines-pro-bounds-pointer-arithmetic)
            ++actualCircuitGatesIterator;
        }
    }

    static void assertThatAnnotationsOfGateAreEqualTo(const Circuit& circuit, const Gate& gate, const std::optional<std::unordered_map<std::string, std::string>>& expectedAnnotationsOfGate) {
        const auto& actualAnnotationsOfGate = circuit.getAnnotations(gate);
        if (!expectedAnnotationsOfGate.has_value()) {
            ASSERT_FALSE(actualAnnotationsOfGate.has_value());
            return;
        }
        ASSERT_TRUE(actualAnnotationsOfGate.has_value());
        for (const auto& [expectedAnnotationKey, expectedAnnotationValue]: *expectedAnnotationsOfGate) {
            const auto& matchingActualAnnotationForKey = actualAnnotationsOfGate->find(expectedAnnotationKey);
            ASSERT_NE(matchingActualAnnotationForKey, actualAnnotationsOfGate->cend()) << "Annotation with key " << expectedAnnotationKey << " did not exist for gate";
            ASSERT_EQ(expectedAnnotationValue, matchingActualAnnotationForKey->second) << "Value of annotation with key " << expectedAnnotationKey << " did not match! Expected: " << expectedAnnotationValue << " Actual: " << matchingActualAnnotationForKey->second;
        }
    }

    static void createNotGateWithSingleTargetLine(Circuit& circuit, Gate::Line targetLine, GeneratedAndExpectedGatePair& generatedAndExpectedGatePair) {
        const auto& generatedNotGate = circuit.createAndAddNotGate(targetLine);
        ASSERT_THAT(generatedNotGate, testing::NotNull());

        auto expectedNotGate  = std::make_shared<Gate>();
        expectedNotGate->type = Gate::Type::Toffoli;
        expectedNotGate->targets.emplace(targetLine);
        assertThatGatesMatch(*generatedNotGate, *expectedNotGate);

        generatedAndExpectedGatePair.generatedGate = generatedNotGate;
        generatedAndExpectedGatePair.expectedGate  = expectedNotGate;
    }
};

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

    auto expectedToffoliGate  = std::make_shared<Gate>();
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
    constexpr unsigned numCircuitLines = 4;
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

    constexpr Gate::Line gateControlLine = 2;
    constexpr Gate::Line targetLine      = 3;
    // Both control lines of toffoli gate were deactivated in propagation scope
    auto createdToffoliGate = circuit->createAndAddToffoliGate(controlLineOne, controlLineTwo, targetLine);
    ASSERT_THAT(createdToffoliGate, testing::NotNull());

    auto expectedToffoliGateWithBothControlLinesDeregistered      = std::make_shared<Gate>();
    expectedToffoliGateWithBothControlLinesDeregistered->type     = Gate::Type::Toffoli;
    expectedToffoliGateWithBothControlLinesDeregistered->controls = {controlLineOne, controlLineTwo};
    expectedToffoliGateWithBothControlLinesDeregistered->targets.emplace(targetLine);
    assertThatGatesMatch(*expectedToffoliGateWithBothControlLinesDeregistered, *createdToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedToffoliGateWithBothControlLinesDeregistered});

    createdToffoliGate = circuit->createAndAddToffoliGate(controlLineOne, gateControlLine, targetLine);
    ASSERT_THAT(createdToffoliGate, testing::NotNull());

    auto secondExpectedToffoliGateWithOneDeregisteredControlLine      = std::make_shared<Gate>();
    secondExpectedToffoliGateWithOneDeregisteredControlLine->type     = Gate::Type::Toffoli;
    secondExpectedToffoliGateWithOneDeregisteredControlLine->controls = {controlLineOne, gateControlLine};
    secondExpectedToffoliGateWithOneDeregisteredControlLine->targets.emplace(targetLine);

    assertThatGatesMatch(*createdToffoliGate, *secondExpectedToffoliGateWithOneDeregisteredControlLine);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedToffoliGateWithBothControlLinesDeregistered, secondExpectedToffoliGateWithOneDeregisteredControlLine});

    createdToffoliGate = circuit->createAndAddToffoliGate(gateControlLine, controlLineOne, targetLine); // NOLINT(readability-suspicious-call-argument)
    ASSERT_THAT(createdToffoliGate, testing::NotNull());

    auto thirdExpectedToffoliGateWithOneDeregisteredControlLine      = std::make_shared<Gate>();
    thirdExpectedToffoliGateWithOneDeregisteredControlLine->type     = Gate::Type::Toffoli;
    thirdExpectedToffoliGateWithOneDeregisteredControlLine->controls = {gateControlLine, controlLineOne};
    thirdExpectedToffoliGateWithOneDeregisteredControlLine->targets.emplace(targetLine);

    assertThatGatesMatch(*createdToffoliGate, *thirdExpectedToffoliGateWithOneDeregisteredControlLine);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedToffoliGateWithBothControlLinesDeregistered, secondExpectedToffoliGateWithOneDeregisteredControlLine, thirdExpectedToffoliGateWithOneDeregisteredControlLine});
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
    const auto           createdToffoliGate = circuit->createAndAddToffoliGate(gateControlLineOne, gateControlLineTwo, targetLine);
    ASSERT_THAT(createdToffoliGate, testing::NotNull());

    auto expectedToffoliGate      = std::make_shared<Gate>();
    expectedToffoliGate->type     = Gate::Type::Toffoli;
    expectedToffoliGate->controls = {gateControlLineOne, gateControlLineTwo};
    expectedToffoliGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedToffoliGate, *createdToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedToffoliGate});
}

TEST_F(CircuitTestsFixture, AddToffoliGateWithCallerProvidedControlLinesMatchingDeregisteredControlLinesOfParentScope) {
    constexpr unsigned numCircuitLines = 5;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne   = 0;
    constexpr Gate::Line controlLineTwo   = 1;
    constexpr Gate::Line controlLineThree = 2;
    constexpr Gate::Line controlLineFour  = 3;
    constexpr Gate::Line targetLine       = 4;

    constexpr Gate::Line propagatedControlLine = controlLineThree;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineThree);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineFour);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(propagatedControlLine);
    circuit->deregisterControlLineFromPropagationInCurrentScope(propagatedControlLine);

    circuit->activateControlLinePropagationScope();

    constexpr Gate::Line gateControlLineOne      = controlLineOne;
    constexpr Gate::Line gateControlLineTwo      = controlLineTwo;
    const auto           firstCreatedToffoliGate = circuit->createAndAddToffoliGate(gateControlLineOne, gateControlLineTwo, targetLine);
    ASSERT_THAT(firstCreatedToffoliGate, testing::NotNull());

    auto expectedFirstToffoliGate      = std::make_shared<Gate>();
    expectedFirstToffoliGate->type     = Gate::Type::Toffoli;
    expectedFirstToffoliGate->controls = {gateControlLineOne, gateControlLineTwo, controlLineFour};
    expectedFirstToffoliGate->targets.emplace(targetLine);
    assertThatGatesMatch(*expectedFirstToffoliGate, *firstCreatedToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstToffoliGate});

    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(propagatedControlLine);
    const auto secondCreatedToffoliGate = circuit->createAndAddToffoliGate(gateControlLineOne, gateControlLineTwo, targetLine);

    auto expectedSecondToffoliGate      = std::make_shared<Gate>();
    expectedSecondToffoliGate->type     = Gate::Type::Toffoli;
    expectedSecondToffoliGate->controls = {propagatedControlLine, gateControlLineOne, gateControlLineTwo, controlLineFour};
    expectedSecondToffoliGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedSecondToffoliGate, *secondCreatedToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstToffoliGate, expectedSecondToffoliGate});
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
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne = 0;
    constexpr Gate::Line controlLineTwo = 1;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineOne);

    constexpr Gate::Line gateControlLine = controlLineTwo;
    constexpr Gate::Line targetLine      = 2;
    const auto           createdCnotGate = circuit->createAndAddCnotGate(gateControlLine, targetLine);
    ASSERT_THAT(createdCnotGate, testing::NotNull());

    auto expectedCnotGate  = std::make_shared<Gate>();
    expectedCnotGate->type = Gate::Type::Toffoli;
    expectedCnotGate->controls.emplace(gateControlLine);
    expectedCnotGate->targets.emplace(targetLine);
    assertThatGatesMatch(*expectedCnotGate, *createdCnotGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedCnotGate});
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

    constexpr Gate::Line controlLineOne   = 0;
    constexpr Gate::Line controlLineTwo   = 1;
    constexpr Gate::Line controlLineThree = 2;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);

    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineOne);
    circuit->deregisterControlLineFromPropagationInCurrentScope(controlLineTwo);

    circuit->activateControlLinePropagationScope();

    constexpr Gate::Line gateControlLine      = controlLineOne;
    constexpr Gate::Line targetLine           = controlLineThree;
    const auto           firstCreatedCnotGate = circuit->createAndAddCnotGate(gateControlLine, targetLine);
    ASSERT_THAT(firstCreatedCnotGate, testing::NotNull());

    auto expectedFirstCnotGate  = std::make_shared<Gate>();
    expectedFirstCnotGate->type = Gate::Type::Toffoli;
    expectedFirstCnotGate->controls.emplace(gateControlLine);
    expectedFirstCnotGate->targets.emplace(targetLine);
    assertThatGatesMatch(*expectedFirstCnotGate, *firstCreatedCnotGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstCnotGate});

    constexpr Gate::Line propagatedControlLine = controlLineTwo;
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(propagatedControlLine);
    const auto secondCreatedCnotGate = circuit->createAndAddCnotGate(gateControlLine, targetLine);

    auto expectedSecondCnotGate      = std::make_shared<Gate>();
    expectedSecondCnotGate->type     = Gate::Type::Toffoli;
    expectedSecondCnotGate->controls = {propagatedControlLine, gateControlLine};
    expectedSecondCnotGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedSecondCnotGate, *secondCreatedCnotGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstCnotGate, expectedSecondCnotGate});
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
    const Gate::LinesLookup gateControlLines = {controlLineOne, controlLineTwo, controlLineThree};

    const auto createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate(gateControlLines, targetLine);
    ASSERT_THAT(createdMultiControlToffoliGate, testing::NotNull());

    auto expectedMultiControlToffoliGate      = std::make_shared<Gate>();
    expectedMultiControlToffoliGate->type     = Gate::Type::Toffoli;
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
    const Gate::LinesLookup gateControlLines   = {controlLineOne, unknownControlLine, controlLineThree};

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
    const Gate::LinesLookup gateControlLines = {controlLineOne, controlLineTwo, controlLineThree};

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

    auto expectedMultiControlToffoliGate      = std::make_shared<Gate>();
    expectedMultiControlToffoliGate->type     = Gate::Type::Toffoli;
    expectedMultiControlToffoliGate->controls = gateControlLines;
    expectedMultiControlToffoliGate->targets.emplace(targetLine);
    assertThatGatesMatch(*expectedMultiControlToffoliGate, *createdMultiControlToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedMultiControlToffoliGate});
}

TEST_F(CircuitTestsFixture, AddMultiControlToffoliGateWithCallerProvidedControlLinesMatchingDeregisteredControlLinesOfParentScope) {
    constexpr unsigned numCircuitLines = 5;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line controlLineOne   = 0;
    constexpr Gate::Line controlLineTwo   = 1;
    constexpr Gate::Line controlLineThree = 2;
    constexpr Gate::Line controlLineFour  = 3;
    constexpr Gate::Line targetLine       = 4;

    constexpr Gate::Line propagatedControlLine    = controlLineThree;
    constexpr Gate::Line notPropagatedControlLine = controlLineFour;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineOne);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineTwo);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineThree);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(controlLineFour);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(propagatedControlLine);
    circuit->deregisterControlLineFromPropagationInCurrentScope(propagatedControlLine);

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(notPropagatedControlLine);
    circuit->deregisterControlLineFromPropagationInCurrentScope(notPropagatedControlLine);

    constexpr Gate::Line gateControlLineOne                  = propagatedControlLine;
    constexpr Gate::Line gateControlLineTwo                  = controlLineTwo;
    const auto           firstCreatedMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate({gateControlLineOne, gateControlLineTwo}, targetLine);
    ASSERT_THAT(firstCreatedMultiControlToffoliGate, testing::NotNull());

    auto expectedFirstMultiControlToffoliGate      = std::make_shared<Gate>();
    expectedFirstMultiControlToffoliGate->type     = Gate::Type::Toffoli;
    expectedFirstMultiControlToffoliGate->controls = {controlLineOne, gateControlLineOne, gateControlLineTwo};
    expectedFirstMultiControlToffoliGate->targets.emplace(targetLine);
    assertThatGatesMatch(*expectedFirstMultiControlToffoliGate, *firstCreatedMultiControlToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstMultiControlToffoliGate});

    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(propagatedControlLine);
    const auto secondCreatedMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate({gateControlLineTwo}, targetLine);

    auto expectedSecondCreatedToffoliGate      = std::make_shared<Gate>();
    expectedSecondCreatedToffoliGate->type     = Gate::Type::Toffoli;
    expectedSecondCreatedToffoliGate->controls = {controlLineOne, propagatedControlLine, gateControlLineTwo};
    expectedSecondCreatedToffoliGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedSecondCreatedToffoliGate, *secondCreatedMultiControlToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstMultiControlToffoliGate, expectedSecondCreatedToffoliGate});
}

TEST_F(CircuitTestsFixture, AddFredkinGate) {
    constexpr unsigned numCircuitLines = 2;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line targetLineOne = 0;
    constexpr Gate::Line targetLineTwo = 1;

    const auto createdFredkinGate = circuit->createAndAddFredkinGate(targetLineOne, targetLineTwo);
    ASSERT_THAT(createdFredkinGate, testing::NotNull());

    auto expectedFredkinGate     = std::make_shared<Gate>();
    expectedFredkinGate->type    = Gate::Type::Fredkin;
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

TEST_F(CircuitTestsFixture, AddFredkinGateWithTargetLinesTargetingSameLine) {
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

    auto expectedFirstFredkinGate  = std::make_shared<Gate>();
    expectedFirstFredkinGate->type = Gate::Type::Fredkin;
    expectedFirstFredkinGate->controls.emplace(controlLineTwo);
    expectedFirstFredkinGate->targets = {notOverlappingTargetLine, overlappingTargetLine};
    assertThatGatesMatch(*expectedFirstFredkinGate, *firstCreatedFredkinGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstFredkinGate});

    const auto secondCreatedFredkinGate = circuit->createAndAddFredkinGate(overlappingTargetLine, notOverlappingTargetLine);
    ASSERT_THAT(secondCreatedFredkinGate, testing::NotNull());

    auto expectedSecondFredkinGate  = std::make_shared<Gate>();
    expectedSecondFredkinGate->type = Gate::Type::Fredkin;
    expectedSecondFredkinGate->controls.emplace(controlLineTwo);
    expectedSecondFredkinGate->targets = {overlappingTargetLine, notOverlappingTargetLine};
    assertThatGatesMatch(*expectedSecondFredkinGate, *secondCreatedFredkinGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstFredkinGate, expectedSecondFredkinGate});
}
// END AddXGate tests

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
    constexpr Gate::Line targetLine     = 1;

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

    const Gate::LinesLookup gateControlLines               = {knownControlLine};
    const auto              createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate(gateControlLines, targetLine);
    ASSERT_THAT(createdMultiControlToffoliGate, testing::NotNull());

    auto expectedMultiControlToffoliGate      = std::make_shared<Gate>();
    expectedMultiControlToffoliGate->type     = Gate::Type::Toffoli;
    expectedMultiControlToffoliGate->controls = gateControlLines;
    expectedMultiControlToffoliGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedMultiControlToffoliGate, *createdMultiControlToffoliGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedMultiControlToffoliGate});
}

TEST_F(CircuitTestsFixture, DeregisterControlLineOfLocalControlLineScope) {
    constexpr unsigned numCircuitLines = 3;
    circuit->setLines(numCircuitLines);

    constexpr Gate::Line targetLine             = 0;
    constexpr Gate::Line activateControlLine    = 1;
    constexpr Gate::Line deactivatedControlLine = 2;

    circuit->activateControlLinePropagationScope();
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(deactivatedControlLine);
    circuit->registerControlLineForPropagationInCurrentAndNestedScopes(activateControlLine);
    circuit->deregisterControlLineFromPropagationInCurrentScope(deactivatedControlLine);

    const auto createdNotGate = circuit->createAndAddNotGate(targetLine);
    ASSERT_THAT(createdNotGate, testing::NotNull());

    auto expectedNotGate  = std::make_shared<Gate>();
    expectedNotGate->type = Gate::Type::Toffoli;
    expectedNotGate->controls.emplace(activateControlLine);
    expectedNotGate->targets.emplace(targetLine);

    assertThatGatesMatch(*expectedNotGate, *createdNotGate);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedNotGate});
}

TEST_F(CircuitTestsFixture, DeregisterControlLineOfParentScopeInLastActivateControlLineScope) {
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

    const Gate::LinesLookup gateControlLines               = {activateControlLine};
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

    const Gate::LinesLookup gateControlLines               = {knownControlLine};
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

    const Gate::LinesLookup gateControlLines               = {controlLineOne};
    const auto              createdMultiControlToffoliGate = circuit->createAndAddMultiControlToffoliGate(gateControlLines, targetLine);
    ASSERT_THAT(createdMultiControlToffoliGate, testing::NotNull());

    auto expectedMultiControlToffoliGate      = std::make_shared<Gate>();
    expectedMultiControlToffoliGate->type     = Gate::Type::Toffoli;
    expectedMultiControlToffoliGate->controls = {controlLineOne, controlLineTwo};
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
    const auto           createdCnotGate = circuit->createAndAddCnotGate(gateControlLine, targetLine);
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

// BEGIN Annotation tests
TEST_F(CircuitTestsFixture, SetAnnotationForGate) {
    circuit->setLines(2);

    constexpr Gate::Line         targetLineOne = 0;
    GeneratedAndExpectedGatePair firstGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineOne, firstGeneratedNotGatePairData);
    auto [firstGeneratedNotGate, expectedFirstNotGate] = firstGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate});
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, std::nullopt);

    constexpr Gate::Line         targetLineTwo = 1;
    GeneratedAndExpectedGatePair secondGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineTwo, secondGeneratedNotGatePairData);
    auto [secondGeneratedNotGate, expectedSecondNotGate] = secondGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate, expectedSecondNotGate});
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *secondGeneratedNotGate, std::nullopt);

    const std::string annotationKey          = "KEY";
    const std::string initialAnnotationValue = "InitialValue";
    circuit->annotate(*firstGeneratedNotGate, annotationKey, initialAnnotationValue);

    const std::unordered_map<std::string, std::string> expectedAnnotationsOfFirstGate = {{annotationKey, initialAnnotationValue}};
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *secondGeneratedNotGate, std::nullopt);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate, expectedSecondNotGate});
}

TEST_F(CircuitTestsFixture, UpdateAnnotationForGate) {
    circuit->setLines(2);

    constexpr Gate::Line         targetLineOne = 0;
    GeneratedAndExpectedGatePair firstGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineOne, firstGeneratedNotGatePairData);
    auto [firstGeneratedNotGate, expectedFirstNotGate] = firstGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate});
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, std::nullopt);

    constexpr Gate::Line         targetLineTwo = 1;
    GeneratedAndExpectedGatePair secondGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineTwo, secondGeneratedNotGatePairData);
    auto [secondGeneratedNotGate, expectedSecondNotGate] = secondGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate, expectedSecondNotGate});
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *secondGeneratedNotGate, std::nullopt);

    const std::string firstAnnotationKey          = "KEY_ONE";
    const std::string initialFirstAnnotationValue = "InitialValue";

    const std::string secondAnnotationKey          = "KEY_TWO";
    const std::string initialSecondAnnotationValue = "OtherValue";
    circuit->annotate(*firstGeneratedNotGate, firstAnnotationKey, initialFirstAnnotationValue);
    circuit->annotate(*firstGeneratedNotGate, secondAnnotationKey, initialSecondAnnotationValue);

    std::unordered_map<std::string, std::string> expectedAnnotationsOfFirstGate = {{firstAnnotationKey, initialFirstAnnotationValue}, {secondAnnotationKey, initialSecondAnnotationValue}};
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *secondGeneratedNotGate, std::nullopt);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate, expectedSecondNotGate});

    const std::string updatedAnnotationValue = "UpdatedValue";
    circuit->annotate(*firstGeneratedNotGate, firstAnnotationKey, updatedAnnotationValue);

    expectedAnnotationsOfFirstGate[firstAnnotationKey] = updatedAnnotationValue;
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *secondGeneratedNotGate, std::nullopt);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate, expectedSecondNotGate});
}

TEST_F(CircuitTestsFixture, SetAnnotationForUnknownGate) {
    circuit->setLines(2);

    constexpr Gate::Line         targetLineOne = 0;
    GeneratedAndExpectedGatePair firstGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineOne, firstGeneratedNotGatePairData);
    auto [firstGeneratedNotGate, expectedFirstNotGate] = firstGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate});
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, std::nullopt);

    constexpr Gate::Line         targetLineTwo = 1;
    GeneratedAndExpectedGatePair secondGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineTwo, secondGeneratedNotGatePairData);
    auto [secondGeneratedNotGate, expectedSecondNotGate] = secondGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate, expectedSecondNotGate});
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *secondGeneratedNotGate, std::nullopt);

    const std::string annotationKey   = "KEY";
    const std::string annotationValue = "VALUE";
    const auto        unknownGate     = std::make_shared<Gate>();
    unknownGate->type                 = Gate::Type::Toffoli;

    circuit->annotate(*unknownGate, annotationKey, annotationValue);
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, std::nullopt);
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *secondGeneratedNotGate, std::nullopt);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate, expectedSecondNotGate});
}

TEST_F(CircuitTestsFixture, UpdateNotExistingAnnotationForGate) {
    circuit->setLines(2);

    constexpr Gate::Line         targetLineOne = 0;
    GeneratedAndExpectedGatePair firstGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineOne, firstGeneratedNotGatePairData);
    auto [firstGeneratedNotGate, expectedFirstNotGate] = firstGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate});
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, std::nullopt);

    constexpr Gate::Line         targetLineTwo = 1;
    GeneratedAndExpectedGatePair secondGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineTwo, secondGeneratedNotGatePairData);
    auto [secondGeneratedNotGate, expectedSecondNotGate] = secondGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate, expectedSecondNotGate});
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *secondGeneratedNotGate, std::nullopt);

    const std::string firstAnnotationKey          = "KEY_ONE";
    const std::string initialFirstAnnotationValue = "InitialValue";
    circuit->annotate(*firstGeneratedNotGate, firstAnnotationKey, initialFirstAnnotationValue);

    std::unordered_map<std::string, std::string> expectedAnnotationsOfFirstGate = {{firstAnnotationKey, initialFirstAnnotationValue}};
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *secondGeneratedNotGate, std::nullopt);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate, expectedSecondNotGate});

    const std::string secondAnnotationKey          = "KEY_TWO";
    const std::string initialSecondAnnotationValue = "OtherValue";
    circuit->annotate(*firstGeneratedNotGate, secondAnnotationKey, initialSecondAnnotationValue);
    expectedAnnotationsOfFirstGate[secondAnnotationKey] = initialSecondAnnotationValue;

    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *secondGeneratedNotGate, std::nullopt);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate, expectedSecondNotGate});
}

TEST_F(CircuitTestsFixture, SetAnnotationForGateWithEmptyKey) {
    circuit->setLines(2);

    constexpr Gate::Line         targetLineOne = 0;
    GeneratedAndExpectedGatePair firstGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineOne, firstGeneratedNotGatePairData);
    auto [firstGeneratedNotGate, expectedFirstNotGate] = firstGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate});
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, std::nullopt);

    constexpr Gate::Line         targetLineTwo = 1;
    GeneratedAndExpectedGatePair secondGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineTwo, secondGeneratedNotGatePairData);
    auto [secondGeneratedNotGate, expectedSecondNotGate] = secondGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate, expectedSecondNotGate});
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *secondGeneratedNotGate, std::nullopt);

    const std::string firstAnnotationKey          = "KEY_ONE";
    const std::string initialFirstAnnotationValue = "InitialValue";
    circuit->annotate(*firstGeneratedNotGate, firstAnnotationKey, initialFirstAnnotationValue);

    std::unordered_map<std::string, std::string> expectedAnnotationsOfFirstGate = {{firstAnnotationKey, initialFirstAnnotationValue}};
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *secondGeneratedNotGate, std::nullopt);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate, expectedSecondNotGate});

    const std::string valueForAnnotationWithEmptyKey = "OtherValue";
    circuit->annotate(*firstGeneratedNotGate, "", valueForAnnotationWithEmptyKey);
    expectedAnnotationsOfFirstGate.emplace("", valueForAnnotationWithEmptyKey);
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *secondGeneratedNotGate, std::nullopt);
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate, expectedSecondNotGate});
}

TEST_F(CircuitTestsFixture, SetGlobalGateAnnotation) {
    circuit->setLines(2);

    constexpr Gate::Line         targetLineOne = 0;
    GeneratedAndExpectedGatePair firstGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineOne, firstGeneratedNotGatePairData);
    auto [firstGeneratedNotGate, expectedFirstNotGate] = firstGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate});
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, std::nullopt);

    const std::string globalAnnotationKey   = "KEY_ONE";
    const std::string globalAnnotationValue = "InitialValue";
    ASSERT_FALSE(circuit->setOrUpdateGlobalGateAnnotation(globalAnnotationKey, globalAnnotationValue));
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, std::nullopt);

    constexpr Gate::Line         targetLineTwo = 1;
    GeneratedAndExpectedGatePair secondGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineTwo, secondGeneratedNotGatePairData);
    auto [secondGeneratedNotGate, expectedSecondNotGate] = secondGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate, expectedSecondNotGate});

    const std::unordered_map<std::string, std::string> expectedAnnotationsOfSecondGate = {{globalAnnotationKey, globalAnnotationValue}};
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, std::nullopt);
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *secondGeneratedNotGate, expectedAnnotationsOfSecondGate);
}

TEST_F(CircuitTestsFixture, UpdateGlobalGateAnnotation) {
    circuit->setLines(2);

    const std::string globalAnnotationKey          = "KEY_ONE";
    const std::string initialGlobalAnnotationValue = "InitialValue";
    ASSERT_FALSE(circuit->setOrUpdateGlobalGateAnnotation(globalAnnotationKey, initialGlobalAnnotationValue));

    constexpr Gate::Line         targetLineOne = 0;
    GeneratedAndExpectedGatePair firstGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineOne, firstGeneratedNotGatePairData);
    auto [firstGeneratedNotGate, expectedFirstNotGate] = firstGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate});

    const std::unordered_map<std::string, std::string> expectedAnnotationsOfFirstGate = {{globalAnnotationKey, initialGlobalAnnotationValue}};
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);

    const std::string updatedGlobalAnnoatationValue = "UpdatedValue";
    ASSERT_TRUE(circuit->setOrUpdateGlobalGateAnnotation(globalAnnotationKey, updatedGlobalAnnoatationValue));

    constexpr Gate::Line         targetLineTwo = 1;
    GeneratedAndExpectedGatePair secondGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineTwo, secondGeneratedNotGatePairData);
    auto [secondGeneratedNotGate, expectedSecondNotGate] = secondGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate, expectedSecondNotGate});

    const std::unordered_map<std::string, std::string> expectedAnnotationsOfSecondGate = {{globalAnnotationKey, updatedGlobalAnnoatationValue}};
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *secondGeneratedNotGate, expectedAnnotationsOfSecondGate);
}

TEST_F(CircuitTestsFixture, UpdateNotExistingGlobalGateAnnotation) {
    circuit->setLines(2);

    const std::string firstGlobalAnnotationKey   = "KEY_ONE";
    const std::string firstGlobalAnnotationValue = "InitialValue";
    ASSERT_FALSE(circuit->setOrUpdateGlobalGateAnnotation(firstGlobalAnnotationKey, firstGlobalAnnotationValue));

    constexpr Gate::Line         targetLineOne = 0;
    GeneratedAndExpectedGatePair firstGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineOne, firstGeneratedNotGatePairData);
    auto [firstGeneratedNotGate, expectedFirstNotGate] = firstGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate});

    const std::unordered_map<std::string, std::string> expectedAnnotationsOfFirstGate = {{firstGlobalAnnotationKey, firstGlobalAnnotationValue}};
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);

    const std::string secondGlobalAnnotationKey   = "KEY_TWO";
    const std::string secondGlobalAnnotationValue = "OtherValue";
    ASSERT_FALSE(circuit->setOrUpdateGlobalGateAnnotation(secondGlobalAnnotationKey, secondGlobalAnnotationValue));

    constexpr Gate::Line         targetLineTwo = 1;
    GeneratedAndExpectedGatePair secondGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineTwo, secondGeneratedNotGatePairData);
    auto [secondGeneratedNotGate, expectedSecondNotGate] = secondGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate, expectedSecondNotGate});

    const std::unordered_map<std::string, std::string> expectedAnnotationsOfSecondGate = {{firstGlobalAnnotationKey, firstGlobalAnnotationValue}, {secondGlobalAnnotationKey, secondGlobalAnnotationValue}};
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *secondGeneratedNotGate, expectedAnnotationsOfSecondGate);
}

TEST_F(CircuitTestsFixture, RemoveGlobalGateAnnotation) {
    circuit->setLines(2);

    const std::string globalAnnotationKey          = "KEY_ONE";
    const std::string initialGlobalAnnotationValue = "InitialValue";
    ASSERT_FALSE(circuit->setOrUpdateGlobalGateAnnotation(globalAnnotationKey, initialGlobalAnnotationValue));

    constexpr Gate::Line         targetLineOne = 0;
    GeneratedAndExpectedGatePair firstGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineOne, firstGeneratedNotGatePairData);
    auto [firstGeneratedNotGate, expectedFirstNotGate] = firstGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate});

    const std::unordered_map<std::string, std::string> expectedAnnotationsOfFirstGate = {{globalAnnotationKey, initialGlobalAnnotationValue}};
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);

    ASSERT_TRUE(circuit->removeGlobalGateAnnotation(globalAnnotationKey));

    constexpr Gate::Line         targetLineTwo = 1;
    GeneratedAndExpectedGatePair secondGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineTwo, secondGeneratedNotGatePairData);
    auto [secondGeneratedNotGate, expectedSecondNotGate] = secondGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate, expectedSecondNotGate});

    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *secondGeneratedNotGate, {});
}

TEST_F(CircuitTestsFixture, SetGlobalGateAnnotationWithEmptyKey) {
    circuit->setLines(2);

    const std::string globalAnnotationKey          = "KEY_ONE";
    const std::string initialGlobalAnnotationValue = "InitialValue";
    ASSERT_FALSE(circuit->setOrUpdateGlobalGateAnnotation(globalAnnotationKey, initialGlobalAnnotationValue));

    constexpr Gate::Line         targetLineOne = 0;
    GeneratedAndExpectedGatePair firstGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineOne, firstGeneratedNotGatePairData);
    auto [firstGeneratedNotGate, expectedFirstNotGate] = firstGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate});

    const std::unordered_map<std::string, std::string> expectedAnnotationsOfFirstGate = {{globalAnnotationKey, initialGlobalAnnotationValue}};
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);

    const std::string valueOfAnnotationWithEmptyKey = "OtherValue";
    ASSERT_FALSE(circuit->setOrUpdateGlobalGateAnnotation("", valueOfAnnotationWithEmptyKey));

    constexpr Gate::Line         targetLineTwo = 1;
    GeneratedAndExpectedGatePair secondGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineTwo, secondGeneratedNotGatePairData);
    auto [secondGeneratedNotGate, expectedSecondNotGate] = secondGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate, expectedSecondNotGate});

    const std::unordered_map<std::string, std::string> expectedAnnotationsOfSecondGate = {{"", valueOfAnnotationWithEmptyKey}};
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *secondGeneratedNotGate, expectedAnnotationsOfSecondGate);
}

TEST_F(CircuitTestsFixture, SetGlobalGateAnnotationMatchingExistingAnnotationOfGateDoesNotUpdateTheLatter) {
    circuit->setLines(2);

    constexpr Gate::Line         targetLineOne = 0;
    GeneratedAndExpectedGatePair firstGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineOne, firstGeneratedNotGatePairData);
    auto [firstGeneratedNotGate, expectedFirstNotGate] = firstGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate});
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, std::nullopt);

    const std::string localAnnotationKey   = "KEY_ONE";
    const std::string localAnnotationValue = "LocalValue";
    circuit->annotate(*firstGeneratedNotGate, localAnnotationKey, localAnnotationValue);
    const std::unordered_map<std::string, std::string> expectedAnnotationsOfFirstGate = {{localAnnotationKey, localAnnotationValue}};
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);

    const std::string& globalAnnotationKey   = localAnnotationKey;
    const std::string  globalAnnotationValue = "InitialValue";
    ASSERT_FALSE(circuit->setOrUpdateGlobalGateAnnotation(globalAnnotationKey, globalAnnotationValue));
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);

    constexpr Gate::Line         targetLineTwo = 1;
    GeneratedAndExpectedGatePair secondGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineTwo, secondGeneratedNotGatePairData);
    auto [secondGeneratedNotGate, expectedSecondNotGate] = secondGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate, expectedSecondNotGate});

    const std::unordered_map<std::string, std::string> expectedAnnotationsOfSecondGate = {{globalAnnotationKey, globalAnnotationValue}};
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *secondGeneratedNotGate, expectedAnnotationsOfSecondGate);
}

TEST_F(CircuitTestsFixture, RemovingGlobalGateAnnotationMatchingExistingAnnotationOfGateDoesNotRemoveTheLatter) {
    circuit->setLines(2);

    constexpr Gate::Line         targetLineOne = 0;
    GeneratedAndExpectedGatePair firstGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineOne, firstGeneratedNotGatePairData);
    auto [firstGeneratedNotGate, expectedFirstNotGate] = firstGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate});
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, std::nullopt);

    const std::string localAnnotationKey   = "KEY_ONE";
    const std::string localAnnotationValue = "LocalValue";
    circuit->annotate(*firstGeneratedNotGate, localAnnotationKey, localAnnotationValue);
    const std::unordered_map<std::string, std::string> expectedAnnotationsOfFirstGate = {{localAnnotationKey, localAnnotationValue}};
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);

    const std::string& globalAnnotationKey   = localAnnotationKey;
    const std::string  globalAnnotationValue = "InitialValue";
    ASSERT_FALSE(circuit->setOrUpdateGlobalGateAnnotation(globalAnnotationKey, globalAnnotationValue));
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);
    ASSERT_TRUE(circuit->removeGlobalGateAnnotation(globalAnnotationKey));

    constexpr Gate::Line         targetLineTwo = 1;
    GeneratedAndExpectedGatePair secondGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineTwo, secondGeneratedNotGatePairData);
    auto [secondGeneratedNotGate, expectedSecondNotGate] = secondGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate, expectedSecondNotGate});

    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *secondGeneratedNotGate, std::nullopt);
}

TEST_F(CircuitTestsFixture, UpdateLocalAnnotationWhoseKeyMatchesGlobalAnnotationDoesOnlyUpdateLocalAnnotation) {
    circuit->setLines(2);

    constexpr Gate::Line         targetLineOne = 0;
    GeneratedAndExpectedGatePair firstGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineOne, firstGeneratedNotGatePairData);
    auto [firstGeneratedNotGate, expectedFirstNotGate] = firstGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate});
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, std::nullopt);

    const std::string localAnnotationKey   = "KEY_ONE";
    const std::string localAnnotationValue = "LocalValue";
    circuit->annotate(*firstGeneratedNotGate, localAnnotationKey, localAnnotationValue);
    const std::unordered_map<std::string, std::string> expectedAnnotationsOfFirstGate = {{localAnnotationKey, localAnnotationValue}};
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);

    const std::string& globalAnnotationKey   = localAnnotationKey;
    const std::string  globalAnnotationValue = "InitialValue";
    ASSERT_FALSE(circuit->setOrUpdateGlobalGateAnnotation(globalAnnotationKey, globalAnnotationValue));
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);

    constexpr Gate::Line         targetLineTwo = 1;
    GeneratedAndExpectedGatePair secondGeneratedNotGatePairData;
    createNotGateWithSingleTargetLine(*circuit, targetLineTwo, secondGeneratedNotGatePairData);
    auto [secondGeneratedNotGate, expectedSecondNotGate] = secondGeneratedNotGatePairData;
    assertThatGatesOfCircuitAreEqualToSequence(*circuit, {expectedFirstNotGate, expectedSecondNotGate});

    std::unordered_map<std::string, std::string> expectedAnnotationsOfSecondGate = {{globalAnnotationKey, globalAnnotationValue}};
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *secondGeneratedNotGate, expectedAnnotationsOfSecondGate);

    const std::string updatedLocalAnnotationValue = "UpdatedValue";
    circuit->annotate(*secondGeneratedNotGate, localAnnotationKey, updatedLocalAnnotationValue);
    expectedAnnotationsOfSecondGate[localAnnotationKey] = updatedLocalAnnotationValue;

    assertThatAnnotationsOfGateAreEqualTo(*circuit, *firstGeneratedNotGate, expectedAnnotationsOfFirstGate);
    assertThatAnnotationsOfGateAreEqualTo(*circuit, *secondGeneratedNotGate, expectedAnnotationsOfSecondGate);
}
// END Annotation tests
