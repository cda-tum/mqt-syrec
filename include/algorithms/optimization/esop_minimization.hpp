#pragma once

#include "core/truthTable/truth_table.hpp"

#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <numeric>
#include <ostream>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

// Boolean expression simplifier (Quine–McCluskey algorithm)
// Adapted from https://github.com/madmann91/minbool
namespace minbool {

    inline std::size_t popcount(const std::uint64_t n) {
        return std::bitset<64U>(n).count();
    }

    struct MinTerm {
        using Value = syrec::TruthTable::Cube::Value;

        MinTerm(const std::uint64_t value = 0U, const std::uint64_t dash = 0U):
            //NOLINT
            value(value),
            dash(dash) {}

        Value operator[](const std::size_t i) const {
            return ((dash >> i) & 1) != 0U ? Value{} : static_cast<Value>((value >> i) & 1);
        }

        [[nodiscard]] MinTerm combine(const MinTerm& other) const {
            const std::uint64_t mask = (value ^ other.value) | (dash ^ other.dash);
            return MinTerm{value & ~mask, dash | mask};
        }

        template<typename F>
        void foreachValue(F f, const std::size_t n, const std::size_t bit = 0U, const std::uint64_t cur = 0U) const {
            if (bit == n) {
                f(cur);
            } else {
                auto val = (*this)[bit];
                if (val == Value{}) {
                    foreachValue(f, n, bit + 1U, cur);
                    foreachValue(f, n, bit + 1U, cur | (1 << bit));
                } else {
                    foreachValue(f, n, bit + 1U, cur | (val == Value{false} ? 0 : (1 << bit)));
                }
            }
        }

        bool operator<(const MinTerm& other) const {
            return value < other.value || (value == other.value && dash < other.dash);
        }

        bool operator==(const MinTerm& other) const {
            return value == other.value && dash == other.dash;
        }

        std::uint64_t value;
        std::uint64_t dash;
    };

} // namespace minbool

namespace std {
    template<>
    struct hash<minbool::MinTerm> {
        auto operator()(const minbool::MinTerm& term) const -> size_t {
            return (33 * term.value ^ term.dash);
        }
    };
} // namespace std

namespace minbool {

    struct ImplicantTable {
        std::vector<std::size_t> groups;
        std::vector<bool>        marks;
        std::vector<MinTerm>     terms;
        std::size_t              nBits;

        explicit ImplicantTable(const std::size_t nBits):
            nBits(nBits) {}

        [[nodiscard]] std::size_t size() const { return terms.size(); }

        void fill(const std::vector<MinTerm>& minterms);

        void combine(std::vector<MinTerm>& res);

        void primes(std::vector<MinTerm>& res) {
            for (std::size_t i = 0U; i < terms.size(); ++i) {
                if (!marks[i]) {
                    res.emplace_back(terms[i]);
                }
            }
        }
    };

    struct PrimeChart {
        std::unordered_map<std::uint64_t, std::vector<MinTerm>> columns;
        std::size_t                                             nBits;

        explicit PrimeChart(const std::size_t nBits):
            nBits(nBits) {}

        [[nodiscard]] std::size_t size() const {
            return columns.size();
        }

        void fill(const std::vector<MinTerm>& primes);

        bool removeEssentials(std::vector<MinTerm>& essentials);

        void removeHeuristic(std::vector<MinTerm>& solution);

        bool simplify();
    };

    std::vector<MinTerm> primeImplicants(std::vector<MinTerm>& terms, const std::size_t& n);

    bool evalBoolean(const std::vector<MinTerm>& solution, std::uint64_t v, const std::size_t& n);

    bool checkSolution(const std::vector<MinTerm>& solution, const std::unordered_set<std::uint64_t>& onValues, const std::size_t& n);

    syrec::TruthTable::Cube::Vector minimizeBoolean(syrec::TruthTable::Cube::Vector const& sigVec);

} // namespace minbool
