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

// Boolean expression simplifier (Quine–McCluskey algorithm)
// Adapted from https://github.com/madmann91/minbool
namespace minbool {

    inline std::size_t popcount(const std::uint64_t n) {
        return std::bitset<64U>(n).count();
    }

    struct MinTerm {
        struct Hash {
            std::size_t operator()(const MinTerm& term) const {
                // Bernstein's hash function
                return 33 * term.value ^ term.dash;
            }
        };

        enum class Value {
            Zero = 0,
            One  = 1,
            Dash = 2
        };

        explicit MinTerm(const std::uint64_t value = 0U, const std::uint64_t dash = 0U):
            value(value),
            dash(dash) {}

        Value operator[](const std::size_t i) const {
            return ((dash >> i) & 1) != 0U ? Value::Dash : static_cast<Value>((value >> i) & 1);
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
                if (val == Value::Dash) {
                    foreachValue(f, n, bit + 1U, cur);
                    foreachValue(f, n, bit + 1U, cur | (1 << bit));
                } else {
                    foreachValue(f, n, bit + 1U, cur | (val == Value::Zero ? 0 : (1 << bit)));
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

    struct ImplicantTable {
        std::vector<std::size_t> groups;
        std::vector<bool>        marks;
        std::vector<MinTerm>     terms;

        [[nodiscard]] std::size_t size() const { return terms.size(); }

        void fill(const std::vector<MinTerm>& minterms, const std::size_t& n) {
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

        void combine(std::vector<MinTerm>& res, const std::size_t& n) {
            for (std::size_t i = 0; i < n; ++i) {
                for (std::size_t j = groups[i]; j < groups[i + 1]; ++j) {
                    for (std::size_t k = groups[i + 1]; k < groups[i + 2]; ++k) {
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

        [[nodiscard]] std::size_t size() const {
            return columns.size();
        }

        void fill(const std::vector<MinTerm>& primes, const std::size_t& n) {
            for (const auto& prime: primes) {
                prime.foreachValue([this, &prime](std::uint64_t value) {
                    columns[value].emplace_back(prime);
                },
                                   n);
            }
            for (auto& [first, second]: columns) {
                std::sort(second.begin(), second.end());
            }
        }

        void removeColumns(const std::vector<std::uint64_t>& values) {
            for (auto value: values) {
                columns.erase(value);
            }
        }

        bool removeEssentials(std::vector<MinTerm>& essentials, const std::size_t& n) {
            std::size_t const count = essentials.size();
            for (const auto& [first, second]: columns) {
                if (second.size() == 1U) {
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
                term.foreachValue([this](std::uint64_t value) {
                    columns.erase(value);
                },
                                  n);
            });
            return true;
        }

        void removeHeuristic(std::vector<MinTerm>& solution, const std::size_t& n) {
            assert(size() > 0);
            std::unordered_map<MinTerm, std::size_t, typename MinTerm::Hash> covers;
            for (auto const& [first, second]: columns) {
                for (const auto& term: second) {
                    covers[term]++;
                }
            }
            // Heuristic: Remove the term that covers the most columns
            std::size_t maxCovers = 0U;
            MinTerm     term;
            for (auto const& [first, second]: covers) {
                if (second > maxCovers) {
                    maxCovers = second;
                    term      = first;
                }
            }
            solution.emplace_back(term);
            term.foreachValue([this](std::uint64_t value) {
                columns.erase(value);
            },
                              n);
        }

        bool simplify() {
            bool change = false;

            for (const auto& [pair1First, pair1Second]: columns) {
                for (const auto& [pair2First, pair2Second]: columns) {
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
            std::unordered_map<MinTerm, std::vector<std::uint64_t>, typename MinTerm::Hash> rows;
            for (auto& [first, second]: columns) {
                for (auto const& term: second) {
                    rows[term].emplace_back(first);
                }
                second.clear();
            }
            for (auto& [first, second]: rows) {
                std::sort(second.begin(), second.end());
            }

            for (const auto& [pair1First, pair1Second]: rows) {
                for (const auto& [pair2First, pair2Second]: rows) {
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

    inline std::vector<MinTerm> primeImplicants(std::vector<MinTerm>& terms, const std::size_t& n) {
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

    inline bool evalBoolean(const std::vector<MinTerm>& solution, const std::uint64_t v, const std::size_t& n) {
        for (const auto& term: solution) {
            bool prod = true;
            for (std::size_t i = 0U; i < n; ++i) {
                bool const bit = ((v >> i) & 1) != 0;
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

    inline bool checkSolution(const std::vector<MinTerm>&       solution,
                              const std::vector<std::uint64_t>& onValues,
                              const std::vector<std::uint64_t>& dcValues, const std::size_t& n) {
        std::unordered_set<std::uint64_t> notOff;
        for (const auto& v: onValues) {
            notOff.emplace(v);
        }
        for (const auto& v: dcValues) {
            notOff.emplace(v);
        }
        for (const auto& v: onValues) {
            if (!evalBoolean(solution, v, n)) {
                return false;
            }
        }
        for (std::uint64_t i = (1 << n) - 1; i > 0U; --i) {
            if (notOff.count(i) == 0 && evalBoolean(solution, i, n)) {
                return false;
            }
        }
        return notOff.count(0) != 0U || !evalBoolean(solution, 0U, n);
    }

    inline std::vector<MinTerm> minimizeBoolean(
            const std::vector<std::uint64_t>& onValues, const std::size_t& n,
            const std::vector<std::uint64_t>& dcValues = {}) {
        if (onValues.size() <= 1U) {
            return onValues.size() == 1U ? std::vector<MinTerm>{onValues.front()} : std::vector<MinTerm>();
        }

        std::vector<MinTerm> init;
        init.reserve(onValues.size() + dcValues.size());
        for (const auto& on: onValues) {
            init.emplace_back(on);
        }
        for (const auto& dc: dcValues) {
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
