#pragma once

#include "core/truthTable/truth_table.hpp"

namespace syrec {

    void rootPath(dd::mNode* src, dd::mNode* dst, TruthTable::Cube::Vector& sigVec, TruthTable::Cube& tempVec);

    void sigPath(dd::mNode* src, TruthTable::Cube::Vector& sigVec, TruthTable::Cube& tempVec);

    bool algoP3(dd::mNode* current);

    void algoP4(dd::mEdge& src, dd::mNode* current, syrec::TruthTable::Cube::Vector& p1SigVec, syrec::TruthTable::Cube::Vector& p2SigVec, const std::vector<std::size_t>& indices, std::unique_ptr<dd::Package<>>& dd);

    void algoP1(dd::mEdge& src, dd::mNode* current, std::unique_ptr<dd::Package<>>& dd);

    void algoP2(dd::mEdge& src, dd::mNode* current, std::unique_ptr<dd::Package<>>& dd);

    void algoP(dd::mEdge& src, dd::mNode* current, std::unique_ptr<dd::Package<>>& dd);

    void algoQ(dd::mEdge& src, std::unique_ptr<dd::Package<>>& dd);
} // namespace syrec
