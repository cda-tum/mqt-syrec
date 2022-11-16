#pragma once

#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <numeric>
#include <ostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

//boolean expression simplifier (Quine–McCluskey algorithm) https://github.com/madmann91/minbool
namespace minbool {

    inline size_t popcount(uint64_t n) {
        return std::bitset<64>(n).count();
    }

    struct MinTerm {
        struct Hash {
            size_t operator()(const MinTerm& term) const {
                // Bernstein's hash function
                return 33 * term.value ^ term.dash;
            }
        };

        enum class Value {
            Zero = 0,
            One  = 1,
            Dash = 2
        };

        MinTerm(uint64_t value = 0, uint64_t dash = 0) // NOLINT(google-explicit-constructor) taking 2 arguments.
            :
            value(value),
            dash(dash) {}

        Value operator[](size_t i) const {
            return ((dash >> i) & 1) != 0U ? Value::Dash : static_cast<Value>((value >> i) & 1);
        }

        [[nodiscard]] MinTerm combine(const MinTerm& other) const {
            uint64_t mask = (value ^ other.value) | (dash ^ other.dash);
            return {value & ~mask, dash | mask};
        }

        template<typename F>
        void foreachValue(F f, const size_t& n, size_t bit = 0, uint64_t cur = 0) const {
            if (bit == n) {
                f(cur);
            } else {
                auto val = (*this)[bit];
                if (val == Value::Dash) {
                    foreachValue(f, n, bit + 1, cur);
                    foreachValue(f, n, bit + 1, cur | (1 << bit));
                } else {
                    foreachValue(f, n, bit + 1, cur | (val == Value::Zero ? 0 : (1 << bit)));
                }
            }
        }

        bool operator<(const MinTerm& other) const {
            return value < other.value || (value == other.value && dash < other.dash);
        }

        bool operator==(const MinTerm& other) const {
            return value == other.value && dash == other.dash;
        }

        uint64_t value;
        uint64_t dash;
    };

    struct ImplicantTable {
        using MinTermN = MinTerm;

        std::vector<size_t>   groups;
        std::vector<bool>     marks;
        std::vector<MinTermN> terms;

        [[nodiscard]] size_t size() const { return terms.size(); }

        void fill(const std::vector<MinTermN>& minterms, const size_t& n) {
            groups.resize(n + 2U, 0U);
            for (const auto& term: minterms) {
                groups[popcount(term.value)]++;
            }
            std::partial_sum(groups.begin(), groups.end(), groups.begin());
            terms.resize(minterms.size());
            marks.resize(minterms.size());
            for (const auto& term: minterms) {
                terms[--groups[popcount(term.value)]] = term;
            }
        }

        void combine(std::vector<MinTermN>& res, const size_t& n) {
            for (size_t i = 0; i < n; ++i) {
                for (size_t j = groups[i]; j < groups[i + 1]; ++j) {
                    for (size_t k = groups[i + 1]; k < groups[i + 2]; ++k) {
                        auto const& termA = terms[j];
                        auto const& termB = terms[k];
                        if ((termA.value & termB.value) == termA.value && (termA.dash == termB.dash)) {
                            marks[j] = true;
                            marks[k] = true;
                            res.emplace_back(termA.combine(termB));
                        }
                    }
                }
            }
        }

        void primes(std::vector<MinTermN>& res) {
            for (size_t i = 0; i < terms.size(); ++i) {
                if (!marks[i]) {
                    res.emplace_back(terms[i]);
                }
            }
        }
    };

    struct PrimeChart {
        using MinTermN = MinTerm;
        std::unordered_map<uint64_t, std::vector<MinTerm>> columns;

        size_t size() const {
            return columns.size();
        }

        void fill(const std::vector<MinTerm>& primes, const size_t& n) {
            for (const auto& prime: primes) {
                prime.foreachValue([this, &prime](uint64_t value) {
                    columns[value].emplace_back(prime);
                },
                                   n);
            }
            for (auto& [first, second]: columns) {
                std::sort(second.begin(), second.end());
            }
        }

        void removeColumns(const std::vector<uint64_t>& values) {
            for (auto value: values) {
                columns.erase(value);
            }
        }

        bool removeEssentials(std::vector<MinTerm>& essentials, const size_t& n) {
            size_t count = essentials.size();
            for (auto& [first, second]: columns) {
                if (second.size() == 1) {
                    essentials.emplace_back(second.front());
                }
            }
            // No essential prime has been found
            if (essentials.size() == count) {
                return false;
            }
            std::sort(essentials.begin() + static_cast<int>(count), essentials.end());
            essentials.erase(std::unique(essentials.begin() + static_cast<int>(count), essentials.end()), essentials.end());

            std::for_each(essentials.begin() + static_cast<int>(count), essentials.end(), [&](const MinTerm& term) {
                term.foreachValue([this](uint64_t value) {
                    columns.erase(value);
                },
                                  n);
            });
            return true;
        }

        void removeHeuristic(std::vector<MinTerm>& solution, const size_t& n) {
            assert(size() > 0);
            std::unordered_map<MinTerm, size_t, typename MinTerm::Hash> covers;
            for (auto const& [first, second]: columns) {
                for (const auto& term: second) {
                    covers[term]++;
                }
            }
            // Heuristic: Remove the term that covers the most columns
            size_t  maxCovers = 0;
            MinTerm term;
            for (auto const& [first, second]: covers) {
                if (second > maxCovers) {
                    maxCovers = second;
                    term      = first;
                }
            }
            solution.emplace_back(term);
            term.foreachValue([this](uint64_t value) {
                columns.erase(value);
            },
                              n);
        }

        bool simplify() {
            bool change = false;

            for (auto& [pair1First, pair1Second]: columns) {
                for (auto& [pair2First, pair2Second]: columns) {
                    if (pair1First == pair2First) {
                        continue;
                    }
                    // Dominating columns are eliminated
                    if (std::includes(pair2Second.begin(), pair2Second.end(),
                                      pair1Second.begin(), pair1Second.end())) {
                        columns.erase(pair2First);
                        change = true;
                        break;
                    }
                }
            }

            // Transpose columns => rows
            std::unordered_map<MinTermN, std::vector<uint64_t>, typename MinTermN::Hash> rows;
            for (auto& [first, second]: columns) {
                for (auto const& term: second) {
                    rows[term].emplace_back(first);
                }
                second.clear();
            }
            for (auto& [first, second]: rows) {
                std::sort(second.begin(), second.end());
            }

            for (auto& [pair1First, pair1Second]: rows) {
                for (auto& [pair2First, pair2Second]: rows) {
                    if (pair1First == pair2First) {
                        continue;
                    }
                    // Dominated rows are eliminated
                    if (std::includes(pair1Second.begin(), pair1Second.end(),
                                      pair2Second.begin(), pair2Second.end())) {
                        rows.erase(pair2First);
                        change = true;
                        break;
                    }
                }
            }

            // Transpose rows => columns
            for (auto const& [first, second]: rows) {
                for (const auto& value: second) {
                    columns[value].emplace_back(first);
                }
            }
            for (auto& [first, second]: columns) {
                std::sort(second.begin(), second.end());
            }

            return change;
        }
    };

    inline std::vector<MinTerm> primeImplicants(std::vector<MinTerm>& terms, const size_t& n) {
        std::vector<MinTerm> primes;

        while (!terms.empty()) {
            ImplicantTable table;
            table.fill(terms, n);
            terms.clear();
            table.combine(terms, n);
            // Remove duplicates
            std::sort(terms.begin(), terms.end());
            terms.erase(std::unique(terms.begin(), terms.end()), terms.end());
            table.primes(primes);
        }
        return primes;
    }

    inline bool evalBoolean(const std::vector<MinTerm>& solution, uint64_t v, const size_t& n) {
        for (const auto& term: solution) {
            bool prod = true;
            for (size_t i = 0; i < n; ++i) {
                bool bit = ((v >> i) & 1) != 0;
                if (term[i] == MinTerm::Value::One) {
                    prod &= bit;
                } else if (term[i] == MinTerm::Value::Zero) {
                    prod &= !bit;
                }
            }
            if (prod) {
                return true;
            }
        }
        return false;
    }

    inline bool checkSolution(const std::vector<MinTerm>&  solution,
                              const std::vector<uint64_t>& onValues,
                              const std::vector<uint64_t>& dcValues, const size_t& n) {
        std::unordered_set<uint64_t> notOff;
        for (auto v: onValues) {
            notOff.emplace(v);
        }
        for (auto v: dcValues) {
            notOff.emplace(v);
        }
        for (auto v: onValues) {
            if (!evalBoolean(solution, v, n)) {
                return false;
            }
        }
        for (uint64_t i = (1 << n) - 1; i > 0; --i) {
            if (notOff.count(i) == 0 && evalBoolean(solution, i, n)) {
                return false;
            }
        }
        return notOff.count(0) != 0 || !evalBoolean(solution, 0, n);
    }

    inline std::vector<MinTerm> minimizeBoolean(
            const std::vector<uint64_t>& onValues, const size_t& n,
            const std::vector<uint64_t>& dcValues = std::vector<uint64_t>()) {
        if (onValues.size() <= 1) {
            return onValues.size() == 1 ? std::vector<MinTerm>{onValues.front()} : std::vector<MinTerm>();
        }

        std::vector<MinTerm> init;
        init.reserve(onValues.size() + dcValues.size());
        for (auto on: onValues) {
            init.emplace_back(on);
        }
        for (auto dc: dcValues) {
            init.emplace_back(dc);
        }
        auto primes = primeImplicants(init, n);

        PrimeChart chart;
        chart.fill(primes, n);
        chart.removeColumns(dcValues);

        std::vector<MinTerm> solution;
        do {
            bool change = chart.removeEssentials(solution, n);
            change |= chart.simplify();
            if (!change && chart.size() > 0) {
                chart.removeHeuristic(solution, n);
            }
        } while (chart.size() > 0);

        assert(checkSolution(solution, onValues, dcValues, n));
        return solution;
    }

} // namespace minbool
