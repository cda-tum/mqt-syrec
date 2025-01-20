#include "core/syrec/expression.hpp"
#include <core/syrec/parser/utils/variable_overlap_check.hpp>

#include "gmock/gmock-matchers.h"
#include <gtest/gtest.h>

#include <vector>

namespace {
    const std::string      DEFAULT_VARIABLE_IDENTIFIER = "varIdent";
    constexpr unsigned int DEFAULT_VARIABLE_BITWIDTH = 16;

    syrec::Number::ptr createNumberContainerForConstantValue(unsigned int value) {
        return std::make_shared<syrec::Number>(value);
    }

    syrec::Number::ptr createNumberContainerForLoopVariableIdentifier(const std::string& loopVariableIdentifier) {
        return std::make_shared<syrec::Number>(loopVariableIdentifier);
    }

    syrec::Expression::ptr createExpressionForConstantValue(unsigned int value) {
        return std::make_shared<syrec::NumericExpression>(createNumberContainerForConstantValue(value), 2);
    }

    syrec::Variable::ptr createVariableInstance(const std::string& variableIdentifier, const std::vector<unsigned int>& variableDimensions, unsigned int variableBitwidth) {
        return std::make_shared<syrec::Variable>(syrec::Variable::Type::In, variableIdentifier, variableDimensions, variableBitwidth);
    }

    void assertOverlapDataMatches(const std::optional<utils::VariableAccessOverlapCheckResult::OverlappingIndicesContainer>& expectedOverlapData, const std::optional<utils::VariableAccessOverlapCheckResult::OverlappingIndicesContainer>& actualOverlapData) {
        if (!expectedOverlapData.has_value()) {
            ASSERT_FALSE(actualOverlapData.has_value()) << "Expected that no overlap data is available";
            return;
        }

        ASSERT_TRUE(actualOverlapData.has_value()) << "Expected that actual overlap data is available";
        ASSERT_THAT(actualOverlapData->knownValueOfAccessedValuePerDimension, testing::ElementsAreArray(expectedOverlapData->knownValueOfAccessedValuePerDimension));
        ASSERT_EQ(expectedOverlapData->overlappingBit, actualOverlapData->overlappingBit);
    }

    void assertEquivalenceBetweenVariableAccessOverlapOperands(const syrec::VariableAccess& lhsOperand, const syrec::VariableAccess& rhsOperand, const utils::VariableAccessOverlapCheckResult& expectedVariableAccessOverlapCheckResult) {
        std::optional<utils::VariableAccessOverlapCheckResult> actualVariableAccessOverlapCheckResult;
        ASSERT_NO_FATAL_FAILURE(actualVariableAccessOverlapCheckResult = utils::checkOverlapBetweenVariableAccesses(lhsOperand, rhsOperand));
        ASSERT_TRUE(actualVariableAccessOverlapCheckResult.has_value());
        ASSERT_EQ(expectedVariableAccessOverlapCheckResult.overlapState, actualVariableAccessOverlapCheckResult->overlapState);
        ASSERT_NO_FATAL_FAILURE(assertOverlapDataMatches(expectedVariableAccessOverlapCheckResult.overlappingIndicesInformations, actualVariableAccessOverlapCheckResult->overlappingIndicesInformations));
    }

    void assertSymmetricEquivalenceBetweenPotentiallyVariableAccessOverlapOperands(const syrec::VariableAccess& lhsOperand, const syrec::VariableAccess& rhsOperand) {
        std::optional<utils::VariableAccessOverlapCheckResult> actualVariableAccessOverlapCheckResult;
        ASSERT_NO_FATAL_FAILURE(actualVariableAccessOverlapCheckResult = utils::checkOverlapBetweenVariableAccesses(lhsOperand, rhsOperand));
        ASSERT_TRUE(actualVariableAccessOverlapCheckResult.has_value());
        ASSERT_EQ(utils::VariableAccessOverlapCheckResult::OverlapState::MaybeOverlapping, actualVariableAccessOverlapCheckResult->overlapState);
        ASSERT_FALSE(actualVariableAccessOverlapCheckResult->overlappingIndicesInformations.has_value());

        ASSERT_NO_FATAL_FAILURE(actualVariableAccessOverlapCheckResult = utils::checkOverlapBetweenVariableAccesses(rhsOperand, lhsOperand));
        ASSERT_TRUE(actualVariableAccessOverlapCheckResult.has_value());
        ASSERT_EQ(utils::VariableAccessOverlapCheckResult::OverlapState::MaybeOverlapping, actualVariableAccessOverlapCheckResult->overlapState);
        ASSERT_FALSE(actualVariableAccessOverlapCheckResult->overlappingIndicesInformations.has_value());
    }

    void assertSymmetricVariableAccessOverlapResultCannotBeDetermined(const syrec::VariableAccess& lhsOperand, const syrec::VariableAccess& rhsOperand) {
        std::optional<utils::VariableAccessOverlapCheckResult> actualVariableAccessOverlapCheckResult;
        ASSERT_NO_FATAL_FAILURE(actualVariableAccessOverlapCheckResult = utils::checkOverlapBetweenVariableAccesses(lhsOperand, rhsOperand));
        ASSERT_FALSE(actualVariableAccessOverlapCheckResult.has_value());
        
        // Operation must be symmmetric (a OP b) = (b OP a)
        ASSERT_NO_FATAL_FAILURE(actualVariableAccessOverlapCheckResult = utils::checkOverlapBetweenVariableAccesses(lhsOperand, rhsOperand));
        ASSERT_FALSE(actualVariableAccessOverlapCheckResult.has_value());
    }

    struct VariableDefinition {
        std::string               identifier;
        std::vector<unsigned int> dimensions;
        unsigned int              bitwidth;

        VariableDefinition(const std::string& variableIdentifier, std::vector<unsigned int>&& variableDimensions, unsigned int bitwidth):
            identifier(variableIdentifier), dimensions(std::move(variableDimensions)), bitwidth(bitwidth) {}
    };

    struct VariableAccessDefinition {
        using BitrangeIndexDataVariant = std::variant<unsigned int, std::string>;

        std::vector<unsigned int>                                        accessedValuePerDimension;
        std::optional<std::pair<syrec::Number::ptr, syrec::Number::ptr>> accessedBitRange;

        [[nodiscard]] static syrec::Number::ptr createNumberContainerForBitrangeIndexDataVariant(const BitrangeIndexDataVariant& bitRangeIndexData) {
            return std::holds_alternative<unsigned int>(bitRangeIndexData)
                ? createNumberContainerForConstantValue(std::get<unsigned int>(bitRangeIndexData))
                : createNumberContainerForLoopVariableIdentifier(std::get<std::string>(bitRangeIndexData));
        }

        explicit VariableAccessDefinition():
            accessedValuePerDimension({}), accessedBitRange(std::nullopt) {}

        explicit VariableAccessDefinition(const BitrangeIndexDataVariant& accessedBit):
            accessedValuePerDimension({}) {
            const auto containerForAccessedBit = createNumberContainerForBitrangeIndexDataVariant(accessedBit);
            accessedBitRange                   = std::make_pair(containerForAccessedBit, containerForAccessedBit);
        }

        explicit VariableAccessDefinition(const std::pair<BitrangeIndexDataVariant, BitrangeIndexDataVariant>& accessedBitRange):
            accessedValuePerDimension({}) {
            this->accessedBitRange = std::make_pair(createNumberContainerForBitrangeIndexDataVariant(accessedBitRange.first), createNumberContainerForBitrangeIndexDataVariant(accessedBitRange.second));
        }

        explicit VariableAccessDefinition(const std::vector<unsigned int>& accessedValuePerDimension)
            : accessedValuePerDimension(accessedValuePerDimension), accessedBitRange(std::nullopt) {}

        explicit VariableAccessDefinition(const std::vector<unsigned int>& accessedValuePerDimension, const BitrangeIndexDataVariant& accessedBit)
            : accessedValuePerDimension(accessedValuePerDimension) {
            const auto containerForAccessedBit = createNumberContainerForBitrangeIndexDataVariant(accessedBit);
            accessedBitRange                   = std::make_pair(containerForAccessedBit, containerForAccessedBit);
        }

        explicit VariableAccessDefinition(const std::vector<unsigned int>& accessedValuePerDimension, const std::pair<BitrangeIndexDataVariant, BitrangeIndexDataVariant>& accessedBitRange)
            : accessedValuePerDimension(accessedValuePerDimension) {
            this->accessedBitRange = std::make_pair(createNumberContainerForBitrangeIndexDataVariant(accessedBitRange.first), createNumberContainerForBitrangeIndexDataVariant(accessedBitRange.second));
        }
    };

    struct ExpectedSymmetricOverlapCheckResultData {
        std::vector<unsigned int>   accessedValuePerDimension;
        unsigned int overlappingBitForLhsOperand;
        unsigned int overlappingBitForRhsOperand;

        explicit ExpectedSymmetricOverlapCheckResultData(std::vector<unsigned int>&& accessedValuePerDimension, unsigned int overlappingBitForLhsOperand):
            accessedValuePerDimension(std::move(accessedValuePerDimension)), overlappingBitForLhsOperand(overlappingBitForLhsOperand), overlappingBitForRhsOperand(overlappingBitForLhsOperand) {}

        explicit ExpectedSymmetricOverlapCheckResultData(std::vector<unsigned int>&& accessedValuePerDimension, unsigned int overlappingBitForLhsOperand, unsigned int overlappingBitForRhsOperand):
            accessedValuePerDimension(std::move(accessedValuePerDimension)), overlappingBitForLhsOperand(overlappingBitForLhsOperand), overlappingBitForRhsOperand(overlappingBitForRhsOperand) {}

    };

    struct OverlapTestData {
        std::string                                                                         testName;
        VariableDefinition                                                                  variableDefinition;
        VariableAccessDefinition                                                            lVarAccessDefinition;
        VariableAccessDefinition                                                            rVarAccessDefinition;
        std::optional<ExpectedSymmetricOverlapCheckResultData>                              optionalExpectedOverlapData;

        explicit OverlapTestData(std::string&& testName, VariableDefinition&& variableDefinition, VariableAccessDefinition&& lVarAccessDefinition, VariableAccessDefinition&& rVarAccessDefinition):
            testName(std::move(testName)), variableDefinition(std::move(variableDefinition)), lVarAccessDefinition(std::move(lVarAccessDefinition)), rVarAccessDefinition(std::move(rVarAccessDefinition)) {}

        explicit OverlapTestData(std::string&& testName, VariableDefinition&& variableDefinition, VariableAccessDefinition&& lVarAccessDefinition, VariableAccessDefinition&& rVarAccessDefinition, ExpectedSymmetricOverlapCheckResultData&& overlapData):
            testName(std::move(testName)), variableDefinition(std::move(variableDefinition)), lVarAccessDefinition(std::move(lVarAccessDefinition)), rVarAccessDefinition(std::move(rVarAccessDefinition)), optionalExpectedOverlapData(overlapData) {}
    };

    class BaseOverlapInVariableAccessesTestFixture : public testing::TestWithParam<OverlapTestData> {
    public:
        virtual utils::VariableAccessOverlapCheckResult::OverlapState getExpectedVariableAccessOverlapCheckResult() = 0;

        void performTestExecution() {
            const auto& [_, variableDefinition, lVarAccessDefinition, rVarAccessDefinition, optionalExpectedOverlapData] = GetParam();
            const auto variableInstance                                                  = std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, variableDefinition.identifier, variableDefinition.dimensions, variableDefinition.bitwidth);

            const syrec::VariableAccess lVarAccessInstance = createVariableAccessFromDefinition(variableInstance, lVarAccessDefinition);
            const syrec::VariableAccess rVarAccessInstance = createVariableAccessFromDefinition(variableInstance, rVarAccessDefinition);

            auto expectedOverlapCheckResultUsingLhsOperandAsBase                           = utils::VariableAccessOverlapCheckResult(getExpectedVariableAccessOverlapCheckResult());
            if (optionalExpectedOverlapData.has_value())
                expectedOverlapCheckResultUsingLhsOperandAsBase.overlappingIndicesInformations = utils::VariableAccessOverlapCheckResult::OverlappingIndicesContainer({optionalExpectedOverlapData->accessedValuePerDimension, optionalExpectedOverlapData->overlappingBitForLhsOperand});
            ASSERT_NO_FATAL_FAILURE(assertEquivalenceBetweenVariableAccessOverlapOperands(lVarAccessInstance, rVarAccessInstance, expectedOverlapCheckResultUsingLhsOperandAsBase));

            auto expectedOverlapCheckResultUsingRhsOperandAsBase                           = utils::VariableAccessOverlapCheckResult(getExpectedVariableAccessOverlapCheckResult());
            if (optionalExpectedOverlapData.has_value())
                expectedOverlapCheckResultUsingRhsOperandAsBase.overlappingIndicesInformations = utils::VariableAccessOverlapCheckResult::OverlappingIndicesContainer({optionalExpectedOverlapData->accessedValuePerDimension, optionalExpectedOverlapData->overlappingBitForRhsOperand});
            ASSERT_NO_FATAL_FAILURE(assertEquivalenceBetweenVariableAccessOverlapOperands(rVarAccessInstance, lVarAccessInstance, expectedOverlapCheckResultUsingRhsOperandAsBase));
        }

    protected:
        [[nodiscard]] static syrec::VariableAccess createVariableAccessFromDefinition(const syrec::Variable::ptr& referenceVar, const VariableAccessDefinition& variableAccessDefinition) {
            syrec::VariableAccess variableAccess;
            variableAccess.setVar(referenceVar);

            for (const auto& accessedValuePerDimension: variableAccessDefinition.accessedValuePerDimension)
                variableAccess.indexes.emplace_back( createExpressionForConstantValue(accessedValuePerDimension));

            if (variableAccessDefinition.accessedBitRange.has_value()) {
                const auto accessedBitrangeStartContainer = variableAccessDefinition.accessedBitRange->first;
                const auto accessedBitrangeEndContainer   = variableAccessDefinition.accessedBitRange->second;
                variableAccess.range                      = std::make_pair(accessedBitrangeStartContainer, accessedBitrangeEndContainer);
            }
            return variableAccess;
        }
    };

    class ExpectingOverlappingVariableAccessesTestFixture: public BaseOverlapInVariableAccessesTestFixture {
        utils::VariableAccessOverlapCheckResult::OverlapState getExpectedVariableAccessOverlapCheckResult() override {
            return utils::VariableAccessOverlapCheckResult::OverlapState::Overlapping;
        }
    };

    class ExpectingPotentiallyOverlappingVariableAccessesTestFixture: public BaseOverlapInVariableAccessesTestFixture {
        utils::VariableAccessOverlapCheckResult::OverlapState getExpectedVariableAccessOverlapCheckResult() override {
            return utils::VariableAccessOverlapCheckResult::OverlapState::MaybeOverlapping;
        }
    };

    class ExpectingNotOverlappingVariableAccessesTestFixture: public BaseOverlapInVariableAccessesTestFixture {
        utils::VariableAccessOverlapCheckResult::OverlapState getExpectedVariableAccessOverlapCheckResult() override {
            return utils::VariableAccessOverlapCheckResult::OverlapState::NotOverlapping;
        }
    };
}

TEST_P(ExpectingOverlappingVariableAccessesTestFixture, ExpectingOverlap) {
    ASSERT_NO_FATAL_FAILURE(performTestExecution());
}

TEST_P(ExpectingPotentiallyOverlappingVariableAccessesTestFixture, ExpectingPotentialOverlap) {
    ASSERT_NO_FATAL_FAILURE(performTestExecution());
}

TEST_P(ExpectingNotOverlappingVariableAccessesTestFixture, ExpectingNoOverlap) {
    ASSERT_NO_FATAL_FAILURE(performTestExecution());
}

/* TODO: Parser should simplify variable declarations of the form 'in a[1][1][1]' to 'in a[1]', while the dimensions in a variable access are only allowed to be omitted in signals of the form a[1]
 * TODO: Should stringifier output signal of the form 'in a[1]' as 'in a[1]' or 'in a' (could also be configured via option).
 * TODO: How should out of range signal accesses be handled
 */

INSTANTIATE_TEST_SUITE_P(VariableAccessOverlapTests, ExpectingOverlappingVariableAccessesTestFixture,
    testing::ValuesIn({
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessNoBitRangeAccess_rAccess_NoDimensionAccessNoBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(),
            VariableAccessDefinition(),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 0)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessNoBitRangeAccess_rAccess_NoDimensionAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(),
            VariableAccessDefinition(2),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 2)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessNoBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(),
            VariableAccessDefinition(std::make_pair(2, 14)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 2)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessNoBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantStartValue",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(),
            VariableAccessDefinition(std::make_pair("$i", DEFAULT_VARIABLE_BITWIDTH - 1)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), DEFAULT_VARIABLE_BITWIDTH - 1)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessNoBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantEndValue",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(),
            VariableAccessDefinition(std::make_pair(2u, "$i")),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 2u)),

        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitAccess_rAccess_NoDimensionAccessNoBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(2),
            VariableAccessDefinition(),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 2)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitAccess_rAccess_NoDimensionAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(2),
            VariableAccessDefinition(2),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 2)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitAccess_rAccess_NoDimensionAccessBitRangeAccessStartEqualToBit",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(2),
            VariableAccessDefinition( std::make_pair(2, 14)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 2)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitAccess_rAccess_NoDimensionAccessBitRangeAccessEndEqualToBit",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(14),
            VariableAccessDefinition(std::make_pair(2, 14)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 14)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitAccess_rAccess_NoDimensionAccessBitRangeAccessOverlappingBit",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(5),
            VariableAccessDefinition(std::make_pair(2, 14)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 5)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantStartValue",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(2),
            VariableAccessDefinition(std::make_pair("$i", 2)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 2)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantEndValue",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(2),
            VariableAccessDefinition(std::make_pair(2u, "$i")),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 2)),


        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessNoBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(2, 14)),
            VariableAccessDefinition(),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 2)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(2, 14)),
            VariableAccessDefinition(5),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 5)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccessStartEqualToBit_rAccess_NoDimensionAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(2, 14)),
            VariableAccessDefinition(2),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 2)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccessEndEqualToBit_rAccess_NoDimensionAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(2, 14)),
            VariableAccessDefinition(14),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 14)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantStartValueEqualToStartBitOfLVarBitrange",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(2, DEFAULT_VARIABLE_BITWIDTH - 1)),
            VariableAccessDefinition(std::make_pair("$i", 2)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 2)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantStartValueEqualToEndBitOfLVarBitrange",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(2, DEFAULT_VARIABLE_BITWIDTH - 1)),
            VariableAccessDefinition(std::make_pair("$i", 2)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 2)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantStartValueEnclosedByAccessedBitRangeOfLVar",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(2, DEFAULT_VARIABLE_BITWIDTH - 1)),
            VariableAccessDefinition(std::make_pair("$i", 5)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 5)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantEndValueEqualToStartBitOfLVarBitrange",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(2, DEFAULT_VARIABLE_BITWIDTH - 1)),
            VariableAccessDefinition(std::make_pair(2, "$i")),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 2)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantEndValueEqualToEndBitOfLVarBitrange",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(2, DEFAULT_VARIABLE_BITWIDTH - 1)),
            VariableAccessDefinition(std::make_pair(2, "$i")),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 2)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantEndValueEnclosedByAccessedBitRangeOfLVar",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(2, DEFAULT_VARIABLE_BITWIDTH - 1)),
            VariableAccessDefinition(std::make_pair(5, "$i")),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 5)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessOverlappingFromTheLeft",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(2, 14)),
            VariableAccessDefinition(std::make_pair(1, 3)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 2)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessOverlappingFromTheRight",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(2, 14)),
            VariableAccessDefinition(std::make_pair(1, 14)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 2)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessBeingEqualToLeftOne",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(2, 14)),
            VariableAccessDefinition(std::make_pair(2, 14)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 2)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessInLeftOne",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(2, 14)),
            VariableAccessDefinition(std::make_pair(5, 8)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 5)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRange_nonConstantBitrangeStartInLVarBitrangeAccessAndBitrangeEndEqualToRVarBitrangeStart",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair("$i", 5)),
            VariableAccessDefinition(std::make_pair(5, 8)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 5)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRange_nonConstantBitrangeStartInLVarBitrangeAccessAndBitrangeEndEqualToRVarBitrangeEnd",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair("$i", 8)),
            VariableAccessDefinition(std::make_pair(8, 5)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 8)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRange_nonConstantBitrangeStartInLVarBitrangeAccessAndBitrangeEndEqualToRVarBitrangeStartWithNonConstantEnd",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair("$i", 5)),
            VariableAccessDefinition(std::make_pair(5, "$j")),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 5)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRange_nonConstantBitrangeStartInLVarBitrangeAccessAndBitrangeEndEqualToRVarBitrangeEndWithNonConstantStart",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair("$i", 8)),
            VariableAccessDefinition(std::make_pair("$j", 8)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 8)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRange_nonConstantBitrangeEndInLVarBitrangeAccessAndBitrangeEndEqualToRVarBitrangeStart",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(5, "$i")),
            VariableAccessDefinition(std::make_pair(5, 8)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 5)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRange_nonConstantBitrangeEndInLVarBitrangeAccessAndBitrangeEndEqualToRVarBitrangeEnd",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(8, "$i")),
            VariableAccessDefinition(std::make_pair(8, 5)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 8)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRange_nonConstantBitrangeEndInLVarBitrangeAccessAndBitrangeEndEqualToRVarBitrangeStartWithNonConstantEnd",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(5, "$i")),
            VariableAccessDefinition(std::make_pair(5, "$j")),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u}), 5)),
        
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessNoBitRangeAccess_rAccess_DimensionAccessNoBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u})),
            VariableAccessDefinition(std::vector({1u})),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), 0)),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessNoBitRangeAccess_rAccess_DimensionAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u})),
            VariableAccessDefinition(std::vector({1u}), 2),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), 2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessNoBitRangeAccess_rAccess_DimensionAccessBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u})),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, 14)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), 2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_NoDimensionAccessNoBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantStartValue",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u})),
            VariableAccessDefinition(std::vector({1u}), std::make_pair("$i", DEFAULT_VARIABLE_BITWIDTH - 1)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH - 1)),
        OverlapTestData("1DSignalMultipleValues_lAccess_NoDimensionAccessNoBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantEndValue",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u})),
            VariableAccessDefinition(std::vector({1u}) , std::make_pair(2u, "$i")),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), 2)),

        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessBitAccess_rAccess_DimensionAccessNoBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), 2),
            VariableAccessDefinition(std::vector({1u})),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), 2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessBitAccess_rAccess_DimensionAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), 2),
            VariableAccessDefinition(std::vector({1u}), 2),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), 2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessBitAccess_rAccess_DimensionAccessBitRangeAccessOverlappingFromTheLeft",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), 2),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, 14)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), 2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessBitAccess_rAccess_DimensionAccessBitRangeAccessOverlappingFromTheRight",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), 2),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(0, 3)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), 2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessBitAccess_rAccess_DimensionAccessBitRangeAccessEqualToBit",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), 2),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, 2)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), 2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_NoDimensionAccessBitAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantStartValue",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), 2),
            VariableAccessDefinition(std::vector({1u}), std::make_pair("$i", 2)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), 2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_NoDimensionAccessBitAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantEndValue",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), 2),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2u, "$i")),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), 2)),

        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessBitRangeAccess_rAccess_DimensionAccessNoBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, 14)),
            VariableAccessDefinition(std::vector({1u})),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), 2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessBitRangeAccess_rAccess_DimensionAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, 14)),
            VariableAccessDefinition(std::vector({1u}), 2),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), 2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessBitRangeAccess_rAccess_DimensionAccessBitRangeAccessOverlappingFromTheLeft",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, 14)),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(0, 3)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), 2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessBitRangeAccess_rAccess_DimensionAccessBitRangeAccessOverlappingFromTheRight",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, 14)),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(1, 14)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), 2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessBitRangeAccess_rAccess_DimensionAccessBitRangeAccessEqualToLeftAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, 14)),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, 14)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), 2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessBitRangeAccess_rAccess_DimensionAccessBitRangeAccessInLeftOne",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, 14)),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(5, 8)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), 5)),
        OverlapTestData("1DSignalMultipleValues_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantStartValueEqualToStartBitOfLVarBitrange",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, DEFAULT_VARIABLE_BITWIDTH - 1)),
            VariableAccessDefinition(std::vector({1u}), std::make_pair("$i", 2)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), 2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantStartValueEqualToEndBitOfLVarBitrange",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, DEFAULT_VARIABLE_BITWIDTH - 1)),
            VariableAccessDefinition(std::vector({1u}), std::make_pair("$i", 2)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), 2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantStartValueEnclosedByAccessedBitRangeOfLVar",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, DEFAULT_VARIABLE_BITWIDTH - 1)),
            VariableAccessDefinition(std::vector({1u}), std::make_pair("$i", 5)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), 5)),
        OverlapTestData("1DSignalMultipleValues_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantEndValueEqualToStartBitOfLVarBitrange",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, DEFAULT_VARIABLE_BITWIDTH - 1)),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, "$i")),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), 2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantEndValueEqualToEndBitOfLVarBitrange",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, DEFAULT_VARIABLE_BITWIDTH - 1)),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, "$i")),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), 2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantEndValueEnclosedByAccessedBitRangeOfLVar",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, DEFAULT_VARIABLE_BITWIDTH - 1)),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(5, "$i")),
            ExpectedSymmetricOverlapCheckResultData(std::vector({1u}), 5)),

        OverlapTestData("NDSignal_lAccess_NoBitRangeAccess_rAccessNoBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u,1u})),
            VariableAccessDefinition(std::vector({0u,1u})),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 0)),
        OverlapTestData("NDSignal_lAccess_NoBitRangeAccess_rAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u})),
            VariableAccessDefinition(std::vector({0u, 1u}), 2),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 2)),
        OverlapTestData("NDSignal_lAccess_NoBitRangeAccess_rAccessBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u})),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, 5)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 2)),
        OverlapTestData("NDSignal_lAccess_NoDimensionAccessNoBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantStartValue",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u})),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair("$i", DEFAULT_VARIABLE_BITWIDTH - 1)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), DEFAULT_VARIABLE_BITWIDTH - 1)),
        OverlapTestData("NDSignal_lAccess_NoDimensionAccessNoBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantEndValue",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u})),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2u, "$i")),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 2)),

        OverlapTestData("NDSignal_lAccess_BitAccess_rAccessNoBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), 2),
            VariableAccessDefinition(std::vector({0u, 1u})),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 2)),
        OverlapTestData("NDSignal_lAccess_BitAccess_rAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), 2),
            VariableAccessDefinition(std::vector({0u, 1u}), 2),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 2)),
        OverlapTestData("NDSignal_lAccess_BitAccess_rAccessBitRangeAccessOverlappingBitFromTheLeft",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), 2),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(0, 8)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 2)),
        OverlapTestData("NDSignal_lAccess_BitAccess_rAccessBitRangeAccessWithStartEqualToBit",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), 2),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, 8)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 2)),
        OverlapTestData("NDSignal_lAccess_BitAccess_rAccessBitRangeAccessWithEndEqualToBit",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), 2),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(0, 2)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 2)),
        OverlapTestData("NDSignal_lAccess_BitAccess_rAccessBitRangeAccessEnclosingBitOfLAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), 2),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(0, 8)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 2)),
        OverlapTestData("NDSignal_lAccess_BitAccess_rAccessBitRangeAccessOverlappingBitFromTheRight",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), 2),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(8, 0)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 2)),
        OverlapTestData("NDSignal_lAccess_NoDimensionAccessBitAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantStartValue",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), 2),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair("$i", 2)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 2)),
        OverlapTestData("NDSignal_lAccess_NoDimensionAccessBitAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantEndValue",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), 2),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2u, "$i")),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 2)),

        OverlapTestData("NDSignal_lAccess_BitRangeAccesss_rAccessNoBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, 5)),
            VariableAccessDefinition(std::vector({0u, 1u})),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 2)),
        OverlapTestData("NDSignal_lAccess_BitRangeAccessStartEqualToBit_rAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, 5)),
            VariableAccessDefinition(std::vector({0u, 1u}), 2),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 2)),
        OverlapTestData("NDSignal_lAccess_BitRangeAccessEndEqualToBit_rAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, 5)),
            VariableAccessDefinition(std::vector({0u, 1u}), 5),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 5)),
        OverlapTestData("NDSignal_lAccess_BitRangeAccessEnclosingBit_rAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, 5)),
            VariableAccessDefinition(std::vector({0u, 1u}), 4),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 4)),
        OverlapTestData("NDSignal_lAccess_BitRangeAccessOverlappingOtherFromTheLeft_rAccessBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(0, 4)),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, 5)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 2)),
        OverlapTestData("NDSignal_lAccess_BitRangeAccessOverlappingOtherFromTheLeft_rAccessBitRangeAccessWithLhsBitrangeStartLargerThanEnd",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(4, 0)),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, 5)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 4, 2)),
        OverlapTestData("NDSignal_lAccess_BitRangeAccessOverlappingOtherFromTheLeft_rAccessBitRangeAccessWithRhsBitrangeStartLargerThanEnd",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(0, 4)),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(5, 2)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 2, 4)),
        OverlapTestData("NDSignal_lAccess_BitRangeAccessOverlappingOtherFromTheRight_rAccessBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(8, 0)),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, 5)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 5, 2)),
        OverlapTestData("NDSignal_lAccess_BitRangeAccessOverlappingOtherFromTheRight_rAccessBitRangeAccessWithRhsBitrangeStartLargerThanEnd",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(8, 0)),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(5, 2)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 5, 5)),
        OverlapTestData("NDSignal_lAccess_BitRangeAccessEqualToOther_rAccessBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, 5)),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, 5)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 2)),
        OverlapTestData("NDSignal_lAccess_BitRangeAccessEnclosingOther_rAccessBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(0, 8)),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, 5)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 2)),
        OverlapTestData("NDSignal_lAccess_BitRangeAccessEnclosingOther_rAccessBitRangeAccess_BothBitrangesWithStartLargerThanEnd",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(8, 0)),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(5, 2)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 5, 5)),
        OverlapTestData("NDSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantStartValueEqualToStartBitOfLVarBitrange",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, DEFAULT_VARIABLE_BITWIDTH - 1)),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair("$i", 2)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 2)),
        OverlapTestData("NDSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantStartValueEqualToEndBitOfLVarBitrange", 
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, DEFAULT_VARIABLE_BITWIDTH - 1)),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair("$i", 2)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 2)),
        OverlapTestData("NDSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantStartValueEnclosedByAccessedBitRangeOfLVar",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, DEFAULT_VARIABLE_BITWIDTH - 1)),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair("$i", 5)),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 5)),
        OverlapTestData("NDSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantEndValueEqualToStartBitOfLVarBitrange",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, DEFAULT_VARIABLE_BITWIDTH - 1)),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, "$i")),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 2)),
        OverlapTestData("NDSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantEndValueEqualToEndBitOfLVarBitrange",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, DEFAULT_VARIABLE_BITWIDTH - 1)),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, "$i")),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 2)),
        OverlapTestData("NDSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessWithNonConstantEndValueEnclosedByAccessedBitRangeOfLVar",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, DEFAULT_VARIABLE_BITWIDTH - 1)),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(5, "$i")),
            ExpectedSymmetricOverlapCheckResultData(std::vector({0u, 1u}), 5)),
    }), [](const testing::TestParamInfo<ExpectingOverlappingVariableAccessesTestFixture::ParamType>& info) {
        return info.param.testName;
    });

INSTANTIATE_TEST_SUITE_P(VariableAccessOverlapTests, ExpectingPotentiallyOverlappingVariableAccessesTestFixture,
    testing::ValuesIn({
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessNoBitRangeAccess_rAccess_NoDimensionAccessNoBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(),
            VariableAccessDefinition())
    }), [](const testing::TestParamInfo<ExpectingPotentiallyOverlappingVariableAccessesTestFixture::ParamType>& info) {
        return info.param.testName;
    });

INSTANTIATE_TEST_SUITE_P(VariableAccessOverlapTests, ExpectingNotOverlappingVariableAccessesTestFixture,
    testing::ValuesIn({
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitAccess_rAccess_BitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(2),
            VariableAccessDefinition(4)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitAccess_rAccess_BitRangeAccessSmallerThanBit",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(4),
            VariableAccessDefinition(std::make_pair(0, 2))),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitAccess_rAccess_BitRangeAccessLargerThanBit",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(4),
            VariableAccessDefinition(std::make_pair(5, 7))),

        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_BitAccessSmallerThanBitrange",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(5,7)),
            VariableAccessDefinition(2)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_BitAccessLargerThanBitrange",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(5, 7)),
            VariableAccessDefinition(8)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_BitRangeAccessSmallerThanBitrange",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(5, 7)),
            VariableAccessDefinition(std::make_pair(0,3))),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_BitRangeAccessLargerThanBitrange",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(5, 7)),
            VariableAccessDefinition(std::make_pair(9,11))),

        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessNoBitRangeAccess_rAccess_DimensionAccessWithNoBitAccessAndDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u})),
            VariableAccessDefinition(std::vector({2u}))),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessNoBitRangeAccess_rAccess_DimensionAccessWithBitAccessAndDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u})),
            VariableAccessDefinition(std::vector({2u}), 2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessNoBitRangeAccess_rAccess_DimensionAccessWithBitRangeAccessAndDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u})),
            VariableAccessDefinition(std::vector({2u}), std::make_pair(2,4))),


        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessWithBitAccess_rAccess_DimensionAccessWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), 2),
            VariableAccessDefinition(std::vector({2u}))),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessWithBitAccess_rAccess_DimensionAccessWithAccessOnSameBitWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), 2),
            VariableAccessDefinition(std::vector({2u}), 2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessWithBitAccess_rAccess_DimensionAccessWithAccessOnDifferentBitWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), 2),
            VariableAccessDefinition(std::vector({2u}), 3)),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessWithBitAccess_rAccess_DimensionAccessWithBitrangeNotOverlappingAndSmallerThanBitWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), 3),
            VariableAccessDefinition(std::vector({2u}), std::make_pair(0, 2))),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessWithBitAccess_rAccess_DimensionAccessWithBitrangeNotOverlappingAndLargerThanBitWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), 3),
            VariableAccessDefinition(std::vector({2u}), std::make_pair(5,7))),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessWithBitAccess_rAccess_DimensionAccessWithBitrangeOverlappingAndSmallerThanBitWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), 3),
            VariableAccessDefinition(std::vector({2u}), std::make_pair(0, 6))),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessWithBitAccess_rAccess_DimensionAccessWithBitrangeOverlappingAndLargerThanBitWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), 3),
            VariableAccessDefinition(std::vector({2u}), std::make_pair(6, 2))),

        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessWithBitAccess_rAccess_DimensionAccessWithAccessOnDifferentBitWithSameValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), 3),
            VariableAccessDefinition(std::vector({1u}), 2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessWithBitAccess_rAccess_DimensionAccessWithBitrangeNotOverlappingBitWithSameValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), 3),
            VariableAccessDefinition(std::vector({1u}), 2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessWithBitAccess_rAccess_DimensionAccessWithBitrangeNotOverlappingAndSmallerThanBitWithSameValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), 2),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(0, 1))),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessWithBitAccess_rAccess_DimensionAccessWithBitrangeNotOverlappingAndLargerThanBitWithSameValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), 2),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(3, 5))),
        
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessWithBitRangeAccess_rAccess_DimensionAccessWithBitAccessInBitrangeWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(3,5)),
            VariableAccessDefinition(std::vector({2u}), 4)),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessWithBitRangeAccess_rAccess_DimensionAccessWithBitAccessEqualToBitrangeStartWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(3,7)),
            VariableAccessDefinition(std::vector({2u}), 3)),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessWithBitRangeAccess_rAccess_DimensionAccessWithBitAccessEqualToBitrangeEndWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(3,7)),
            VariableAccessDefinition(std::vector({2u}), 7)),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessWithBitRangeAccess_rAccess_DimensionAccessWithBitSmallerThanBitrangeWithSameValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(3,7)),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(0,2))),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessWithBitRangeAccess_rAccess_DimensionAccessWithBitLargerThanBitrangeWithSameValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(3,7)),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(9,11))),

        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessWithBitRangeAccess_rAccess_DimensionAccessWithBitRangeEqualToOtherWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(3,7)),
            VariableAccessDefinition(std::vector({2u}), std::make_pair(3,7))),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessWithBitRangeAccess_rAccess_DimensionAccessWithBitRangeEnclosedToOtherWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(3,7)),
            VariableAccessDefinition(std::vector({2u}), std::make_pair(5,6))),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessWithBitRangeAccess_rAccess_DimensionAccessWithBitRangeOverlappingOtherFromTheLeftWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(3,7)),
            VariableAccessDefinition(std::vector({2u}), std::make_pair(0,5))),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessWithBitRangeAccess_rAccess_DimensionAccessWithBitRangeOverlappingOtherFromTheRightWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(3,7)),
            VariableAccessDefinition(std::vector({2u}), std::make_pair(10,5))),

        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessWithBitRangeAccess_rAccess_DimensionAccessWithBitRangeSmallerThanOtherWithSameValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(3,7)),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(0,2))),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessWithBitRangeAccess_rAccess_DimensionAccessWithBitRangeLargerThanOtherWithSameValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(3,7)),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(9,11))),


        
        OverlapTestData("NDSignal_lAccess_DimensionAccessWithBitAccess_rAccess_DimensionAccessWithDifferentValueOfDimension_FirstAccessedValueDifferent",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 2u}), 2),
            VariableAccessDefinition(std::vector({0u, 1u}))),
        OverlapTestData("NDSignal_lAccess_DimensionAccessWithBitAccess_rAccess_DimensionAccessWithDifferentValueOfDimension_OtherAccessedValueDifferent",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 2u}), 2),
            VariableAccessDefinition(std::vector({1u, 2u}))),
        OverlapTestData("NDSignal_lAccess_DimensionAccessWithBitAccess_rAccess_DimensionAccessWithDifferentValueOfDimension_MultipleAccessedValuesDifferent",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 2u}), 2),
            VariableAccessDefinition(std::vector({1u, 1u}))),
        OverlapTestData("NDSignal_lAccess_DimensionAccessWithBitAccess_rAccess_DimensionAccessWithAccessOnSameBitWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 2u}), 2),
            VariableAccessDefinition(std::vector({1u,2u}), 2)),
        OverlapTestData("NDSignal_lAccess_DimensionAccessWithBitAccess_rAccess_DimensionAccessWithAccessOnDifferentBitWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 2u}), 2),
            VariableAccessDefinition(std::vector({0u, 1u}), 3)),
        OverlapTestData("NDSignal_lAccess_DimensionAccessWithBitAccess_rAccess_DimensionAccessWithBitrangeNotOverlappingAndSmallerThanBitWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 2u}), 2),
            VariableAccessDefinition(std::vector({1u, 2u}), std::make_pair(0, 1))),
        OverlapTestData("NDSignal_lAccess_DimensionAccessWithBitAccess_rAccess_DimensionAccessWithBitrangeNotOverlappingAndLargerThanBitWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 2u}), 2),
            VariableAccessDefinition(std::vector({1u,2u}), std::make_pair(5,7))),
        OverlapTestData("NDSignal_lAccess_DimensionAccessWithBitAccess_rAccess_DimensionAccessWithBitrangeOverlappingAndSmallerThanBitWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 2u}), 2),
            VariableAccessDefinition(std::vector({1u, 2u}), std::make_pair(0, 3))),
        OverlapTestData("NDSignal_lAccess_DimensionAccessWithBitAccess_rAccess_DimensionAccessWithBitrangeOverlappingAndLargerThanBitWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 2u}), 2),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(7,0))),

        OverlapTestData("NDSignal_lAccess_DimensionAccessWithBitAccess_rAccess_DimensionAccessWithAccessOnDifferentBitWithSameValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 2u}), 2),
            VariableAccessDefinition(std::vector({0u,2u}), 3)),
        
        OverlapTestData("NDSignal_lAccess_DimensionAccessWithBitAccess_rAccess_DimensionAccessWithBitrangeNotOverlappingAndSmallerThanBitWithSameValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 2u}), 2),
            VariableAccessDefinition(std::vector({0u, 2u}), 1)),
        OverlapTestData("NDSignal_lAccess_DimensionAccessWithBitAccess_rAccess_DimensionAccessWithBitrangeNotOverlappingAndLargerThanBitWithSameValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 2u}), 2),
            VariableAccessDefinition(std::vector({0u, 2u}), 5)),

        OverlapTestData("NDSignal_lAccess_DimensionAccessWithBitRangeAccess_rAccess_DimensionAccessWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u,1u}), std::make_pair(3,7)),
            VariableAccessDefinition(std::vector({1u, 0u}))),
        OverlapTestData("NDSignal_lAccess_DimensionAccessWithBitRangeAccess_rAccess_DimensionAccessWithBitAccessInBitrangeWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(3, 7)),
            VariableAccessDefinition(std::vector({1u, 1u}), 5)),
        OverlapTestData("NDSignal_lAccess_DimensionAccessWithBitRangeAccess_rAccess_DimensionAccessWithBitAccessEqualToBitrangeStartWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(3, 7)),
            VariableAccessDefinition(std::vector({0u, 2u}), 3)),
        OverlapTestData("NDSignal_lAccess_DimensionAccessWithBitRangeAccess_rAccess_DimensionAccessWithBitAccessEqualToBitrangeEndWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(3, 7)),
            VariableAccessDefinition(std::vector({1u, 1u}), 7)),
        OverlapTestData("NDSignal_lAccess_DimensionAccessWithBitRangeAccess_rAccess_DimensionAccessWithBitSmallerThanBitrangeWithSameValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(3, 7)),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(0, 2))),
        OverlapTestData("NDSignal_lAccess_DimensionAccessWithBitRangeAccess_rAccess_DimensionAccessWithBitLargerThanBitrangeWithSameValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(3, 7)),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(10,11))),

        OverlapTestData("NDSignal_lAccess_DimensionAccessWithBitRangeAccess_rAccess_DimensionAccessWithBitRangeEqualToOtherWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(3,7)),
            VariableAccessDefinition(std::vector({1u, 1u}), std::make_pair(3,7))),
        OverlapTestData("NDSignal_lAccess_DimensionAccessWithBitRangeAccess_rAccess_DimensionAccessWithBitRangeEnclosedToOtherWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(3,7)),
            VariableAccessDefinition(std::vector({0u, 2u}), std::make_pair(4,6))),
        OverlapTestData("NDSignal_lAccess_DimensionAccessWithBitRangeAccess_rAccess_DimensionAccessWithBitRangeOverlappingOtherFromTheLeftWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(3,7)),
            VariableAccessDefinition(std::vector({0u, 2u}), std::make_pair(1, 5))),
        OverlapTestData("NDSignal_lAccess_DimensionAccessWithBitRangeAccess_rAccess_DimensionAccessWithBitRangeOverlappingOtherFromTheRightWithDifferentValueOfDimension",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(3,7)),
            VariableAccessDefinition(std::vector({1u, 2u}), std::make_pair(11, 0)))


    }), [](const testing::TestParamInfo<ExpectingNotOverlappingVariableAccessesTestFixture::ParamType>& info) {
        return info.param.testName;
    });

// TODO: Tests for invalid variable accesses (nullptr expressions, etc.), missmatching reference variables?
// TODO: Tests for overlap of partial variable accesses?
// TODO: How should indices out of range be handled (accessed value of dimension or bit range)

// TODO: Defined expected behaviour for cases in which the user provides invalid values for the bit range start and/or end (i.e. nullptrs)
TEST(VariableAccessOverlapTests, ReferenceVariableNotSetCorrectlyDetected) {
    const std::vector<unsigned int> variableDimensions        = {1, 2};
    const auto                      rOperandReferenceVariable = createVariableInstance(DEFAULT_VARIABLE_IDENTIFIER + "otherIdent", variableDimensions, DEFAULT_VARIABLE_BITWIDTH);
    ASSERT_THAT(rOperandReferenceVariable, testing::NotNull());

    auto lhsVariableAccess = syrec::VariableAccess();
    lhsVariableAccess.setVar(nullptr);
    lhsVariableAccess.indexes = {createExpressionForConstantValue(0), createExpressionForConstantValue(1)};

    auto rhsVariableAccess = syrec::VariableAccess();
    rhsVariableAccess.setVar(rOperandReferenceVariable);
    rhsVariableAccess.indexes = lhsVariableAccess.indexes;
    ASSERT_NO_FATAL_FAILURE(assertSymmetricVariableAccessOverlapResultCannotBeDetermined(lhsVariableAccess, rhsVariableAccess));
}

TEST(VariableAccessOverlapTests, MissmatchInReferenceVariableIdentifierDetectedCorrectly) {
    const std::vector<unsigned int> variableDimensions       = {1, 2};
    const auto                      lOperandReferenceVariable = createVariableInstance(DEFAULT_VARIABLE_IDENTIFIER, variableDimensions, DEFAULT_VARIABLE_BITWIDTH);
    ASSERT_THAT(lOperandReferenceVariable, testing::NotNull());

    const auto rOperandReferenceVariable = createVariableInstance(DEFAULT_VARIABLE_IDENTIFIER + "otherIdent", variableDimensions, DEFAULT_VARIABLE_BITWIDTH);
    ASSERT_THAT(rOperandReferenceVariable, testing::NotNull());

    auto lhsVariableAccess = syrec::VariableAccess();
    lhsVariableAccess.setVar(lOperandReferenceVariable);
    lhsVariableAccess.indexes = {createExpressionForConstantValue(0), createExpressionForConstantValue(1)};

    auto rhsVariableAccess = syrec::VariableAccess();
    rhsVariableAccess.setVar(rOperandReferenceVariable);
    rhsVariableAccess.indexes = lhsVariableAccess.indexes;
    ASSERT_NO_FATAL_FAILURE(assertSymmetricVariableAccessOverlapResultCannotBeDetermined(lhsVariableAccess, rhsVariableAccess));
}

TEST(VariableAccessOverlapTests, MissmatchInReferenceVariableBitwidthDetectedCorrectly) {
    const std::vector<unsigned int> variableDimensions        = {1, 2};
    const auto                      lOperandReferenceVariable = createVariableInstance(DEFAULT_VARIABLE_IDENTIFIER, variableDimensions, DEFAULT_VARIABLE_BITWIDTH);
    ASSERT_THAT(lOperandReferenceVariable, testing::NotNull());

    const auto rOperandReferenceVariable = createVariableInstance(lOperandReferenceVariable->name, variableDimensions, lOperandReferenceVariable->bitwidth + 2);
    ASSERT_THAT(rOperandReferenceVariable, testing::NotNull());

    auto lhsVariableAccess = syrec::VariableAccess();
    lhsVariableAccess.setVar(lOperandReferenceVariable);
    lhsVariableAccess.indexes = {createExpressionForConstantValue(0), createExpressionForConstantValue(1)};

    auto rhsVariableAccess = syrec::VariableAccess();
    rhsVariableAccess.setVar(rOperandReferenceVariable);
    rhsVariableAccess.indexes = lhsVariableAccess.indexes;
    ASSERT_NO_FATAL_FAILURE(assertSymmetricVariableAccessOverlapResultCannotBeDetermined(lhsVariableAccess, rhsVariableAccess));

    auto rOperandReferenceVariableWithSmallerBitwidth = std::make_shared<syrec::Variable>(*rOperandReferenceVariable);
    rOperandReferenceVariable->bitwidth               = lOperandReferenceVariable->bitwidth - 2;
    rhsVariableAccess.setVar(rOperandReferenceVariable);
    ASSERT_NO_FATAL_FAILURE(assertSymmetricVariableAccessOverlapResultCannotBeDetermined(lhsVariableAccess, rhsVariableAccess));
}

TEST(VariableAccessOverlapTests, MissmatchInReferenceVariableDimensionsDetectedCorrectly) {
    const std::vector<unsigned int> variableDimensions        = {1, 2};
    const auto                      lOperandReferenceVariable = createVariableInstance(DEFAULT_VARIABLE_IDENTIFIER, variableDimensions, DEFAULT_VARIABLE_BITWIDTH);
    ASSERT_THAT(lOperandReferenceVariable, testing::NotNull());

    auto lhsVariableAccess = syrec::VariableAccess();
    lhsVariableAccess.setVar(lOperandReferenceVariable);
    lhsVariableAccess.indexes = {createExpressionForConstantValue(0), createExpressionForConstantValue(1)};

    const std::vector<std::vector<unsigned int>> rOperandReferenceVariableValuesPerDimension = {
        {1,2,3}, {1}, {2,1}, {0, 2}
    };

    const auto rOperandReferenceVariable = createVariableInstance(DEFAULT_VARIABLE_IDENTIFIER, {}, DEFAULT_VARIABLE_BITWIDTH);
    ASSERT_THAT(rOperandReferenceVariable, testing::NotNull());

    auto rhsVariableAccess = syrec::VariableAccess();
    rhsVariableAccess.setVar(rOperandReferenceVariable);

    for (const auto& numValuesPerDimensionOfRhsVariableAccess : rOperandReferenceVariableValuesPerDimension) {

        rOperandReferenceVariable->dimensions = numValuesPerDimensionOfRhsVariableAccess;
        rhsVariableAccess.indexes.clear();
        for (std::size_t i = 0; i < numValuesPerDimensionOfRhsVariableAccess.size(); ++i)
            rhsVariableAccess.indexes.emplace_back(createExpressionForConstantValue(0));

        ASSERT_NO_FATAL_FAILURE(assertSymmetricVariableAccessOverlapResultCannotBeDetermined(lhsVariableAccess, rhsVariableAccess));
    }
}

TEST(VariableAccessOverlapTests, InvalidValueForBitrangeStartValueResultsInPotentialOverlapAndNoCrash) {
    const std::vector<unsigned int> variableDimensions = {1, 2};
    const auto accessedVariableInstance = createVariableInstance(DEFAULT_VARIABLE_IDENTIFIER, variableDimensions, DEFAULT_VARIABLE_BITWIDTH);
    ASSERT_THAT(accessedVariableInstance, testing::NotNull());

    auto lhsVariableAccess = syrec::VariableAccess();
    lhsVariableAccess.setVar(accessedVariableInstance);
    lhsVariableAccess.indexes = {createExpressionForConstantValue(0), createExpressionForConstantValue(1)};
    lhsVariableAccess.range   = std::make_pair(nullptr, createNumberContainerForConstantValue(DEFAULT_VARIABLE_BITWIDTH - 1));

    auto rhsVariableAccess = syrec::VariableAccess();
    rhsVariableAccess.setVar(accessedVariableInstance);
    rhsVariableAccess.indexes = {createExpressionForConstantValue(0), createExpressionForConstantValue(1)};
    rhsVariableAccess.range = std::make_pair(createNumberContainerForConstantValue(0), lhsVariableAccess.range->first);
    ASSERT_NO_FATAL_FAILURE(assertSymmetricEquivalenceBetweenPotentiallyVariableAccessOverlapOperands(lhsVariableAccess, rhsVariableAccess));
}

TEST(VariableAccessOverlapTests, InvalidValueForBitrangeEndValueResultsInPotentialOverlapAndNoCrash) {
    GTEST_SKIP();
    const std::vector<unsigned int> variableDimensions       = {1, 2};
    const auto                      accessedVariableInstance = createVariableInstance(DEFAULT_VARIABLE_IDENTIFIER, variableDimensions, DEFAULT_VARIABLE_BITWIDTH);
    ASSERT_THAT(accessedVariableInstance, testing::NotNull());

    auto lhsVariableAccess = syrec::VariableAccess();
    lhsVariableAccess.setVar(accessedVariableInstance);
    lhsVariableAccess.indexes = {createExpressionForConstantValue(0), createExpressionForConstantValue(1)};
    lhsVariableAccess.range   = std::make_pair(createNumberContainerForConstantValue(0), nullptr);

    auto rhsVariableAccess = syrec::VariableAccess();
    rhsVariableAccess.setVar(accessedVariableInstance);
    rhsVariableAccess.indexes = {createExpressionForConstantValue(0), createExpressionForConstantValue(1)};
    rhsVariableAccess.range   = std::make_pair(createNumberContainerForConstantValue(2), createNumberContainerForConstantValue(DEFAULT_VARIABLE_BITWIDTH - 1));
    ASSERT_NO_FATAL_FAILURE(assertSymmetricEquivalenceBetweenPotentiallyVariableAccessOverlapOperands(lhsVariableAccess, rhsVariableAccess));
}

TEST(VariableAccessOverlapTests, InvalidValueForBothBitrangeStartAndAendResultsInPotentialOverlapAndNoCrash) {
    const std::vector<unsigned int> variableDimensions       = {1, 2};
    const auto                      accessedVariableInstance = createVariableInstance(DEFAULT_VARIABLE_IDENTIFIER, variableDimensions, DEFAULT_VARIABLE_BITWIDTH);
    ASSERT_THAT(accessedVariableInstance, testing::NotNull());

    auto lhsVariableAccess = syrec::VariableAccess();
    lhsVariableAccess.setVar(accessedVariableInstance);
    lhsVariableAccess.indexes = {createExpressionForConstantValue(0), createExpressionForConstantValue(1)};
    lhsVariableAccess.range   = std::make_pair(nullptr, nullptr);

    auto rhsVariableAccess = syrec::VariableAccess();
    rhsVariableAccess.setVar(accessedVariableInstance);
    rhsVariableAccess.indexes = {createExpressionForConstantValue(0), createExpressionForConstantValue(1)};
    rhsVariableAccess.range   = std::make_pair(createNumberContainerForConstantValue(0), lhsVariableAccess.range->first);
    ASSERT_NO_FATAL_FAILURE(assertSymmetricEquivalenceBetweenPotentiallyVariableAccessOverlapOperands(lhsVariableAccess, rhsVariableAccess));
}

TEST(VariableAccessOverlapTests, NonConstantValueForBitrangeStartValueResultsInPotentialOverlapAndNoCrash) {
    const std::vector<unsigned int> variableDimensions       = {1, 2};
    const auto                      accessedVariableInstance = createVariableInstance(DEFAULT_VARIABLE_IDENTIFIER, variableDimensions, DEFAULT_VARIABLE_BITWIDTH);
    ASSERT_THAT(accessedVariableInstance, testing::NotNull());

    auto lhsVariableAccess = syrec::VariableAccess();
    lhsVariableAccess.setVar(accessedVariableInstance);
    lhsVariableAccess.indexes = {createExpressionForConstantValue(0), createExpressionForConstantValue(1)};
    lhsVariableAccess.range   = std::make_pair(createNumberContainerForLoopVariableIdentifier("$i"), createNumberContainerForConstantValue(DEFAULT_VARIABLE_BITWIDTH - 1));

    auto rhsVariableAccess = syrec::VariableAccess();
    rhsVariableAccess.setVar(accessedVariableInstance);
    rhsVariableAccess.indexes = {createExpressionForConstantValue(0), createExpressionForConstantValue(1)};
    rhsVariableAccess.range   = std::make_pair(createNumberContainerForConstantValue(0), lhsVariableAccess.range->first);
    ASSERT_NO_FATAL_FAILURE(assertSymmetricEquivalenceBetweenPotentiallyVariableAccessOverlapOperands(lhsVariableAccess, rhsVariableAccess));
}

// Maybe overlapping
TEST(VariableAccessOverlapTests, NonConstantValueForBothBitrangeStartAndAendResultsInPotentialOverlapAndNoCrash) {
    const std::vector<unsigned int> variableDimensions       = {1, 2};
    const auto                      accessedVariableInstance = createVariableInstance(DEFAULT_VARIABLE_IDENTIFIER, variableDimensions, DEFAULT_VARIABLE_BITWIDTH);
    ASSERT_THAT(accessedVariableInstance, testing::NotNull());

    auto lhsVariableAccess = syrec::VariableAccess();
    lhsVariableAccess.setVar(accessedVariableInstance);
    lhsVariableAccess.indexes = {createExpressionForConstantValue(0), createExpressionForConstantValue(1)};
    lhsVariableAccess.range   = std::make_pair(createNumberContainerForLoopVariableIdentifier("$j"), createNumberContainerForLoopVariableIdentifier("$i"));

    auto rhsVariableAccess = syrec::VariableAccess();
    rhsVariableAccess.setVar(accessedVariableInstance);
    rhsVariableAccess.indexes = {createExpressionForConstantValue(0), createExpressionForConstantValue(1)};
    rhsVariableAccess.range   = std::make_pair(createNumberContainerForConstantValue(0), lhsVariableAccess.range->first);
    ASSERT_NO_FATAL_FAILURE(assertSymmetricEquivalenceBetweenPotentiallyVariableAccessOverlapOperands(lhsVariableAccess, rhsVariableAccess));
}

TEST(VariableAccessOverlapTests, InvalidValueForAccessedValueOfDimensionResultsInPotentialOverlapAndNoCrash) {
    const std::vector<unsigned int> variableDimensions       = {1, 2};
    const auto                      accessedVariableInstance = createVariableInstance(DEFAULT_VARIABLE_IDENTIFIER, variableDimensions, DEFAULT_VARIABLE_BITWIDTH);
    ASSERT_THAT(accessedVariableInstance, testing::NotNull());

    auto lhsVariableAccess = syrec::VariableAccess();
    lhsVariableAccess.setVar(accessedVariableInstance);
    lhsVariableAccess.indexes = {nullptr, createExpressionForConstantValue(1)};

    auto rhsVariableAccess = syrec::VariableAccess();
    rhsVariableAccess.setVar(accessedVariableInstance);
    rhsVariableAccess.indexes = {createExpressionForConstantValue(0), createExpressionForConstantValue(1)};
    rhsVariableAccess.range   = std::make_pair(createNumberContainerForConstantValue(0),  createNumberContainerForConstantValue(accessedVariableInstance->bitwidth - 1));
    ASSERT_NO_FATAL_FAILURE(assertSymmetricEquivalenceBetweenPotentiallyVariableAccessOverlapOperands(lhsVariableAccess, rhsVariableAccess));
}

TEST(VariableAccessOverlapTests, NonConstantValueForAccessedValueOfDimensionResultsInPotentialOverlapAndNoCrash) {
    const std::vector<unsigned int> variableDimensions       = {1, 2};
    const auto                      accessedVariableInstance = createVariableInstance(DEFAULT_VARIABLE_IDENTIFIER, variableDimensions, DEFAULT_VARIABLE_BITWIDTH);
    ASSERT_THAT(accessedVariableInstance, testing::NotNull());

    const auto binaryExpressionDefiningAccessedValueOfFirstDimension = std::make_shared<syrec::BinaryExpression>(
        createExpressionForConstantValue(2), 
        syrec::BinaryExpression::BinaryOperation::Add, 
        std::make_shared<syrec::NumericExpression>(createNumberContainerForLoopVariableIdentifier("$i"), 2));

    auto lhsVariableAccess = syrec::VariableAccess();
    lhsVariableAccess.setVar(accessedVariableInstance);
    lhsVariableAccess.indexes = {createExpressionForConstantValue(0), binaryExpressionDefiningAccessedValueOfFirstDimension};
    lhsVariableAccess.range   = std::make_pair(createNumberContainerForConstantValue(0), createNumberContainerForConstantValue(accessedVariableInstance->bitwidth - 1));

    auto rhsVariableAccess = syrec::VariableAccess();
    rhsVariableAccess.setVar(accessedVariableInstance);
    rhsVariableAccess.indexes = {createExpressionForConstantValue(0), createExpressionForConstantValue(1)};
    rhsVariableAccess.range   = std::make_pair(createNumberContainerForConstantValue(5), lhsVariableAccess.range->first);
    ASSERT_NO_FATAL_FAILURE(assertSymmetricEquivalenceBetweenPotentiallyVariableAccessOverlapOperands(lhsVariableAccess, rhsVariableAccess));
}

TEST(VariableAccessOverlapTests, InvalidValueInAccessedBitrangesOfBothOperandsResultsInPotentialOverlapAndNoCrash) {
    const std::vector<unsigned int> variableDimensions       = {1, 2};
    const auto                      accessedVariableInstance = createVariableInstance(DEFAULT_VARIABLE_IDENTIFIER, variableDimensions, DEFAULT_VARIABLE_BITWIDTH);
    ASSERT_THAT(accessedVariableInstance, testing::NotNull());

    auto lhsVariableAccess = syrec::VariableAccess();
    lhsVariableAccess.setVar(accessedVariableInstance);
    lhsVariableAccess.indexes = {createExpressionForConstantValue(0), createExpressionForConstantValue(1)};
    lhsVariableAccess.range   = std::make_pair(nullptr, createNumberContainerForConstantValue(accessedVariableInstance->bitwidth - 1));

    auto rhsVariableAccess = syrec::VariableAccess();
    rhsVariableAccess.setVar(accessedVariableInstance);
    rhsVariableAccess.indexes = {createExpressionForConstantValue(0), createExpressionForConstantValue(1)};
    rhsVariableAccess.range = std::make_pair(createNumberContainerForConstantValue(0), nullptr);
    ASSERT_NO_FATAL_FAILURE(assertSymmetricEquivalenceBetweenPotentiallyVariableAccessOverlapOperands(lhsVariableAccess, rhsVariableAccess));

    lhsVariableAccess.range = std::make_pair(nullptr, createNumberContainerForConstantValue(accessedVariableInstance->bitwidth - 1));
    rhsVariableAccess.range = std::make_pair(nullptr, createNumberContainerForConstantValue(5));
    ASSERT_NO_FATAL_FAILURE(assertSymmetricEquivalenceBetweenPotentiallyVariableAccessOverlapOperands(lhsVariableAccess, rhsVariableAccess));

    lhsVariableAccess.range = std::make_pair(nullptr, nullptr);
    rhsVariableAccess.range = std::make_pair(nullptr, createNumberContainerForConstantValue(5));
    ASSERT_NO_FATAL_FAILURE(assertSymmetricEquivalenceBetweenPotentiallyVariableAccessOverlapOperands(lhsVariableAccess, rhsVariableAccess));
}

TEST(VariableAccessOverlapTests, NonConstantValueInAccessedBitrangesOfBothOperandsResultsInPotentialOverlapAndNoCrash) {
    const std::vector<unsigned int> variableDimensions       = {1, 2};
    const auto                      accessedVariableInstance = createVariableInstance(DEFAULT_VARIABLE_IDENTIFIER, variableDimensions, DEFAULT_VARIABLE_BITWIDTH);
    ASSERT_THAT(accessedVariableInstance, testing::NotNull());

    const auto containerForLoopVariable = createNumberContainerForLoopVariableIdentifier("$i");
    auto lhsVariableAccess = syrec::VariableAccess();
    lhsVariableAccess.setVar(accessedVariableInstance);
    lhsVariableAccess.indexes = {createExpressionForConstantValue(0), createExpressionForConstantValue(1)};
    lhsVariableAccess.range   = std::make_pair(containerForLoopVariable, createNumberContainerForConstantValue(accessedVariableInstance->bitwidth - 1));

    auto rhsVariableAccess = syrec::VariableAccess();
    rhsVariableAccess.setVar(accessedVariableInstance);
    rhsVariableAccess.indexes = {createExpressionForConstantValue(0), createExpressionForConstantValue(1)};
    rhsVariableAccess.range   = std::make_pair(createNumberContainerForConstantValue(0), containerForLoopVariable);
    ASSERT_NO_FATAL_FAILURE(assertSymmetricEquivalenceBetweenPotentiallyVariableAccessOverlapOperands(lhsVariableAccess, rhsVariableAccess));

    lhsVariableAccess.range = std::make_pair(containerForLoopVariable, createNumberContainerForConstantValue(accessedVariableInstance->bitwidth - 1));
    rhsVariableAccess.range = std::make_pair(containerForLoopVariable, createNumberContainerForConstantValue(5));
    ASSERT_NO_FATAL_FAILURE(assertSymmetricEquivalenceBetweenPotentiallyVariableAccessOverlapOperands(lhsVariableAccess, rhsVariableAccess));

    lhsVariableAccess.range = std::make_pair(containerForLoopVariable, containerForLoopVariable);
    rhsVariableAccess.range = std::make_pair(containerForLoopVariable, createNumberContainerForConstantValue(5));
    ASSERT_NO_FATAL_FAILURE(assertSymmetricEquivalenceBetweenPotentiallyVariableAccessOverlapOperands(lhsVariableAccess, rhsVariableAccess));
}

TEST(VariableAccessOverlapTests, InvalidValueForAccessedValueOfDimensionOfBothOperandsResultsInPotentialOverlapAndNoCrash) {
    const std::vector<unsigned int> variableDimensions       = {1, 2};
    const auto                      accessedVariableInstance = createVariableInstance(DEFAULT_VARIABLE_IDENTIFIER, variableDimensions, DEFAULT_VARIABLE_BITWIDTH);
    ASSERT_THAT(accessedVariableInstance, testing::NotNull());

    const auto binaryExpressionDefiningAccessedValueOfFirstDimensionUsingFirstLoopVariable = std::make_shared<syrec::BinaryExpression>(
            createExpressionForConstantValue(2),
            syrec::BinaryExpression::BinaryOperation::Add,
            std::make_shared<syrec::NumericExpression>(createNumberContainerForLoopVariableIdentifier("$i"), 2));

    auto lhsVariableAccess = syrec::VariableAccess();
    lhsVariableAccess.setVar(accessedVariableInstance);
    lhsVariableAccess.indexes = {nullptr, binaryExpressionDefiningAccessedValueOfFirstDimensionUsingFirstLoopVariable};
    lhsVariableAccess.range   = std::make_pair(createNumberContainerForConstantValue(0), createNumberContainerForConstantValue(accessedVariableInstance->bitwidth - 1));

    auto rhsVariableAccess = syrec::VariableAccess();
    rhsVariableAccess.setVar(accessedVariableInstance);
    rhsVariableAccess.indexes = {createExpressionForConstantValue(0), nullptr};
    rhsVariableAccess.range   = std::make_pair(createNumberContainerForConstantValue(5), lhsVariableAccess.range->first);
    ASSERT_NO_FATAL_FAILURE(assertSymmetricEquivalenceBetweenPotentiallyVariableAccessOverlapOperands(lhsVariableAccess, rhsVariableAccess));

    rhsVariableAccess.indexes[0]         = nullptr;
    ASSERT_NO_FATAL_FAILURE(assertSymmetricEquivalenceBetweenPotentiallyVariableAccessOverlapOperands(lhsVariableAccess, rhsVariableAccess));
}

TEST(VariableAccessOverlapTests, NonConstantValueForAccessedValueOfDimensionOfBothOperandsResultsInPotentialOverlapAndNoCrash) {
    const std::vector<unsigned int> variableDimensions       = {1, 2};
    const auto                      accessedVariableInstance = createVariableInstance(DEFAULT_VARIABLE_IDENTIFIER, variableDimensions, DEFAULT_VARIABLE_BITWIDTH);
    ASSERT_THAT(accessedVariableInstance, testing::NotNull());

    const auto binaryExpressionDefiningAccessedValueOfFirstDimensionUsingFirstLoopVariable = std::make_shared<syrec::BinaryExpression>(
            createExpressionForConstantValue(2),
            syrec::BinaryExpression::BinaryOperation::Add,
            std::make_shared<syrec::NumericExpression>(createNumberContainerForLoopVariableIdentifier("$i"), 2));

    auto lhsVariableAccess = syrec::VariableAccess();
    lhsVariableAccess.setVar(accessedVariableInstance);
    lhsVariableAccess.indexes = {binaryExpressionDefiningAccessedValueOfFirstDimensionUsingFirstLoopVariable, binaryExpressionDefiningAccessedValueOfFirstDimensionUsingFirstLoopVariable};
    lhsVariableAccess.range   = std::make_pair(createNumberContainerForConstantValue(0), createNumberContainerForConstantValue(accessedVariableInstance->bitwidth - 1));

    auto rhsVariableAccess = syrec::VariableAccess();
    rhsVariableAccess.setVar(accessedVariableInstance);
    rhsVariableAccess.indexes = {createExpressionForConstantValue(0), binaryExpressionDefiningAccessedValueOfFirstDimensionUsingFirstLoopVariable};
    rhsVariableAccess.range   = std::make_pair(createNumberContainerForConstantValue(5), lhsVariableAccess.range->first);
    ASSERT_NO_FATAL_FAILURE(assertSymmetricEquivalenceBetweenPotentiallyVariableAccessOverlapOperands(lhsVariableAccess, rhsVariableAccess));

    const auto exprForSecondLoopVariable = std::make_shared<syrec::NumericExpression>(createNumberContainerForLoopVariableIdentifier("$j"), 2);
    rhsVariableAccess.indexes[0]         = exprForSecondLoopVariable;
    ASSERT_NO_FATAL_FAILURE(assertSymmetricEquivalenceBetweenPotentiallyVariableAccessOverlapOperands(lhsVariableAccess, rhsVariableAccess));
}