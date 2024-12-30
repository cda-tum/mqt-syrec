#include "core/syrec/variable.hpp"
#include "core/syrec/variable.hpp"
#include "core/syrec/parser/utils/symbolTable/base_symbol_table.hpp"

#include "gmock/gmock-matchers.h"
#include <gtest/gtest.h>

using namespace utils;

namespace {
    using ExpectedSymbolTableEntry = TemporaryVariableScope::ScopeEntry::readOnylPtr;

    constexpr unsigned int          DEFAULT_BITWIDTH            = 16;
    const std::vector<unsigned int> DEFAULT_VARIABLE_DIMENSIONS = {2, 1};

    class SingleVariableTypeTestFixture : public testing::TestWithParam<std::pair<syrec::Variable::Type, syrec::Variable::Type>> {
    protected:
        syrec::Variable::Type variableTypeUnderTest;
        syrec::Variable::Type otherVariableType;

        SingleVariableTypeTestFixture():
            variableTypeUnderTest(std::get<0>(GetParam())), otherVariableType(std::get<1>(GetParam())) {}
    };

    void assertCreationOfTemporaryVariableScopeSuccedds(BaseSymbolTable symbolTable, TemporaryVariableScope::ptr& openedScope) {
        ASSERT_NO_FATAL_FAILURE(openedScope = symbolTable.openTemporaryScope());
        ASSERT_THAT(openedScope, testing::NotNull());
    }

    void assertVariableInsertionResultMatchesExpectedOne(TemporaryVariableScope& temporaryVariableScope, const std::variant<syrec::Number::ptr, syrec::Variable::ptr>& variableVariant, bool shouldInsertionSucceed) {
        bool variableInsertionResult = false;
        if (std::holds_alternative<syrec::Number::ptr>(variableVariant)) {
            const auto& loopVariable = std::get<syrec::Number::ptr>(variableVariant);
            if (loopVariable)
                ASSERT_TRUE(loopVariable->isLoopVariable());

            ASSERT_NO_FATAL_FAILURE(variableInsertionResult = temporaryVariableScope.recordLoopVariable(loopVariable));
        } else {
            ASSERT_NO_FATAL_FAILURE(variableInsertionResult = temporaryVariableScope.recordVariable(std::get<syrec::Variable::ptr>(variableVariant)));
        }
        ASSERT_EQ(shouldInsertionSucceed, variableInsertionResult);
    }

    void assertVariableInsertionResultIsSuccessful(TemporaryVariableScope& temporaryVariableScope, const std::variant<syrec::Number::ptr, syrec::Variable::ptr>& variableVariant) {
        ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultMatchesExpectedOne(temporaryVariableScope, variableVariant, true));
    }

    void assertVariableInsertionResultIsNotSuccessful(TemporaryVariableScope& temporaryVariableScope, const std::variant<syrec::Number::ptr, syrec::Variable::ptr>& variableVariant) {
        ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultMatchesExpectedOne(temporaryVariableScope, variableVariant, false));
    }

    void assertFetchedEntryFromSymbolTableMatchesExpectedOne(const std::optional<ExpectedSymbolTableEntry>& expectedEntry, const std::optional<ExpectedSymbolTableEntry>& actualEntry) {
        ASSERT_EQ(expectedEntry.has_value(), actualEntry.has_value());
        if (!expectedEntry.has_value())
            return;

        const TemporaryVariableScope::ScopeEntry& expectedEntryRef = **expectedEntry;
        const TemporaryVariableScope::ScopeEntry& actualEntryRef   = **actualEntry;
        ASSERT_EQ(expectedEntryRef.getVariableIdentifier(), actualEntryRef.getVariableIdentifier());
        ASSERT_EQ(expectedEntryRef.isReferenceToLoopVariable(), actualEntryRef.isReferenceToLoopVariable());

        if (expectedEntryRef.isReferenceToLoopVariable()) {
            ASSERT_FALSE(actualEntryRef.getReadonlyVariableData().has_value());
            ASSERT_TRUE(actualEntryRef.getReadOnlyLoopVariableData().has_value());

            const std::shared_ptr<const syrec::Number> loopVariableData = actualEntryRef.getReadOnlyLoopVariableData().value();
            ASSERT_THAT(loopVariableData, testing::NotNull());
            ASSERT_TRUE(loopVariableData->isLoopVariable());
            ASSERT_EQ(expectedEntryRef.getVariableIdentifier(), loopVariableData->variableName());

            ASSERT_FALSE(actualEntryRef.getDeclaredVariableBitwidth().has_value());
            ASSERT_THAT(actualEntryRef.getDeclaredVariableDimensions(), testing::ElementsAre(1));
        } else {
            ASSERT_EQ(expectedEntryRef.getDeclaredVariableBitwidth(), actualEntryRef.getDeclaredVariableBitwidth());
            ASSERT_THAT(actualEntryRef.getDeclaredVariableDimensions(), testing::ElementsAreArray(expectedEntryRef.getDeclaredVariableDimensions()));
            ASSERT_FALSE(actualEntryRef.getReadOnlyLoopVariableData().has_value());
            ASSERT_TRUE(actualEntryRef.getReadonlyVariableData().has_value());

            const std::shared_ptr<const syrec::Variable> variableData = actualEntryRef.getReadonlyVariableData().value();
            ASSERT_THAT(variableData, testing::NotNull());
            ASSERT_EQ(expectedEntryRef.getReadonlyVariableData()->get()->type, variableData->type);
            ASSERT_EQ(expectedEntryRef.getVariableIdentifier(), variableData->name);
            ASSERT_EQ(expectedEntryRef.getDeclaredVariableBitwidth(), variableData->bitwidth);
            ASSERT_THAT(variableData->dimensions, testing::ElementsAreArray(expectedEntryRef.getDeclaredVariableDimensions()));
        }
    }

    bool operator==(const ExpectedSymbolTableEntry& lOperand, const ExpectedSymbolTableEntry& rOperand) {
        if (lOperand == nullptr)
            return rOperand == nullptr;

        if (lOperand->getVariableIdentifier() != rOperand->getVariableIdentifier() || lOperand->isReferenceToLoopVariable() != rOperand->isReferenceToLoopVariable())
            return false;

        if (lOperand->isReferenceToLoopVariable()) {
            if (rOperand->getReadonlyVariableData().has_value() || !rOperand->getReadOnlyLoopVariableData().has_value())
                return false;

            const std::shared_ptr<const syrec::Number> loopVariableData = rOperand->getReadOnlyLoopVariableData().value();
            return loopVariableData && loopVariableData->isLoopVariable() && lOperand->getVariableIdentifier() == loopVariableData->variableName() && !rOperand->getDeclaredVariableBitwidth().has_value() && rOperand->getDeclaredVariableDimensions().size() == 1 && rOperand->getDeclaredVariableDimensions().front() == 1;
        }
        if (!lOperand->getReadonlyVariableData().has_value() || lOperand->getReadonlyVariableData().value() == nullptr || rOperand->getReadOnlyLoopVariableData().has_value() || !rOperand->getReadonlyVariableData().has_value())
            return false;

        const std::shared_ptr<const syrec::Variable> expectedVariableData = lOperand->getReadonlyVariableData().value();
        const std::shared_ptr<const syrec::Variable> actualVariableData   = rOperand->getReadonlyVariableData().value();
        return actualVariableData && expectedVariableData->type == actualVariableData->type && expectedVariableData->bitwidth == actualVariableData->bitwidth && expectedVariableData->name == actualVariableData->name && std::equal(expectedVariableData->dimensions.cbegin(), expectedVariableData->dimensions.cend(), actualVariableData->dimensions.cbegin(), actualVariableData->dimensions.cend(), [](const unsigned int expectedNumberOfValuesOfDimension, const unsigned int actualNumberOfValuesOfDimension) {
                   return expectedNumberOfValuesOfDimension == actualNumberOfValuesOfDimension;
               });
    }

    void assertFetchedEntriesFromSymbolTableMatchExpectedOnes(const std::vector<ExpectedSymbolTableEntry>& expectedEntries, const std::vector<ExpectedSymbolTableEntry>& actualEntries) {
        ASSERT_EQ(expectedEntries.size(), actualEntries.size());
        ASSERT_TRUE(std::all_of(expectedEntries.cbegin(), expectedEntries.cend(), [](const ExpectedSymbolTableEntry& entry) { return entry != nullptr; }));
        ASSERT_TRUE(std::all_of(expectedEntries.cbegin(), expectedEntries.cend(), [](const ExpectedSymbolTableEntry& entry) { return entry != nullptr; }));
        // TODO: For now this call does not use the provided == operator, maybe we should define a custom MATCHER_P(...) instead. But how can the matcher be passed to the UnorderedElementsAreArray(...) call?
        //ASSERT_THAT(actualEntries, testing::UnorderedElementsAreArray(expectedEntries));

        // TODO: Rework this solution which is easy to read but a nightmare to determine which element was not found (since only the return value of the std::all_of(...) predicate is compared)
        ASSERT_TRUE(std::all_of(
                expectedEntries.cbegin(),
                expectedEntries.cend(),
                [&actualEntries](const ExpectedSymbolTableEntry& expected) {
                    return std::any_of(
                            actualEntries.cbegin(),
                            actualEntries.cend(),
                            [&expected](const ExpectedSymbolTableEntry& actualEntry) {
                                return expected == actualEntry;
                            });
                }));

        //    ASSERT_THAT(expected, testing::NotNull());
        //    ASSERT_THAT(actual, testing::NotNull());
        //    ASSERT_NO_FATAL_FAILURE(assertFetchedEntryFromSymbolTableMatchesExpectedOne(expected, actual));
    }

    void assertInsertionOfNVariableInstanceOfTypeIsSuccessful(TemporaryVariableScope& variableScope, syrec::Variable::Type variableType, const std::size_t numInstancesToCreate, const std::string& variableIdentifierPrefix, syrec::Variable::vec* containerForCreatedInstances) {
        ASSERT_GT(numInstancesToCreate, 0);

        if (containerForCreatedInstances && containerForCreatedInstances->empty())
            containerForCreatedInstances->reserve(numInstancesToCreate);

        for (std::size_t i = 0; i < numInstancesToCreate; ++i) {
            const auto variableInstance = std::make_shared<syrec::Variable>(variableType, variableIdentifierPrefix + "varIdent" + std::to_string(i), DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);
            ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(variableScope, variableInstance));
            if (containerForCreatedInstances)
                containerForCreatedInstances->emplace_back(variableInstance);
        }
    }

    [[nodiscard]] ExpectedSymbolTableEntry createExpectedSymbolTableEntryFrom(const std::variant<syrec::Variable::ptr, syrec::Number::ptr>& variableInstanceVariant) {
        if (std::holds_alternative<syrec::Variable::ptr>(variableInstanceVariant))
            return std::make_shared<TemporaryVariableScope::ScopeEntry>(std::get<syrec::Variable::ptr>(variableInstanceVariant));

        return std::make_shared<TemporaryVariableScope::ScopeEntry>(std::get<syrec::Number::ptr>(variableInstanceVariant));
    }

    void assertVariableExistanceByNameFromSymbolTable(const TemporaryVariableScope& variableScope, const std::variant<syrec::Number::ptr, syrec::Variable::ptr>& expectedVariableVariant) {
        std::optional<TemporaryVariableScope::ScopeEntry::readOnylPtr> actualVariableEntryFromSymbolTable;
        std::optional<TemporaryVariableScope::ScopeEntry::readOnylPtr> expectedVariableEntryFromSymbolTable;
        if (std::holds_alternative<syrec::Number::ptr>(expectedVariableVariant)) {
            const syrec::Number::ptr& expectedVariableNumberContainer = std::get<syrec::Number::ptr>(expectedVariableVariant);
            ASSERT_THAT(expectedVariableNumberContainer, testing::NotNull());
            ASSERT_TRUE(expectedVariableNumberContainer->isLoopVariable());
            ASSERT_NO_FATAL_FAILURE(actualVariableEntryFromSymbolTable = variableScope.getVariableByName(expectedVariableNumberContainer->variableName()));
            expectedVariableEntryFromSymbolTable = createExpectedSymbolTableEntryFrom(expectedVariableNumberContainer);
        }
        else {
            const syrec::Variable::ptr& expectedVariableContainer = std::get<syrec::Variable::ptr>(expectedVariableVariant);
            ASSERT_THAT(expectedVariableContainer, testing::NotNull());
            ASSERT_NO_FATAL_FAILURE(actualVariableEntryFromSymbolTable = variableScope.getVariableByName(expectedVariableContainer->name));
            expectedVariableEntryFromSymbolTable = createExpectedSymbolTableEntryFrom(expectedVariableContainer);
            
        }
        ASSERT_NO_FATAL_FAILURE(assertFetchedEntryFromSymbolTableMatchesExpectedOne(expectedVariableEntryFromSymbolTable, actualVariableEntryFromSymbolTable));
    }

    
    [[nodiscard]] std::vector<ExpectedSymbolTableEntry> createdExpectedSymbolTableEntriesFrom(const std::vector<syrec::Variable::ptr>& variableInstances) {
        std::vector<ExpectedSymbolTableEntry> resultContainer;
        std::transform(
                variableInstances.cbegin(),
                variableInstances.cend(),
                std::back_inserter(resultContainer),
                [](const syrec::Variable::ptr& variableInstance) {
                    return createExpectedSymbolTableEntryFrom(variableInstance);
                });
        return resultContainer;
    }

    [[nodiscard]] std::string stringifyVariableType(syrec::Variable::Type variableType) {
        switch (variableType) {
            case syrec::Variable::Type::In:
                return "in";
            case syrec::Variable::Type::Out:
                return "out";
            case syrec::Variable::Type::Inout:
                return "inout";
            case syrec::Variable::Type::Wire:
                return "wire";
            case syrec::Variable::Type::State:
                return "state";
            default:
                throw std::invalid_argument("Could not stringify variable type");
        }
    }

    [[nodiscard]] std::vector<syrec::Variable::Type> getCollectionOfVariableTypes() {
        return {syrec::Variable::Type::In, syrec::Variable::Type::Out, syrec::Variable::Type::Inout, syrec::Variable::Type::Wire, syrec::Variable::Type::State};
    }
}

TEST_P(SingleVariableTypeTestFixture, InsertionOfVariableOfType) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;

    const auto        variableInstance = std::make_shared<syrec::Variable>(variableTypeUnderTest, "variableIdent", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, variableInstance));
}

TEST_P(SingleVariableTypeTestFixture, SearchForVariableUsingIdentifier) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    constexpr std::size_t numVariableEntriesMatchingType = 5;
    syrec::Variable::vec  createdVariableInstancesMatchingType;
    ASSERT_NO_FATAL_FAILURE(assertInsertionOfNVariableInstanceOfTypeIsSuccessful(*variableScope, variableTypeUnderTest, numVariableEntriesMatchingType, "ofType", &createdVariableInstancesMatchingType));

    constexpr std::size_t numVariableEntriesNotMatchingType = 3;
    syrec::Variable::vec  createdVariableInstancesNotMatchingType;
    ASSERT_NO_FATAL_FAILURE(assertInsertionOfNVariableInstanceOfTypeIsSuccessful(*variableScope, otherVariableType, numVariableEntriesNotMatchingType, "notOfType", &createdVariableInstancesNotMatchingType));

    const std::vector<ExpectedSymbolTableEntry> expectedSymbolTableEntriesForVariablesMatchingType = createdExpectedSymbolTableEntriesFrom(createdVariableInstancesMatchingType);
    std::vector<ExpectedSymbolTableEntry>       actualSymbolTableEntriesForVariablesMatchingType;
    ASSERT_NO_FATAL_FAILURE(actualSymbolTableEntriesForVariablesMatchingType = variableScope->getVariablesMatchingType({variableTypeUnderTest}));
    ASSERT_NO_FATAL_FAILURE(assertFetchedEntriesFromSymbolTableMatchExpectedOnes(expectedSymbolTableEntriesForVariablesMatchingType, actualSymbolTableEntriesForVariablesMatchingType));

    const std::vector<ExpectedSymbolTableEntry> expectedSymbolTableEntriesForVariablesNotMatchingType = createdExpectedSymbolTableEntriesFrom(createdVariableInstancesNotMatchingType);
    std::vector<ExpectedSymbolTableEntry>       actualSymbolTableEntriesForVariablesNotMatchingType;
    ASSERT_NO_FATAL_FAILURE(actualSymbolTableEntriesForVariablesNotMatchingType = variableScope->getVariablesMatchingType({otherVariableType}));
    ASSERT_NO_FATAL_FAILURE(assertFetchedEntriesFromSymbolTableMatchExpectedOnes(expectedSymbolTableEntriesForVariablesNotMatchingType, actualSymbolTableEntriesForVariablesNotMatchingType));

    const syrec::Variable::ptr              variableInstanceOfInterest                 = createdVariableInstancesMatchingType.at(numVariableEntriesMatchingType - 3);
    std::optional<ExpectedSymbolTableEntry> expectedSymbolTableEntryMatchingIdentifier = createExpectedSymbolTableEntryFrom(variableInstanceOfInterest);
    std::optional<ExpectedSymbolTableEntry> actualSymbolTableEntryMatchingIdentifier;
    ASSERT_NO_FATAL_FAILURE(actualSymbolTableEntryMatchingIdentifier = variableScope->getVariableByName(variableInstanceOfInterest->name));
    ASSERT_NO_FATAL_FAILURE(assertFetchedEntryFromSymbolTableMatchesExpectedOne(expectedSymbolTableEntryMatchingIdentifier, actualSymbolTableEntryMatchingIdentifier));
}

TEST_P(SingleVariableTypeTestFixture, SearchForVariablesOfSingleType) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    constexpr std::size_t numVariableEntries = 5;
    syrec::Variable::vec  createdVariableInstances;
    ASSERT_NO_FATAL_FAILURE(assertInsertionOfNVariableInstanceOfTypeIsSuccessful(*variableScope, variableTypeUnderTest, numVariableEntries, "ofType", &createdVariableInstances));

    const std::vector<ExpectedSymbolTableEntry> expectedSymbolTableEntriesForVariableType = createdExpectedSymbolTableEntriesFrom(createdVariableInstances);
    std::vector<ExpectedSymbolTableEntry>       actualSymbolTableEntriesForVariableType;
    ASSERT_NO_FATAL_FAILURE(actualSymbolTableEntriesForVariableType = variableScope->getVariablesMatchingType({variableTypeUnderTest}));
    ASSERT_NO_FATAL_FAILURE(assertFetchedEntriesFromSymbolTableMatchExpectedOnes(expectedSymbolTableEntriesForVariableType, actualSymbolTableEntriesForVariableType));
}

TEST_P(SingleVariableTypeTestFixture, RemoveModuleParameter) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    constexpr std::size_t numVariableEntriesMatchingType = 5;
    syrec::Variable::vec  createdVariableInstancesMatchingType;
    ASSERT_NO_FATAL_FAILURE(assertInsertionOfNVariableInstanceOfTypeIsSuccessful(*variableScope, variableTypeUnderTest, numVariableEntriesMatchingType, "ofType", &createdVariableInstancesMatchingType));

    constexpr std::size_t numVariableEntriesNotMatchingType = 3;
    syrec::Variable::vec  createdVariableInstancesNotMatchingType;
    ASSERT_NO_FATAL_FAILURE(assertInsertionOfNVariableInstanceOfTypeIsSuccessful(*variableScope, otherVariableType, numVariableEntriesNotMatchingType, "notOfType", &createdVariableInstancesNotMatchingType));

    const std::vector<ExpectedSymbolTableEntry> expectedSymbolTableEntriesForVariablesMatchingType = createdExpectedSymbolTableEntriesFrom(createdVariableInstancesMatchingType);
    std::vector<ExpectedSymbolTableEntry>       actualSymbolTableEntriesForVariablesMatchingType;
    ASSERT_NO_FATAL_FAILURE(actualSymbolTableEntriesForVariablesMatchingType = variableScope->getVariablesMatchingType({variableTypeUnderTest}));
    ASSERT_NO_FATAL_FAILURE(assertFetchedEntriesFromSymbolTableMatchExpectedOnes(expectedSymbolTableEntriesForVariablesMatchingType, actualSymbolTableEntriesForVariablesMatchingType));

    const std::vector<ExpectedSymbolTableEntry> expectedSymbolTableEntriesForVariablesNotMatchingType = createdExpectedSymbolTableEntriesFrom(createdVariableInstancesNotMatchingType);
    std::vector<ExpectedSymbolTableEntry>       actualSymbolTableEntriesForVariablesNotMatchingType;
    ASSERT_NO_FATAL_FAILURE(actualSymbolTableEntriesForVariablesNotMatchingType = variableScope->getVariablesMatchingType({otherVariableType}));
    ASSERT_NO_FATAL_FAILURE(assertFetchedEntriesFromSymbolTableMatchExpectedOnes(expectedSymbolTableEntriesForVariablesNotMatchingType, actualSymbolTableEntriesForVariablesNotMatchingType));

    constexpr std::size_t indexOfRemovedVariable              = numVariableEntriesMatchingType - 3;
    const std::string     identifierOfVariableToRemove        = createdVariableInstancesMatchingType.at(indexOfRemovedVariable)->name;
    bool                  wasModuleParameterRemovalSuccessful = false;
    ASSERT_NO_FATAL_FAILURE(wasModuleParameterRemovalSuccessful = variableScope->removeVariable(identifierOfVariableToRemove));
    ASSERT_TRUE(wasModuleParameterRemovalSuccessful);

    std::vector createdVariableInstancesMatchingTypeAfterRemoval = createdVariableInstancesMatchingType;
    createdVariableInstancesMatchingTypeAfterRemoval.erase(std::next(createdVariableInstancesMatchingTypeAfterRemoval.begin(), indexOfRemovedVariable));

    const std::vector<ExpectedSymbolTableEntry> expectedSymbolTableEntriesForVariablesMatchingTypeAfterRemoval = createdExpectedSymbolTableEntriesFrom(createdVariableInstancesMatchingTypeAfterRemoval);
    ASSERT_NO_FATAL_FAILURE(actualSymbolTableEntriesForVariablesMatchingType = variableScope->getVariablesMatchingType({variableTypeUnderTest}));
    ASSERT_NO_FATAL_FAILURE(assertFetchedEntriesFromSymbolTableMatchExpectedOnes(expectedSymbolTableEntriesForVariablesMatchingTypeAfterRemoval, actualSymbolTableEntriesForVariablesMatchingType));

    ASSERT_NO_FATAL_FAILURE(actualSymbolTableEntriesForVariablesNotMatchingType = variableScope->getVariablesMatchingType({otherVariableType}));
    ASSERT_NO_FATAL_FAILURE(assertFetchedEntriesFromSymbolTableMatchExpectedOnes(expectedSymbolTableEntriesForVariablesNotMatchingType, actualSymbolTableEntriesForVariablesNotMatchingType));
}

TEST_P(SingleVariableTypeTestFixture, ExistsVariableForIdentifier) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    constexpr std::size_t numVariableEntriesMatchingType = 5;
    syrec::Variable::vec  createdVariableInstancesMatchingType;
    ASSERT_NO_FATAL_FAILURE(assertInsertionOfNVariableInstanceOfTypeIsSuccessful(*variableScope, variableTypeUnderTest, numVariableEntriesMatchingType, "ofType", &createdVariableInstancesMatchingType));

    constexpr std::size_t numVariableEntriesNotMatchingType = 3;
    syrec::Variable::vec  createdVariableInstancesNotMatchingType;
    ASSERT_NO_FATAL_FAILURE(assertInsertionOfNVariableInstanceOfTypeIsSuccessful(*variableScope, otherVariableType, numVariableEntriesNotMatchingType, "notOfType", &createdVariableInstancesNotMatchingType));

    const syrec::Variable::ptr variableInstanceOfInterest                                 = createdVariableInstancesMatchingType.at(numVariableEntriesMatchingType - 3);
    bool                       actualStatusWhetherVariableForIdentfierExistsInSymboltable = false;
    ASSERT_NO_FATAL_FAILURE(actualStatusWhetherVariableForIdentfierExistsInSymboltable = variableScope->existsVariableForName(variableInstanceOfInterest->name));
    ASSERT_TRUE(actualStatusWhetherVariableForIdentfierExistsInSymboltable);
}

INSTANTIATE_TEST_SUITE_P(
        ParameterizedTemporaryVariableScopeTests,
        SingleVariableTypeTestFixture,
        testing::Values(
                std::make_pair(syrec::Variable::Type::In, syrec::Variable::Type::Out),
                std::make_pair(syrec::Variable::Type::Out, syrec::Variable::Type::Wire),
                std::make_pair(syrec::Variable::Type::Inout, syrec::Variable::Type::In),
                std::make_pair(syrec::Variable::Type::Wire, syrec::Variable::Type::In),
                std::make_pair(syrec::Variable::Type::State, syrec::Variable::Type::Out)));

TEST(TemporaryVariableScopeTests, InsertLoopVariable) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;

    const std::string loopVariableIdent    = "$loopVariableIdent";
    const auto        loopVariableInstance = std::make_shared<syrec::Number>(loopVariableIdent);
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, loopVariableInstance));
}

TEST(TemporaryVariableScopeTests, SearchForVariableUsingIdentifierWithNoMatchingSymbolTableEntry) {
    BaseSymbolTable symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    constexpr std::size_t numVariableEntriesMatchingType = 5;
    ASSERT_NO_FATAL_FAILURE(assertInsertionOfNVariableInstanceOfTypeIsSuccessful(*variableScope, syrec::Variable::Type::In, numVariableEntriesMatchingType, "ofType", nullptr));

    const std::string variableIdentifierWithNoMatchingSymbolTableEntry = "aVariantIdent";
    ASSERT_FALSE(variableScope->getVariableByName(variableIdentifierWithNoMatchingSymbolTableEntry).has_value());
}

// Mix parameter as well as local variable types
TEST(TemporaryVariableScopeTests, SearchForVariablesOfDifferentTypesWithOnlySomeHavingMatches) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    syrec::Variable::vec variableInstancesOfTypeIn;
    syrec::Variable::vec variableInstancesOfTypeWire;
    constexpr std::size_t numVariableEntriesPerType = 4;

    ASSERT_NO_FATAL_FAILURE(assertInsertionOfNVariableInstanceOfTypeIsSuccessful(*variableScope, syrec::Variable::Type::In, numVariableEntriesPerType, stringifyVariableType(syrec::Variable::Type::In), &variableInstancesOfTypeIn));
    ASSERT_NO_FATAL_FAILURE(assertInsertionOfNVariableInstanceOfTypeIsSuccessful(*variableScope, syrec::Variable::Type::Wire, numVariableEntriesPerType, stringifyVariableType(syrec::Variable::Type::Wire), &variableInstancesOfTypeWire));    
    for (const auto variableTypesNotOfInterest : {syrec::Variable::Type::Inout, syrec::Variable::Type::State}) {
        ASSERT_NO_FATAL_FAILURE(assertInsertionOfNVariableInstanceOfTypeIsSuccessful(*variableScope, variableTypesNotOfInterest, numVariableEntriesPerType, stringifyVariableType(variableTypesNotOfInterest), nullptr));    
    }

    std::vector<ExpectedSymbolTableEntry> expectedSymbolTableEntriesMatchingTypes;
    std::transform(variableInstancesOfTypeIn.cbegin(),
                   variableInstancesOfTypeIn.cend(),
                   std::back_inserter(expectedSymbolTableEntriesMatchingTypes),
                   [](const syrec::Variable::ptr& variableInstance) {
                       return createExpectedSymbolTableEntryFrom(variableInstance);
                   });

    std::transform(variableInstancesOfTypeWire.cbegin(),
                   variableInstancesOfTypeWire.cend(),
                   std::back_inserter(expectedSymbolTableEntriesMatchingTypes),
                   [](const syrec::Variable::ptr& variableInstance) {
                       return createExpectedSymbolTableEntryFrom(variableInstance);
                   });

    std::vector<ExpectedSymbolTableEntry> actualSymbolTableEntriesMatchingTypes;
    ASSERT_NO_FATAL_FAILURE(actualSymbolTableEntriesMatchingTypes = variableScope->getVariablesMatchingType({syrec::Variable::Type::In, syrec::Variable::Type::Wire, syrec::Variable::Type::Out}));
    ASSERT_NO_FATAL_FAILURE(assertFetchedEntriesFromSymbolTableMatchExpectedOnes(expectedSymbolTableEntriesMatchingTypes, actualSymbolTableEntriesMatchingTypes));
}

TEST(TemporaryVariableScopeTests, SearchForVariablesOfDifferentTypesWithNoMatches) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    for (const syrec::Variable::Type variableType : {syrec::Variable::Type::In, syrec::Variable::Type::Wire}) {
        constexpr std::size_t numVariablesPerType = 4;
        ASSERT_NO_FATAL_FAILURE(assertInsertionOfNVariableInstanceOfTypeIsSuccessful(*variableScope, variableType, numVariablesPerType, stringifyVariableType(variableType), nullptr));    
    }

    std::vector<ExpectedSymbolTableEntry> actualSymbolTableEntriesMatchingTypes;
    for (const syrec::Variable::Type variableType : {syrec::Variable::Type::Out, syrec::Variable::Type::Inout, syrec::Variable::Type::State }) {
        ASSERT_NO_FATAL_FAILURE(actualSymbolTableEntriesMatchingTypes = variableScope->getVariablesMatchingType({variableType}));
        ASSERT_NO_FATAL_FAILURE(assertFetchedEntriesFromSymbolTableMatchExpectedOnes({}, actualSymbolTableEntriesMatchingTypes));   
    }
}

TEST(TemporaryVariableScopeTests, SearchForVariablesOfDifferentTypesWillNotMatchLoopVariables) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    const std::string firstLoopVariableIdentifier = "$loopVarOne";
    const auto        firstLoopVariable           = std::make_shared<syrec::Number>(firstLoopVariableIdentifier);

    const std::string secondLoopVariableIdentifier = "$loopVarTwo";
    const auto        secondLoopVariable           = std::make_shared<syrec::Number>(secondLoopVariableIdentifier);

    const auto        nonLoopVariableOne          = std::make_shared<syrec::Variable>(syrec::Variable::Type::In, "varOne", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);
    const auto        nonLoopVariableTwo          = std::make_shared<syrec::Variable>(syrec::Variable::Type::Wire, "varTwo", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);

    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, firstLoopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, secondLoopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, nonLoopVariableOne));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, nonLoopVariableTwo));

    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, firstLoopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, secondLoopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, nonLoopVariableOne));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, nonLoopVariableTwo));

    std::vector<ExpectedSymbolTableEntry> actualSymbolTableEntriesMatchingType;
    ASSERT_NO_FATAL_FAILURE(actualSymbolTableEntriesMatchingType = variableScope->getVariablesMatchingType({nonLoopVariableOne->type}));
    ASSERT_NO_FATAL_FAILURE(assertFetchedEntriesFromSymbolTableMatchExpectedOnes(createdExpectedSymbolTableEntriesFrom({nonLoopVariableOne}), actualSymbolTableEntriesMatchingType));

    ASSERT_NO_FATAL_FAILURE(actualSymbolTableEntriesMatchingType = variableScope->getVariablesMatchingType({nonLoopVariableTwo->type}));
    ASSERT_NO_FATAL_FAILURE(assertFetchedEntriesFromSymbolTableMatchExpectedOnes(createdExpectedSymbolTableEntriesFrom({nonLoopVariableTwo}), actualSymbolTableEntriesMatchingType));

    for (const syrec::Variable::Type variableType: getCollectionOfVariableTypes()) {
        if (variableType == nonLoopVariableOne->type || variableType == nonLoopVariableTwo->type)
            continue;

        ASSERT_NO_FATAL_FAILURE(actualSymbolTableEntriesMatchingType = variableScope->getVariablesMatchingType({variableType}));
        ASSERT_NO_FATAL_FAILURE(assertFetchedEntriesFromSymbolTableMatchExpectedOnes(createdExpectedSymbolTableEntriesFrom({}), actualSymbolTableEntriesMatchingType));
    }
}

TEST(TemporaryVariableScopeTests, SearchForVariableUsingLoopVariableIdentifierDoesNotWork) {
    BaseSymbolTable symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    for (const syrec::Variable::Type variableType: getCollectionOfVariableTypes()) {
        constexpr std::size_t numVariablesPerType = 2;
        ASSERT_NO_FATAL_FAILURE(assertInsertionOfNVariableInstanceOfTypeIsSuccessful(*variableScope, variableType, numVariablesPerType, stringifyVariableType(variableType), nullptr));
    }

    std::optional<ExpectedSymbolTableEntry> actualSymbolTableEntriesMatchingIdentifier;
    ASSERT_NO_FATAL_FAILURE(actualSymbolTableEntriesMatchingIdentifier = variableScope->getVariableByName("$loopVariable"));
    ASSERT_FALSE(actualSymbolTableEntriesMatchingIdentifier.has_value());
}

TEST(TemporaryVariableScopeTests, SearchForLoopVariableUsingVariableIdentifierDoesNotWork) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    const std::string firstLoopVariableIdentifierWithoutPrefix = "loopVarOne";
    const std::string firstLoopVariableIdentifier              = "$" + firstLoopVariableIdentifierWithoutPrefix;
    const std::string secondLoopVariableIdentifier             = "$loopVarTwo";
    const auto        firstLoopVariable                        = std::make_shared<syrec::Number>(firstLoopVariableIdentifier);
    const auto        secondLoopVariable                       = std::make_shared<syrec::Number>(secondLoopVariableIdentifier);
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, firstLoopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, secondLoopVariable));

    std::optional<ExpectedSymbolTableEntry> actualSymbolTableEntriesMatchingIdentifier;
    ASSERT_NO_FATAL_FAILURE(actualSymbolTableEntriesMatchingIdentifier = variableScope->getVariableByName(firstLoopVariableIdentifierWithoutPrefix));
    ASSERT_FALSE(actualSymbolTableEntriesMatchingIdentifier.has_value());

    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, firstLoopVariable));
}

TEST(TemporaryVariableScopeTests, RemoveLoopVariable) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    const std::string identifierOfLoopVariableToRemove = "$loopVarOne";
    const std::string identifierOfOtherLoopVariable    = "$loopVarTwo";
    const auto        loopVariableToRemove             = std::make_shared<syrec::Number>(identifierOfLoopVariableToRemove);
    const auto        otherLoopVariable                = std::make_shared<syrec::Number>(identifierOfOtherLoopVariable);
    const auto        nonLoopVariableOne               = std::make_shared<syrec::Variable>(syrec::Variable::Type::In, "varOne", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);
    const auto        nonLoopVariableTwo               = std::make_shared<syrec::Variable>(syrec::Variable::Type::Wire, "varTwo", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);

    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, loopVariableToRemove));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, otherLoopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, nonLoopVariableOne));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, nonLoopVariableTwo));

    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, loopVariableToRemove));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, otherLoopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, nonLoopVariableOne));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, nonLoopVariableTwo));

    bool wasLoopVariableRemovalSuccessful = false;
    ASSERT_NO_FATAL_FAILURE(wasLoopVariableRemovalSuccessful = variableScope->removeVariable(loopVariableToRemove->variableName()));
    ASSERT_TRUE(wasLoopVariableRemovalSuccessful);

    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, otherLoopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, nonLoopVariableOne));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, nonLoopVariableTwo));
}

TEST(TemporaryVariableScopeTests, RemoveVariableInEmptySymbolTable) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    bool wasVariableRemovalSuccessful = false;
    ASSERT_NO_FATAL_FAILURE(wasVariableRemovalSuccessful = variableScope->removeVariable("nonExistingVarIdentifier"));
    ASSERT_FALSE(wasVariableRemovalSuccessful);
}

TEST(TemporaryVariableScopeTests, RemoveVariableUsingIdentifierHavingNoMatch) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    const std::string loopVariableIdentifier = "$loopVarIdent";
    const auto        loopVariable           = std::make_shared<syrec::Number>(loopVariableIdentifier);
    const auto        nonLoopVariableOne     = std::make_shared<syrec::Variable>(syrec::Variable::Type::In, "varOne", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);
    const auto        nonLoopVariableTwo     = std::make_shared<syrec::Variable>(syrec::Variable::Type::Wire, "varTwo", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);

    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, loopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, nonLoopVariableOne));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, nonLoopVariableTwo));
    
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, loopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, nonLoopVariableOne));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, nonLoopVariableTwo));

    bool wasVariableRemovalSuccessful = false;
    ASSERT_NO_FATAL_FAILURE(wasVariableRemovalSuccessful = variableScope->removeVariable("anotherVariableIdent"));
    ASSERT_FALSE(wasVariableRemovalSuccessful);

    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, loopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, nonLoopVariableOne));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, nonLoopVariableTwo));
}

TEST(TemporaryVariableScopeTests, RemoveVariableUsingLoopVariableIdentifierDoesNotWork) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    const std::string loopVariableIdentifierWithoutPrefix = "loopVarIdent";
    const std::string loopVariableIdentifier              = "$" + loopVariableIdentifierWithoutPrefix;
    const auto        loopVariable                        = std::make_shared<syrec::Number>(loopVariableIdentifier);
    const auto        nonLoopVariableOne                  = std::make_shared<syrec::Variable>(syrec::Variable::Type::In, loopVariableIdentifierWithoutPrefix, DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);
    const auto        nonLoopVariableTwo                  = std::make_shared<syrec::Variable>(syrec::Variable::Type::Wire, "varTwo", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);

    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, loopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, nonLoopVariableOne));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, nonLoopVariableTwo));

    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, loopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, nonLoopVariableOne));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, nonLoopVariableTwo));

    bool wasVariableRemovalSuccessful = false;
    ASSERT_NO_FATAL_FAILURE(wasVariableRemovalSuccessful = variableScope->removeVariable(loopVariableIdentifier));
    ASSERT_TRUE(wasVariableRemovalSuccessful);

    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, nonLoopVariableOne));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, nonLoopVariableTwo));

    ASSERT_NO_FATAL_FAILURE(wasVariableRemovalSuccessful = variableScope->removeVariable(loopVariableIdentifier));
    ASSERT_FALSE(wasVariableRemovalSuccessful);
}

TEST(TemporaryVariableScopeTests, RemoveLoopVariableUsingVariableIdentifierDoesNotWork) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    const std::string loopVariableIdentifierWithoutPrefix = "loopVarIdent";
    const std::string loopVariableIdentifier              = "$" + loopVariableIdentifierWithoutPrefix;
    const auto        loopVariable                        = std::make_shared<syrec::Number>(loopVariableIdentifier);
    const auto        nonLoopVariableOne                  = std::make_shared<syrec::Variable>(syrec::Variable::Type::In, "varOne", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);
    const auto        nonLoopVariableTwo                  = std::make_shared<syrec::Variable>(syrec::Variable::Type::Wire, "varTwo", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);

    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, loopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, nonLoopVariableOne));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, nonLoopVariableTwo));

    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, loopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, nonLoopVariableOne));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, nonLoopVariableTwo));

    bool wasVariableRemovalSuccessful = false;
    ASSERT_NO_FATAL_FAILURE(wasVariableRemovalSuccessful = variableScope->removeVariable(nonLoopVariableOne->name));
    ASSERT_TRUE(wasVariableRemovalSuccessful);

    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, loopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, nonLoopVariableTwo));

    ASSERT_NO_FATAL_FAILURE(wasVariableRemovalSuccessful = variableScope->removeVariable(nonLoopVariableOne->name));
    ASSERT_FALSE(wasVariableRemovalSuccessful);
}

TEST(TemporaryVariableScopeTests, ExistsVariableForIdentifierHavingNoMatch) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    const std::string loopVariableIdentifier = "$loopVarIdent";
    const auto        loopVariable           = std::make_shared<syrec::Number>(loopVariableIdentifier);
    const auto        nonLoopVariableOne     = std::make_shared<syrec::Variable>(syrec::Variable::Type::In, "varOne", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);
    const auto        nonLoopVariableTwo     = std::make_shared<syrec::Variable>(syrec::Variable::Type::Wire, "varTwo", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);

    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, loopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, nonLoopVariableOne));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, nonLoopVariableTwo));

    bool existsSymbolTableEntryForVariableIdentifier = false;
    ASSERT_NO_FATAL_FAILURE(existsSymbolTableEntryForVariableIdentifier = variableScope->existsVariableForName("variableIdentWithoutMatch"));
    ASSERT_FALSE(existsSymbolTableEntryForVariableIdentifier);
}

TEST(TemporaryVariableScopeTests, ExistsVariableForIdentifierInEmptySymbolTable) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    bool existsSymbolTableEntryForVariableIdentifier = false;
    ASSERT_NO_FATAL_FAILURE(existsSymbolTableEntryForVariableIdentifier = variableScope->existsVariableForName("variableIdentWithoutMatch"));
    ASSERT_FALSE(existsSymbolTableEntryForVariableIdentifier);
}

TEST(TemporaryVariableScopeTests, ExistsVariableUsingLoopVariableIdentifierReturnsFalse) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    const auto        nonLoopVariableOne     = std::make_shared<syrec::Variable>(syrec::Variable::Type::In, "varOne", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);
    const auto        nonLoopVariableTwo     = std::make_shared<syrec::Variable>(syrec::Variable::Type::Wire, "varTwo", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);

    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, nonLoopVariableOne));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, nonLoopVariableTwo));

    bool existsSymbolTableEntryForVariableIdentifier = false;
    ASSERT_NO_FATAL_FAILURE(existsSymbolTableEntryForVariableIdentifier = variableScope->existsVariableForName("$" + nonLoopVariableOne->name));
    ASSERT_FALSE(existsSymbolTableEntryForVariableIdentifier);
}

TEST(TemporaryVariableScopeTests, ExistsLoopVariableUsingVariableIdentifierReturnsFalse) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    const std::string loopVariableOneIdentifierWithoutPrefix = "loopVarOne";
    const std::string loopVariableTwoIdentifierWithoutPrefix = "loopVarTwo";
    const auto        loopVariableOne           = std::make_shared<syrec::Number>("$" + loopVariableOneIdentifierWithoutPrefix);
    const auto        loopVariableTwo           = std::make_shared<syrec::Number>("$" + loopVariableTwoIdentifierWithoutPrefix);

    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, loopVariableOne));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, loopVariableTwo));

    bool existsSymbolTableEntryForVariableIdentifier = false;
    ASSERT_NO_FATAL_FAILURE(existsSymbolTableEntryForVariableIdentifier = variableScope->existsVariableForName(loopVariableOneIdentifierWithoutPrefix));
    ASSERT_FALSE(existsSymbolTableEntryForVariableIdentifier);
}

// Error cases
TEST(TemporaryVariableScopeTests, InsertionOfInvalidLoopVariableEntryNotPossible) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    const std::string loopVariableIdentifier = "$loopVar";
    const auto        loopVariable           = std::make_shared<syrec::Number>(loopVariableIdentifier);
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, loopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, loopVariable));

    syrec::Number::ptr invalidLoopVariableContainer = nullptr;
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsNotSuccessful(*variableScope, invalidLoopVariableContainer));

    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, loopVariable));
}

TEST(TemporaryVariableScopeTests, InsertionOfLoopVariableWithEmptyIdentifierNotPossible) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    const std::string loopVariableIdentifier = "$loopVar";
    const auto        loopVariable           = std::make_shared<syrec::Number>(loopVariableIdentifier);
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, loopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, loopVariable));

    const std::string  emptyLoopVariableIdentifier  = "";
    syrec::Number::ptr invalidLoopVariableContainer = std::make_shared<syrec::Number>(emptyLoopVariableIdentifier);
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsNotSuccessful(*variableScope, invalidLoopVariableContainer));

    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, loopVariable));
}

TEST(TemporaryVariableScopeTests, InsertionOfDuplicateLoopVariableNotPossible) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    const std::string loopVariableIdentifier = "$loopVar";
    const auto        loopVariable           = std::make_shared<syrec::Number>(loopVariableIdentifier);
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, loopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, loopVariable));

    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsNotSuccessful(*variableScope, loopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, loopVariable));
}

TEST(TemporaryVariableScopeTests, InsertionOfLoopVariableWithoutRequiredNamePrefixNotPossible) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    const std::string loopVariableIdentifier = "$loopVar";
    const auto        loopVariable           = std::make_shared<syrec::Number>(loopVariableIdentifier);
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, loopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, loopVariable));

    const std::string loopVariableIdentifierWithoutExpectedPrefix = "otherLoopVar";
    const auto        invalidLoopVariable                         = std::make_shared<syrec::Number>(loopVariableIdentifierWithoutExpectedPrefix);
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsNotSuccessful(*variableScope, invalidLoopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, loopVariable));

    std::optional<ExpectedSymbolTableEntry> actualLoopVariableEntryForIdentifierWithoutExpectedPrefix;
    ASSERT_NO_FATAL_FAILURE(actualLoopVariableEntryForIdentifierWithoutExpectedPrefix = variableScope->getVariableByName(loopVariableIdentifierWithoutExpectedPrefix));
    ASSERT_FALSE(actualLoopVariableEntryForIdentifierWithoutExpectedPrefix.has_value());
}

TEST(TemporaryVariableScopeTests, InsertionOfDuplicateVariableNotPossible) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    const auto firstVariable = std::make_shared<syrec::Variable>(syrec::Variable::Type::In, "varOne", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);
    const auto secondVariable = std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "varTwo", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);
    const auto thirdVariable = std::make_shared<syrec::Variable>(syrec::Variable::Type::Wire, "varThree", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);

    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, firstVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, secondVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, thirdVariable));

    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, firstVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, secondVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, thirdVariable));

    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsNotSuccessful(*variableScope, firstVariable));

    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, firstVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, secondVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, thirdVariable));
}

TEST(TemporaryVariableScopeTests, InsertionOfInvalidVariableEntryNotPossible) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    const auto firstVariable  = std::make_shared<syrec::Variable>(syrec::Variable::Type::In, "varOne", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);
    const auto secondVariable = std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "varTwo", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);

    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, firstVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, secondVariable));

    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, firstVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, secondVariable));

    const syrec::Variable::ptr invalidVariableEntry = nullptr;
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsNotSuccessful(*variableScope, invalidVariableEntry));

    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, firstVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, secondVariable));
}

TEST(TemporaryVariableScopeTests, InsertionOfVariableEntryWithEmptyIdentifierNotPossible) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    const auto firstVariable  = std::make_shared<syrec::Variable>(syrec::Variable::Type::In, "varOne", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);
    const auto secondVariable = std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "varTwo", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);

    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, firstVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, secondVariable));

    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, firstVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, secondVariable));

    const syrec::Variable::ptr invalidVariableEntry = std::make_shared<syrec::Variable>(syrec::Variable::Type::Wire, "", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsNotSuccessful(*variableScope, invalidVariableEntry));

    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, firstVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, secondVariable));
}

TEST(TemporaryVariableScopeTests, SearchForVariableUsingEmptyIdentifier) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    const auto firstVariable  = std::make_shared<syrec::Variable>(syrec::Variable::Type::In, "varOne", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);
    const auto secondVariable = std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "varTwo", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);
    const auto thirdVariable  = std::make_shared<syrec::Variable>(syrec::Variable::Type::Wire, "varThree", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);

    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, firstVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, secondVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, thirdVariable));

    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, firstVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, secondVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, thirdVariable));

    std::optional<ExpectedSymbolTableEntry> symbolTableEntryMatchingEmptyIdentifier;
    ASSERT_NO_FATAL_FAILURE(symbolTableEntryMatchingEmptyIdentifier = variableScope->getVariableByName(""));
    ASSERT_FALSE(symbolTableEntryMatchingEmptyIdentifier.has_value());
}

TEST(TemporaryVariableScopeTests, InsertNonLoopVariableNumberContainer) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    const std::string loopVariableIdentifier = "$loopVar";
    const auto        loopVariable           = std::make_shared<syrec::Number>(loopVariableIdentifier);
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, loopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, loopVariable));

    const auto nonLoopVariableContainer = std::make_shared<syrec::Number>(200);
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsNotSuccessful(*variableScope, loopVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, loopVariable));
}

TEST(TemporaryVariableScopeTests, SearchForEmptyCollectionOfVariableTypes) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    const auto firstVariable  = std::make_shared<syrec::Variable>(syrec::Variable::Type::In, "varOne", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);
    const auto secondVariable = std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "varTwo", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);
    const auto thirdVariable  = std::make_shared<syrec::Variable>(syrec::Variable::Type::Wire, "varThree", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);

    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, firstVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, secondVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, thirdVariable));

    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, firstVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, secondVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, thirdVariable));

    std::vector<ExpectedSymbolTableEntry> actualSymbolTableEntriesForVariablesMatchingTypes;
    ASSERT_NO_FATAL_FAILURE(actualSymbolTableEntriesForVariablesMatchingTypes = variableScope->getVariablesMatchingType({}));
    ASSERT_TRUE(actualSymbolTableEntriesForVariablesMatchingTypes.empty());
}

TEST(TemporaryVariableScopeTests, SearchForCollectionOfVariableTypesInEmptySymbolTable) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    std::vector<ExpectedSymbolTableEntry> actualSymbolTableEntriesForVariablesMatchingTypes;
    ASSERT_NO_FATAL_FAILURE(actualSymbolTableEntriesForVariablesMatchingTypes = variableScope->getVariablesMatchingType({}));
    ASSERT_TRUE(actualSymbolTableEntriesForVariablesMatchingTypes.empty());
}

TEST(TemporaryVariableScopeTests, RemoveVariableUsingEmptyIdentifier) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    const auto firstVariable  = std::make_shared<syrec::Variable>(syrec::Variable::Type::In, "varOne", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);
    const auto secondVariable = std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "varTwo", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);
    
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, firstVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, secondVariable));
    
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, firstVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, secondVariable));

    bool wasVariableRemovalSuccessful = false;
    ASSERT_NO_FATAL_FAILURE(variableScope->removeVariable(""));
    ASSERT_FALSE(wasVariableRemovalSuccessful);

    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, firstVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, secondVariable));
}

TEST(TemporaryVariableScopeTests, ExistsVariableForEmptyIdentifier) {
    BaseSymbolTable             symbolTable;
    TemporaryVariableScope::ptr variableScope;
    ASSERT_NO_FATAL_FAILURE(assertCreationOfTemporaryVariableScopeSuccedds(symbolTable, variableScope));

    const auto firstVariable  = std::make_shared<syrec::Variable>(syrec::Variable::Type::In, "varOne", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);
    const auto secondVariable = std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "varTwo", DEFAULT_VARIABLE_DIMENSIONS, DEFAULT_BITWIDTH);

    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, firstVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableInsertionResultIsSuccessful(*variableScope, secondVariable));

    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, firstVariable));
    ASSERT_NO_FATAL_FAILURE(assertVariableExistanceByNameFromSymbolTable(*variableScope, secondVariable));

    std::optional<ExpectedSymbolTableEntry> actualSymbolTableEntryMatchingEmptyIdentifier;
    ASSERT_NO_FATAL_FAILURE(actualSymbolTableEntryMatchingEmptyIdentifier = variableScope->getVariableByName(""));
    ASSERT_FALSE(actualSymbolTableEntryMatchingEmptyIdentifier.has_value());
}