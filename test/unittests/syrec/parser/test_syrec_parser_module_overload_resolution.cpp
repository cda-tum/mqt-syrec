#include "core/syrec/program.hpp"

#include "gmock/gmock-matchers.h"
#include <gtest/gtest.h>
#include <memory>

class ParserModuleOverloadResolutionTestsFixture : public testing::Test {
public:
    using CallStmtVariant = std::variant<std::shared_ptr<syrec::CallStatement>, std::shared_ptr<syrec::UncallStatement>>;

    static void performTestExecution(const std::string& stringifiedSyrecProgram, 
        const syrec::Module& signatureOfModuleInWhichCallStatementShallBeInspected, 
        const syrec::Module& signatureOfModuleBeingTargetOfCall, 
        const std::vector<std::string>& expectedVariableIdentifiersOfParametersForCallStatement) {
        syrec::Program parserInstance;
        assertStringifiedSyrecProgramProcessingCompletesWithoutErrors(parserInstance, stringifiedSyrecProgram);

        const std::optional<syrec::Module::ptr> moduleOfInterest = findFirstModuleMatchingSignature(parserInstance.modules(), signatureOfModuleInWhichCallStatementShallBeInspected);
        // We could use an assert here but since the future access on the optional is then considered as unchecked, the if guard is used instead.
        if (!moduleOfInterest.has_value())
            FAIL() << "Expected to be able to locate the module of interest";

        const std::optional<syrec::Module::ptr> moduleExpectedToBeingTargetOfCall = findFirstModuleMatchingSignature(parserInstance.modules(), signatureOfModuleBeingTargetOfCall);
        ASSERT_TRUE(moduleExpectedToBeingTargetOfCall.has_value()) << "Expected to be able to locate the module which should be the target of the to be located call/uncall statement";

        const std::optional<CallStmtVariant>& actualCallStatementVariant = findFirstCallStatementInListOfStatements(moduleOfInterest->get()->statements);
        ASSERT_TRUE(actualCallStatementVariant.has_value()) << "Expected to be able to locate a single call/uncall statement in the generated IR of the SyReC program";

        const auto expectedCallStatement   = std::get_if<std::shared_ptr<syrec::CallStatement>>(&*actualCallStatementVariant);
        const auto expectedUncallStatement = std::get_if<std::shared_ptr<syrec::UncallStatement>>(&*actualCallStatementVariant);

        syrec::Module::ptr   actualModuleBeingTargetOfCall;
        std::vector<std::string> actualVariableIdentifiersOfParametersForCallStatement;
        if (expectedCallStatement) {
            ASSERT_THAT(expectedCallStatement->get()->target, testing::NotNull());
            actualModuleBeingTargetOfCall    = expectedCallStatement->get()->target;
            actualVariableIdentifiersOfParametersForCallStatement = expectedCallStatement->get()->parameters;
        } else {
            ASSERT_THAT(expectedUncallStatement->get()->target, testing::NotNull());
            actualModuleBeingTargetOfCall    = expectedUncallStatement->get()->target;
            actualVariableIdentifiersOfParametersForCallStatement = expectedUncallStatement->get()->parameters;
        }

        ASSERT_EQ(*moduleExpectedToBeingTargetOfCall, actualModuleBeingTargetOfCall);
        ASSERT_TRUE(doModuleSignaturesMatch(signatureOfModuleBeingTargetOfCall, *actualModuleBeingTargetOfCall));
        ASSERT_THAT(actualVariableIdentifiersOfParametersForCallStatement, testing::ElementsAreArray(expectedVariableIdentifiersOfParametersForCallStatement));
    }

protected:
    static void assertStringifiedSyrecProgramProcessingCompletesWithoutErrors(syrec::Program& program, const std::string& stringifiedSyrecProgram) {
        const auto determinedParsingErrors = program.readFromString(stringifiedSyrecProgram);
        ASSERT_TRUE(determinedParsingErrors.empty()) << "Expected to be able to process the user provided syrec program without errors";
    }

    [[nodiscard]] static std::optional<CallStmtVariant> findFirstCallStatementInListOfStatements(const std::vector<syrec::Statement::ptr>& statements) {
        std::optional<CallStmtVariant> foundCallStmtInstance;

        for (std::size_t i = 0; i < statements.size() && !foundCallStmtInstance.has_value(); ++i){
            if (!statements.at(i))
                continue;

            const syrec::Statement* stmtReference = &*statements.at(i);
            if (const auto& stmtAsForStatement = dynamic_cast<const syrec::ForStatement*>(stmtReference); stmtAsForStatement) {
                foundCallStmtInstance = findFirstCallStatementInListOfStatements(stmtAsForStatement->statements);
            } else if (const auto& stmtAsIfStatement = dynamic_cast<const syrec::IfStatement*>(stmtReference); stmtAsIfStatement) {
                foundCallStmtInstance = findFirstCallStatementInListOfStatements(stmtAsIfStatement->thenStatements);
                if (!foundCallStmtInstance.has_value())
                    foundCallStmtInstance = findFirstCallStatementInListOfStatements(stmtAsIfStatement->elseStatements);
                // TODO: Distinction whether only call or uncall statements should be considered as potential matches
            } else if (const auto& stmtAsCallStatement = dynamic_cast<const syrec::CallStatement*>(stmtReference); stmtAsCallStatement) {
                foundCallStmtInstance = std::make_shared<syrec::CallStatement>(*stmtAsCallStatement);
            } else if (const auto& stmtAsUncallStatement = dynamic_cast<const syrec::UncallStatement*>(stmtReference); stmtAsUncallStatement) {
                foundCallStmtInstance = std::make_shared<syrec::UncallStatement>(*stmtAsUncallStatement);
            }
        }
        return foundCallStmtInstance;
    }

    [[nodiscard]] static std::optional<syrec::Module::ptr> findFirstModuleMatchingSignature(const syrec::Module::vec& modules, const syrec::Module& moduleSignatureToSearchFor) {
        const auto& moduleMatchingSignature = std::find_if(
                modules.cbegin(),
                modules.cend(),
                [&moduleSignatureToSearchFor](const syrec::Module::ptr& module) {
                    return doModuleSignaturesMatch(moduleSignatureToSearchFor, *module);
                });
        return moduleMatchingSignature != modules.cend() ? std::make_optional(*moduleMatchingSignature) : std::nullopt;
    }

    [[nodiscard]] static bool doModuleSignaturesMatch(const syrec::Module& expectedModuleSignatureData, const syrec::Module& actualModuleSignatureData) {
        return actualModuleSignatureData.name == expectedModuleSignatureData.name
            && actualModuleSignatureData.parameters.size() == expectedModuleSignatureData.parameters.size()
            && (!expectedModuleSignatureData.parameters.empty()
                ? std::find_first_of(
                actualModuleSignatureData.parameters.cbegin(), actualModuleSignatureData.parameters.cend(),
                expectedModuleSignatureData.parameters.cbegin(), expectedModuleSignatureData.parameters.cend(),
                [](const syrec::Variable::ptr& actualParameter, const syrec::Variable::ptr& expectedParameter) {
                    return actualParameter->name == expectedParameter->name
                            && actualParameter->dimensions.size() == expectedParameter->dimensions.size()
                            && (!expectedParameter->dimensions.empty()
                                ? std::find_first_of(
                                actualParameter->dimensions.cbegin(), actualParameter->dimensions.cend(),
                                expectedParameter->dimensions.cbegin(), expectedParameter->dimensions.cend(), 
                                [](const unsigned int actualNumValuesOfDimension, const unsigned int expectedNumValuesOfDimension) {
                                        return actualNumValuesOfDimension == expectedNumValuesOfDimension;
                                }) != actualParameter->dimensions.cend() : true)
                            && actualParameter->bitwidth == expectedParameter->bitwidth;
            }) != actualModuleSignatureData.parameters.cend() : true);
    }
};

TEST_F(ParserModuleOverloadResolutionTestsFixture, CallOfModuleWithParameterOfTypeInWithCallerArgumentOfTypeIn) {
    const std::string& stringifiedSyrecProgramToProcess = "module print(in a(4)) skip module main(in b(4)) call print(b)";
    syrec::Module      signatureOfModuleContainingCallStmt("main");
    signatureOfModuleContainingCallStmt.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::In, "b", std::vector({1u}), 4));

    syrec::Module      signatureOfModuleBeingTargetOfCall("print");
    signatureOfModuleBeingTargetOfCall.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::In, "a", std::vector({1u}), 4));

    std::vector<std::string> variableIdentifiersOfParametersForCallStmt;
    variableIdentifiersOfParametersForCallStmt.emplace_back(signatureOfModuleContainingCallStmt.parameters.front()->name);

    performTestExecution(stringifiedSyrecProgramToProcess, signatureOfModuleContainingCallStmt, signatureOfModuleBeingTargetOfCall, variableIdentifiersOfParametersForCallStmt);
}

TEST_F(ParserModuleOverloadResolutionTestsFixture, CallOfModuleWithParameterOfTypeInWithCallerArgumentOfTypeInout) {
    const std::string& stringifiedSyrecProgramToProcess = "module print(in a(4)) skip module main(inout b(4)) call print(b)";
    syrec::Module      signatureOfModuleContainingCallStmt("main");
    signatureOfModuleContainingCallStmt.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::In, "b", std::vector({1u}), 4));

    syrec::Module signatureOfModuleBeingTargetOfCall("print");
    signatureOfModuleBeingTargetOfCall.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::In, "a", std::vector({1u}), 4));

    std::vector<std::string> variableIdentifiersOfParametersForCallStmt;
    variableIdentifiersOfParametersForCallStmt.emplace_back(signatureOfModuleContainingCallStmt.parameters.front()->name);

    performTestExecution(stringifiedSyrecProgramToProcess, signatureOfModuleContainingCallStmt, signatureOfModuleBeingTargetOfCall, variableIdentifiersOfParametersForCallStmt);
}

TEST_F(ParserModuleOverloadResolutionTestsFixture, CallOfModuleWithParameterOfTypeInWithCallerArgumentOfTypeOut) {
    const std::string& stringifiedSyrecProgramToProcess = "module print(in a(4)) skip module main(out b(4)) call print(b)";
    syrec::Module      signatureOfModuleContainingCallStmt("main");
    signatureOfModuleContainingCallStmt.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Out, "b", std::vector({1u}), 4));

    syrec::Module signatureOfModuleBeingTargetOfCall("print");
    signatureOfModuleBeingTargetOfCall.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::In, "a", std::vector({1u}), 4));

    std::vector<std::string> variableIdentifiersOfParametersForCallStmt;
    variableIdentifiersOfParametersForCallStmt.emplace_back(signatureOfModuleContainingCallStmt.parameters.front()->name);

    performTestExecution(stringifiedSyrecProgramToProcess, signatureOfModuleContainingCallStmt, signatureOfModuleBeingTargetOfCall, variableIdentifiersOfParametersForCallStmt);
}

TEST_F(ParserModuleOverloadResolutionTestsFixture, CallOfModuleWithParameterOfTypeInWithCallerArgumentOfTypeWire) {
    const std::string& stringifiedSyrecProgramToProcess = "module print(in a(4)) skip module main() wire b(4) call print(b)";
    syrec::Module      signatureOfModuleContainingCallStmt("main");

    syrec::Module signatureOfModuleBeingTargetOfCall("print");
    signatureOfModuleBeingTargetOfCall.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::In, "a", std::vector({1u}), 4));

    std::vector<std::string> variableIdentifiersOfParametersForCallStmt;
    variableIdentifiersOfParametersForCallStmt.emplace_back("b");

    performTestExecution(stringifiedSyrecProgramToProcess, signatureOfModuleContainingCallStmt, signatureOfModuleBeingTargetOfCall, variableIdentifiersOfParametersForCallStmt);
}

TEST_F(ParserModuleOverloadResolutionTestsFixture, CallOfModuleWithParameterOfTypeInoutWithCallerArgumentOfTypeInout) {
    const std::string& stringifiedSyrecProgramToProcess = "module incr(inout a(4)) ++= a module main(inout b(4)) call incr(b)";
    syrec::Module      signatureOfModuleContainingCallStmt("main");
    signatureOfModuleContainingCallStmt.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "b", std::vector({1u}), 4));

    syrec::Module signatureOfModuleBeingTargetOfCall("incr");
    signatureOfModuleBeingTargetOfCall.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "a", std::vector({1u}), 4));

    std::vector<std::string> variableIdentifiersOfParametersForCallStmt;
    variableIdentifiersOfParametersForCallStmt.emplace_back(signatureOfModuleContainingCallStmt.parameters.front()->name);

    performTestExecution(stringifiedSyrecProgramToProcess, signatureOfModuleContainingCallStmt, signatureOfModuleBeingTargetOfCall, variableIdentifiersOfParametersForCallStmt);
}

TEST_F(ParserModuleOverloadResolutionTestsFixture, CallOfModuleWithParameterOfTypeInoutWithCallerArgumentOfTypeOut) {
    const std::string& stringifiedSyrecProgramToProcess = "module incr(inout a(4)) ++= a module main(inout b(4)) call incr(b)";
    syrec::Module      signatureOfModuleContainingCallStmt("main");
    signatureOfModuleContainingCallStmt.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Out, "b", std::vector({1u}), 4));

    syrec::Module signatureOfModuleBeingTargetOfCall("incr");
    signatureOfModuleBeingTargetOfCall.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "a", std::vector({1u}), 4));

    std::vector<std::string> variableIdentifiersOfParametersForCallStmt;
    variableIdentifiersOfParametersForCallStmt.emplace_back(signatureOfModuleContainingCallStmt.parameters.front()->name);

    performTestExecution(stringifiedSyrecProgramToProcess, signatureOfModuleContainingCallStmt, signatureOfModuleBeingTargetOfCall, variableIdentifiersOfParametersForCallStmt);
}

TEST_F(ParserModuleOverloadResolutionTestsFixture, CallOfModuleWithParameterOfTypeInoutWithCallerArgumentOfTypeWire) {
    const std::string& stringifiedSyrecProgramToProcess = "module incr(inout a(4)) ++= a module main() wire b(4) call incr(b)";
    syrec::Module      signatureOfModuleContainingCallStmt("main");

    syrec::Module signatureOfModuleBeingTargetOfCall("incr");
    signatureOfModuleBeingTargetOfCall.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "a", std::vector({1u}), 4));

    std::vector<std::string> variableIdentifiersOfParametersForCallStmt;
    variableIdentifiersOfParametersForCallStmt.emplace_back("b");

    performTestExecution(stringifiedSyrecProgramToProcess, signatureOfModuleContainingCallStmt, signatureOfModuleBeingTargetOfCall, variableIdentifiersOfParametersForCallStmt);
}

TEST_F(ParserModuleOverloadResolutionTestsFixture, CallOfModuleWithParameterOfTypeOutWithCallerArgumentOfTypeInout) {
    const std::string& stringifiedSyrecProgramToProcess = "module incr(out a(4)) ++= a module main(inout b(4)) call incr(b)";
    syrec::Module      signatureOfModuleContainingCallStmt("main");
    signatureOfModuleContainingCallStmt.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "b", std::vector({1u}), 4));

    syrec::Module signatureOfModuleBeingTargetOfCall("incr");
    signatureOfModuleBeingTargetOfCall.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Out, "a", std::vector({1u}), 4));

    std::vector<std::string> variableIdentifiersOfParametersForCallStmt;
    variableIdentifiersOfParametersForCallStmt.emplace_back(signatureOfModuleContainingCallStmt.parameters.front()->name);

    performTestExecution(stringifiedSyrecProgramToProcess, signatureOfModuleContainingCallStmt, signatureOfModuleBeingTargetOfCall, variableIdentifiersOfParametersForCallStmt);
}

TEST_F(ParserModuleOverloadResolutionTestsFixture, CallOfModuleWithParameterOfTypeOutWithCallerArgumentOfTypeOut) {
    const std::string& stringifiedSyrecProgramToProcess = "module incr(out a(4)) ++= a module main(out b(4)) call incr(b)";
    syrec::Module      signatureOfModuleContainingCallStmt("main");
    signatureOfModuleContainingCallStmt.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Out, "b", std::vector({1u}), 4));

    syrec::Module signatureOfModuleBeingTargetOfCall("incr");
    signatureOfModuleBeingTargetOfCall.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Out, "a", std::vector({1u}), 4));

    std::vector<std::string> variableIdentifiersOfParametersForCallStmt;
    variableIdentifiersOfParametersForCallStmt.emplace_back(signatureOfModuleContainingCallStmt.parameters.front()->name);

    performTestExecution(stringifiedSyrecProgramToProcess, signatureOfModuleContainingCallStmt, signatureOfModuleBeingTargetOfCall, variableIdentifiersOfParametersForCallStmt);
}

TEST_F(ParserModuleOverloadResolutionTestsFixture, CallOfModuleWithOverloadResolutionUsingVariableTypeToSelectCorrectModule) {
    const std::string& stringifiedSyrecProgramToProcess = "module incr(in a(4)) skip module incr(inout a(8)) ++= a module main(out b(8)) call incr(b)";

    syrec::Module signatureOfModuleContainingCallStmt("main");
    signatureOfModuleContainingCallStmt.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Out, "b", std::vector({1u}), 8));

    syrec::Module signatureOfModuleBeingTargetOfCall("incr");
    signatureOfModuleBeingTargetOfCall.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "a", std::vector({1u}), 8));

    std::vector<std::string> variableIdentifiersOfParametersForCallStmt;
    variableIdentifiersOfParametersForCallStmt.emplace_back(signatureOfModuleContainingCallStmt.parameters.front()->name);

    performTestExecution(stringifiedSyrecProgramToProcess, signatureOfModuleContainingCallStmt, signatureOfModuleBeingTargetOfCall, variableIdentifiersOfParametersForCallStmt);
}

TEST_F(ParserModuleOverloadResolutionTestsFixture, CallOfModuleWithOverloadResolutionUsingBitwidthToSelectCorrectModule) {
    const std::string& stringifiedSyrecProgramToProcess = "module incr(inout a(4)) ++= a module incr(inout a(8)) ++= a module main(out b(8)) call incr(b)";

    syrec::Module signatureOfModuleContainingCallStmt("main");
    signatureOfModuleContainingCallStmt.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Out, "b", std::vector({1u}), 8));

    syrec::Module signatureOfModuleBeingTargetOfCall("incr");
    signatureOfModuleBeingTargetOfCall.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "a", std::vector({1u}), 8));

    std::vector<std::string> variableIdentifiersOfParametersForCallStmt;
    variableIdentifiersOfParametersForCallStmt.emplace_back(signatureOfModuleContainingCallStmt.parameters.front()->name);

    performTestExecution(stringifiedSyrecProgramToProcess, signatureOfModuleContainingCallStmt, signatureOfModuleBeingTargetOfCall, variableIdentifiersOfParametersForCallStmt);
}

TEST_F(ParserModuleOverloadResolutionTestsFixture, CallOfModuleWithOverloadResolutionUsingNumberOfValuesOfDimensionToSelectCorrectModuleWithOtherOverloadHavingLessValuesOfDimensions) {
    const std::string& stringifiedSyrecProgramToProcess = "module incr(inout a(4)) ++= a module incr(inout a[2](4)) ++= a[0]; ++= a[1] module main(inout b[2](4)) call incr(b)";

    syrec::Module signatureOfModuleContainingCallStmt("main");
    signatureOfModuleContainingCallStmt.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "b", std::vector({2u}), 4));

    syrec::Module signatureOfModuleBeingTargetOfCall("incr");
    signatureOfModuleBeingTargetOfCall.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "a", std::vector({2u}), 4));

    std::vector<std::string> variableIdentifiersOfParametersForCallStmt;
    variableIdentifiersOfParametersForCallStmt.emplace_back(signatureOfModuleContainingCallStmt.parameters.front()->name);

    performTestExecution(stringifiedSyrecProgramToProcess, signatureOfModuleContainingCallStmt, signatureOfModuleBeingTargetOfCall, variableIdentifiersOfParametersForCallStmt);
}

TEST_F(ParserModuleOverloadResolutionTestsFixture, CallOfModuleWithOverloadResolutionUsingNumberOfValuesOfDimensionToSelectCorrectModuleWithOtherOverloadHavingMoreValuesOfDimensions) {
    const std::string& stringifiedSyrecProgramToProcess = "module incr(inout a[2](4)) ++= a[0]; ++= a[1] module incr(inout a[4](4)) ++= a[0]; ++= a[1] module main(inout b[2](4)) call incr(b)";

    syrec::Module signatureOfModuleContainingCallStmt("main");
    signatureOfModuleContainingCallStmt.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "b", std::vector({2u}), 4));

    syrec::Module signatureOfModuleBeingTargetOfCall("incr");
    signatureOfModuleBeingTargetOfCall.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "a", std::vector({2u}), 4));

    std::vector<std::string> variableIdentifiersOfParametersForCallStmt;
    variableIdentifiersOfParametersForCallStmt.emplace_back(signatureOfModuleContainingCallStmt.parameters.front()->name);

    performTestExecution(stringifiedSyrecProgramToProcess, signatureOfModuleContainingCallStmt, signatureOfModuleBeingTargetOfCall, variableIdentifiersOfParametersForCallStmt);
}

TEST_F(ParserModuleOverloadResolutionTestsFixture, CallOfModuleWithOverloadResolutionUsingNumberOfDimensionsToSelectCorrectModuleWithOtherOverloadHavingLessDimensions) {
    const std::string& stringifiedSyrecProgramToProcess = "module incr(inout a[2](4)) ++= a[0]; ++= a[1] module incr(inout a[2][3](4)) ++= a[0][0]; ++= a[1][0] module main(inout b[2][3](4)) call incr(b)";

    syrec::Module signatureOfModuleContainingCallStmt("main");
    signatureOfModuleContainingCallStmt.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "b", std::vector({2u, 3u}), 4));

    syrec::Module signatureOfModuleBeingTargetOfCall("incr");
    signatureOfModuleBeingTargetOfCall.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "a", std::vector({2u, 3u}), 4));

    std::vector<std::string> variableIdentifiersOfParametersForCallStmt;
    variableIdentifiersOfParametersForCallStmt.emplace_back(signatureOfModuleContainingCallStmt.parameters.front()->name);

    performTestExecution(stringifiedSyrecProgramToProcess, signatureOfModuleContainingCallStmt, signatureOfModuleBeingTargetOfCall, variableIdentifiersOfParametersForCallStmt);
}

TEST_F(ParserModuleOverloadResolutionTestsFixture, CallOfModuleWithOverloadResolutionUsingNumberOfDimensionsToSelectCorrectModuleWithOtherOverloadHavingMoreDimensions) {
    const std::string& stringifiedSyrecProgramToProcess = "module incr(inout a[2](4)) ++= a[0]; ++= a[1] module incr(inout a[2][3](4)) ++= a[0][0]; ++= a[1][0] module main(inout b[2](4)) call incr(b)";

    syrec::Module signatureOfModuleContainingCallStmt("main");
    signatureOfModuleContainingCallStmt.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "b", std::vector({2u}), 4));

    syrec::Module signatureOfModuleBeingTargetOfCall("incr");
    signatureOfModuleBeingTargetOfCall.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "a", std::vector({2u}), 4));

    std::vector<std::string> variableIdentifiersOfParametersForCallStmt;
    variableIdentifiersOfParametersForCallStmt.emplace_back(signatureOfModuleContainingCallStmt.parameters.front()->name);

    performTestExecution(stringifiedSyrecProgramToProcess, signatureOfModuleContainingCallStmt, signatureOfModuleBeingTargetOfCall, variableIdentifiersOfParametersForCallStmt);
}

TEST_F(ParserModuleOverloadResolutionTestsFixture, CallOfModuleWithOverloadResolutionUsingNumberOfParametersToSelectCorrectModuleWithOtherOverloadHavingLessParameters) {
    const std::string& stringifiedSyrecProgramToProcess = "module add(inout a[2](4)) ++= a[0]; ++= a[1] module add(inout a[2](4), in b(4)) a[0] += b; a[0] += b module main(inout lOp[2](4), in rOp(4)) call add(lOp, rOp)";

    syrec::Module signatureOfModuleContainingCallStmt("main");
    signatureOfModuleContainingCallStmt.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "lOp", std::vector({2u}), 4));
    signatureOfModuleContainingCallStmt.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::In, "rOp", std::vector({1u}), 4));

    syrec::Module signatureOfModuleBeingTargetOfCall("add");
    signatureOfModuleBeingTargetOfCall.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "a", std::vector({2u}), 4));
    signatureOfModuleBeingTargetOfCall.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::In, "b", std::vector({1u}), 4));

    std::vector<std::string> variableIdentifiersOfParametersForCallStmt;
    variableIdentifiersOfParametersForCallStmt.emplace_back(signatureOfModuleContainingCallStmt.parameters.front()->name);
    variableIdentifiersOfParametersForCallStmt.emplace_back(signatureOfModuleContainingCallStmt.parameters.back()->name);

    performTestExecution(stringifiedSyrecProgramToProcess, signatureOfModuleContainingCallStmt, signatureOfModuleBeingTargetOfCall, variableIdentifiersOfParametersForCallStmt);
}

TEST_F(ParserModuleOverloadResolutionTestsFixture, CallOfModuleWithOverloadResolutionUsingNumberOfParametersToSelectCorrectModuleWithOtherOverloadHavingMoreParameters) {
    const std::string& stringifiedSyrecProgramToProcess = "module add(inout a[2](4)) ++= a[0]; ++= a[1] module add(inout a[2](4), in b(4)) a[0] += b; a[0] += b module main(inout lOp[2](4)) call add(lOp)";

    syrec::Module signatureOfModuleContainingCallStmt("main");
    signatureOfModuleContainingCallStmt.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "lOp", std::vector({2u}), 4));

    syrec::Module signatureOfModuleBeingTargetOfCall("add");
    signatureOfModuleBeingTargetOfCall.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "a", std::vector({2u}), 4));

    std::vector<std::string> variableIdentifiersOfParametersForCallStmt;
    variableIdentifiersOfParametersForCallStmt.emplace_back(signatureOfModuleContainingCallStmt.parameters.front()->name);

    performTestExecution(stringifiedSyrecProgramToProcess, signatureOfModuleContainingCallStmt, signatureOfModuleBeingTargetOfCall, variableIdentifiersOfParametersForCallStmt);
}

TEST_F(ParserModuleOverloadResolutionTestsFixture, CallOfModuleDefinedNotInMainModuleCorrectlyResolved) {
    const std::string& stringifiedSyrecProgramToProcess = "module incr(inout a(4)) ++= a module incr(inout a[2](4)) wire x(4), y(4) x += a[0]; y += a[1]; call incr(x) module main() skip";

    syrec::Module signatureOfModuleContainingCallStmt("incr");
    signatureOfModuleContainingCallStmt.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "a", std::vector({2u}), 4));

    syrec::Module signatureOfModuleBeingTargetOfCall("incr");
    signatureOfModuleBeingTargetOfCall.parameters.emplace_back(std::make_shared<syrec::Variable>(syrec::Variable::Type::Inout, "a", std::vector({1u}), 4));

    std::vector<std::string> variableIdentifiersOfParametersForCallStmt;
    variableIdentifiersOfParametersForCallStmt.emplace_back("x");

    performTestExecution(stringifiedSyrecProgramToProcess, signatureOfModuleContainingCallStmt, signatureOfModuleBeingTargetOfCall, variableIdentifiersOfParametersForCallStmt);
}