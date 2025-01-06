#include "core/syrec/expression.hpp"
#include <core/syrec/parser/utils/variable_overlap_check.hpp>

#include <gtest/gtest.h>

#include <vector>

namespace {
    const std::string      DEFAULT_VARIABLE_IDENTIFIER = "varIdent";
    constexpr unsigned int DEFAULT_VARIABLE_BITWIDTH = 16;

    syrec::Number::ptr createNumberContainerForConstantValue(unsigned int value) {
        return std::make_shared<syrec::Number>(value);
    }

    syrec::Expression::ptr createExpressionForConstantValue(unsigned int value) {
        return std::make_shared<syrec::NumericExpression>(createNumberContainerForConstantValue(value), 2);
    }

    syrec::Variable::ptr createVariableInstance(const std::string& variableIdentifier, const std::vector<unsigned int>& variableDimensions, unsigned int variableBitwidth) {
        return std::make_shared<syrec::Variable>(syrec::Variable::Type::In, variableIdentifier, variableDimensions, variableBitwidth);
    }

    struct VariableDefinition {
        std::string               identifier;
        std::vector<unsigned int> dimensions;
        unsigned int              bitwidth;

        VariableDefinition(const std::string& variableIdentifier, std::vector<unsigned int>&& variableDimensions, unsigned int bitwidth):
            identifier(variableIdentifier), dimensions(std::move(variableDimensions)), bitwidth(bitwidth) {}
    };

    struct VariableAccessDefinition {
        std::vector<unsigned int>                            accessedValuePerDimension;
        std::optional<std::pair<unsigned int, unsigned int>> accessedBitRange;

        explicit VariableAccessDefinition():
            accessedValuePerDimension({}), accessedBitRange(std::nullopt) {}

        explicit VariableAccessDefinition(unsigned int accessedBit):
            accessedValuePerDimension({}), accessedBitRange(std::make_pair(accessedBit, accessedBit)) {}

        explicit VariableAccessDefinition(std::pair<unsigned int, unsigned int> accessedBitRange):
            accessedValuePerDimension({}), accessedBitRange(accessedBitRange) {}

        explicit VariableAccessDefinition(const std::vector<unsigned int>& accessedValuePerDimension)
            : accessedValuePerDimension(accessedValuePerDimension), accessedBitRange(std::nullopt) {}

        explicit VariableAccessDefinition(const std::vector<unsigned int>& accessedValuePerDimension, unsigned int accessedBit)
            : accessedValuePerDimension(accessedValuePerDimension), accessedBitRange(std::make_pair(accessedBit, accessedBit)) {}

        explicit VariableAccessDefinition(const std::vector<unsigned int>& accessedValuePerDimension, std::pair<unsigned int, unsigned int> accessedBitRange)
            : accessedValuePerDimension(accessedValuePerDimension), accessedBitRange(accessedBitRange) {}
    };

    struct OverlapTestData {
        std::string              testName;
        VariableDefinition       variableDefinition;
        VariableAccessDefinition lVarAccessDefinition;
        VariableAccessDefinition rVarAccessDefinition;

        explicit OverlapTestData(std::string&& testName, VariableDefinition&& variableDefinition, VariableAccessDefinition&& lVarAccessDefinition, VariableAccessDefinition&& rVarAccessDefinition):
            testName(std::move(testName)), variableDefinition(std::move(variableDefinition)), lVarAccessDefinition(std::move(lVarAccessDefinition)), rVarAccessDefinition(std::move(rVarAccessDefinition)) {}
    };

    class BaseOverlapInVariableAccessesTestFixture : public testing::TestWithParam<OverlapTestData> {
    public:
        virtual utils::VariableAccessOverlapCheckResult getExpectedVariableAccessOverlapCheckResult() = 0;

        void performTestExecution() {
            const auto& [_, variableDefinition, lVarAccessDefinition, rVarAccessDefinition] = GetParam();
            const auto variableInstance                                                  = std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, variableDefinition.identifier, variableDefinition.dimensions, variableDefinition.bitwidth);

            const syrec::VariableAccess lVarAccessInstance = createVariableAccessFromDefinition(variableInstance, lVarAccessDefinition);
            const syrec::VariableAccess rVarAccessInstance = createVariableAccessFromDefinition(variableInstance, rVarAccessDefinition);

            std::optional<utils::VariableAccessOverlapCheckResult> actualVariableAccessOverlapCheckResult;
            ASSERT_NO_FATAL_FAILURE(actualVariableAccessOverlapCheckResult = utils::checkOverlapBetweenVariableAccesses(lVarAccessInstance, rVarAccessInstance));
            ASSERT_TRUE(actualVariableAccessOverlapCheckResult.has_value());
            ASSERT_EQ(getExpectedVariableAccessOverlapCheckResult(), actualVariableAccessOverlapCheckResult.value());

            // Operation must be symmetric (a OP b) = (b OP a)
            ASSERT_NO_FATAL_FAILURE(actualVariableAccessOverlapCheckResult = utils::checkOverlapBetweenVariableAccesses(rVarAccessInstance, lVarAccessInstance));
            ASSERT_TRUE(actualVariableAccessOverlapCheckResult.has_value());
            ASSERT_EQ(getExpectedVariableAccessOverlapCheckResult(), actualVariableAccessOverlapCheckResult.value());
        }

    protected:
        [[nodiscard]] static syrec::VariableAccess createVariableAccessFromDefinition(const syrec::Variable::ptr& referenceVar, const VariableAccessDefinition& variableAccessDefinition) {
            syrec::VariableAccess variableAccess;
            variableAccess.setVar(referenceVar);

            for (const auto& accessedValuePerDimension: variableAccessDefinition.accessedValuePerDimension)
                variableAccess.indexes.emplace_back(std::make_shared<syrec::NumericExpression>(std::make_shared<syrec::Number>(accessedValuePerDimension), 1));

            if (variableAccessDefinition.accessedBitRange.has_value()) {
                const auto accessedBitrangeStartContainer = std::make_shared<syrec::Number>(variableAccessDefinition.accessedBitRange->first);
                const auto accessedBitrangeEndContainer = std::make_shared<syrec::Number>(variableAccessDefinition.accessedBitRange->second);
                variableAccess.range                      = std::make_pair(accessedBitrangeStartContainer, accessedBitrangeEndContainer);
            }
            return variableAccess;
        }
    };

    class ExpectingOverlappingVariableAccessesTestFixture: public BaseOverlapInVariableAccessesTestFixture {
        utils::VariableAccessOverlapCheckResult getExpectedVariableAccessOverlapCheckResult() override {
            return utils::VariableAccessOverlapCheckResult::Overlapping;
        }
    };

    class ExpectingPotentiallyOverlappingVariableAccessesTestFixture: public BaseOverlapInVariableAccessesTestFixture {
        utils::VariableAccessOverlapCheckResult getExpectedVariableAccessOverlapCheckResult() override {
            return utils::VariableAccessOverlapCheckResult::MaybeOverlapping;
        }
    };

    class ExpectingNotOverlappingVariableAccessesTestFixture: public BaseOverlapInVariableAccessesTestFixture {
        utils::VariableAccessOverlapCheckResult getExpectedVariableAccessOverlapCheckResult() override {
            return utils::VariableAccessOverlapCheckResult::NotOverlapping;
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
            VariableAccessDefinition()),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessNoBitRangeAccess_rAccess_NoDimensionAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(),
            VariableAccessDefinition( 2)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessNoBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(),
            VariableAccessDefinition( std::make_pair(2, 14))),

        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitAccess_rAccess_NoDimensionAccessNoBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(2),
            VariableAccessDefinition()),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitAccess_rAccess_NoDimensionAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(2),
            VariableAccessDefinition( 2)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitAccess_rAccess_NoDimensionAccessBitRangeAccessStartEqualToBit",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(2),
            VariableAccessDefinition( std::make_pair(2, 14))),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitAccess_rAccess_NoDimensionAccessBitRangeAccessEndEqualToBit",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(14),
            VariableAccessDefinition(std::make_pair(2, 14))),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitAccess_rAccess_NoDimensionAccessBitRangeAccessOverlappingBit",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(5),
            VariableAccessDefinition(std::make_pair(2, 14))),

        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessNoBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(2, 14)),
            VariableAccessDefinition()),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(2, 14)),
            VariableAccessDefinition(5)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccessStartEqualToBit_rAccess_NoDimensionAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(2, 14)),
            VariableAccessDefinition(2)),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccessEndEqualToBit_rAccess_NoDimensionAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(2, 14)),
            VariableAccessDefinition(14)),

        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessOverlappingFromTheLeft",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(2, 14)),
            VariableAccessDefinition(std::make_pair(1, 3))),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessOverlappingFromTheRight",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(2, 14)),
            VariableAccessDefinition(std::make_pair(1, 14))),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessBeingEqualToLeftOne",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(2, 14)),
            VariableAccessDefinition(std::make_pair(2, 14))),
        OverlapTestData("1DSignal_lAccess_NoDimensionAccessBitRangeAccess_rAccess_NoDimensionAccessBitRangeAccessInLeftOne",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({1u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::make_pair(2, 14)),
            VariableAccessDefinition(std::make_pair(5, 8))),

        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessNoBitRangeAccess_rAccess_DimensionAccessNoBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u})),
            VariableAccessDefinition(std::vector({1u}))),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessNoBitRangeAccess_rAccess_DimensionAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u})),
            VariableAccessDefinition(std::vector({1u}), 2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessNoBitRangeAccess_rAccess_DimensionAccessBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u})),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, 14))),

        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessBitAccess_rAccess_DimensionAccessNoBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}) ,2),
            VariableAccessDefinition(std::vector({1u}))),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessBitAccess_rAccess_DimensionAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}) ,2),
            VariableAccessDefinition(std::vector({1u}), 2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessBitAccess_rAccess_DimensionAccessBitRangeAccessOverlappingFromTheLeft",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}) ,2),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, 14))),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessBitAccess_rAccess_DimensionAccessBitRangeAccessOverlappingFromTheRight",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), 2),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(0, 3))),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessBitAccess_rAccess_DimensionAccessBitRangeAccessEqualToBit",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), 2),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, 2))),

        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessBitRangeAccess_rAccess_DimensionAccessNoBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}) ,std::make_pair(2, 14)),
            VariableAccessDefinition(std::vector({1u}))),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessBitRangeAccess_rAccess_DimensionAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}) ,std::make_pair(2, 14)),
            VariableAccessDefinition(std::vector({1u}) ,2)),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessBitRangeAccess_rAccess_DimensionAccessBitRangeAccessOverlappingFromTheLeft",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}) ,std::make_pair(2, 14)),
            VariableAccessDefinition(std::vector({1u}) ,std::make_pair(0, 3))),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessBitRangeAccess_rAccess_DimensionAccessBitRangeAccessOverlappingFromTheRight",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, 14)),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(1, 14))),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessBitRangeAccess_rAccess_DimensionAccessBitRangeAccessEqualToLeftAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, 14)),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, 14))),
        OverlapTestData("1DSignalMultipleValues_lAccess_DimensionAccessBitRangeAccess_rAccess_DimensionAccessBitRangeAccessInLeftOne",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(2, 14)),
            VariableAccessDefinition(std::vector({1u}), std::make_pair(5, 8))),


        OverlapTestData("NDSignal_lAccess_NoBitRangeAccess_rAccessNoBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u,1u})),
            VariableAccessDefinition(std::vector({0u,1u}))),
        OverlapTestData("NDSignal_lAccess_NoBitRangeAccess_rAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u})),
            VariableAccessDefinition(std::vector({0u, 1u}), 2)),
        OverlapTestData("NDSignal_lAccess_NoBitRangeAccess_rAccessBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u})),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, 5))),

        OverlapTestData("NDSignal_lAccess_BitAccess_rAccessNoBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), 2),
            VariableAccessDefinition(std::vector({0u, 1u}))),
        OverlapTestData("NDSignal_lAccess_BitAccess_rAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), 2),
            VariableAccessDefinition(std::vector({0u, 1u}), 2)),
        OverlapTestData("NDSignal_lAccess_BitAccess_rAccessBitRangeAccessOverlappingBitFromTheLeft",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), 2),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(0, 8))),
        OverlapTestData("NDSignal_lAccess_BitAccess_rAccessBitRangeAccessWithStartEqualToBit",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), 2),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, 8))),
        OverlapTestData("NDSignal_lAccess_BitAccess_rAccessBitRangeAccessWithEndEqualToBit",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), 2),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(0, 2))),
        OverlapTestData("NDSignal_lAccess_BitAccess_rAccessBitRangeAccessEnclosingBitOfLAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), 2),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(0, 8))),
        OverlapTestData("NDSignal_lAccess_BitAccess_rAccessBitRangeAccessOverlappingBitFromTheRight",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), 2),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(8, 0))),

        OverlapTestData("NDSignal_lAccess_BitRangeAccesss_rAccessNoBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, 5)),
            VariableAccessDefinition(std::vector({0u, 1u}))),
        OverlapTestData("NDSignal_lAccess_BitRangeAccessStartEqualToBit_rAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, 5)),
            VariableAccessDefinition(std::vector({0u, 1u}), 2)),
        OverlapTestData("NDSignal_lAccess_BitRangeAccessEndEqualToBit_rAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, 5)),
            VariableAccessDefinition(std::vector({0u, 1u}), 5)),
        OverlapTestData("NDSignal_lAccess_BitRangeAccessEnclosingBit_rAccessBitAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, 5)),
            VariableAccessDefinition(std::vector({0u, 1u}), 4)),
        OverlapTestData("NDSignal_lAccess_BitRangeAccessOverlappingOtherFromTheLeft_rAccessBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(0, 4)),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, 5))),
        OverlapTestData("NDSignal_lAccess_BitRangeAccessOverlappingOtherFromTheRight_rAccessBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(8, 0)),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, 5))),
        OverlapTestData("NDSignal_lAccess_BitRangeAccessEqualToOther_rAccessBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, 5)),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, 5))),
        OverlapTestData("NDSignal_lAccess_BitRangeAccessEnclosingOther_rAccessBitRangeAccess",
            VariableDefinition(DEFAULT_VARIABLE_IDENTIFIER, std::vector({2u, 3u}), DEFAULT_VARIABLE_BITWIDTH),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(0, 8)),
            VariableAccessDefinition(std::vector({0u, 1u}), std::make_pair(2, 5)))
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