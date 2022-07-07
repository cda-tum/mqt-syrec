#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <utility>
#include <vector>

namespace syrec {

    class TruthTable {
    public:
        using ValueType = std::optional<bool>;

        struct CubeType {
            std::vector<ValueType> c;

            bool operator<(const CubeType& cv) const {
                return (this->c < cv.c);
            }

            [[nodiscard]] std::vector<CubeType> in_cube_to_full_cubes() const;

            void insert_zero();
        };

        using CubeVector = std::map<CubeType, CubeType>;

        using CubeTypeVec = std::vector<CubeType>;

        [[nodiscard]] std::size_t nInputs() const {
            if (!cubes.empty()) {
                return cubes.begin()->first.c.size();
            } else {
                return 0;
            }
        }

        [[nodiscard]] std::size_t nOutputs() const {
            if (!cubes.empty()) {
                return cubes.begin()->second.c.size();
            } else {
                return 0;
            }
        }

        void add_entry(CubeType const& input, CubeType const& output) {
            cubes.try_emplace(input, output);
        }

        void clear() {
            cubes.clear();
        }

        [[nodiscard]] const CubeVector& io_cube() const {
            return cubes;
        }

        static CubeType number_to_cube(std::size_t number, std::size_t bw);

        void extend_truth_table();

        void HuffmanCodes();

    private:
        CubeVector cubes;
    };

    struct MinHeapNode {
        TruthTable::CubeType data;

        std::size_t freq;

        MinHeapNode* left;
        MinHeapNode* right;

        MinHeapNode(TruthTable::CubeType data, std::size_t const freq):
            data(std::move(data)), freq(freq)

        {
            left = right = nullptr;
        }
    };

    struct compare {
        bool operator()(MinHeapNode const* l, MinHeapNode const* r) const

        {
            return (l->freq > r->freq);
        }
    };

    TruthTable::CubeType append_zero(TruthTable::CubeType enc);

    TruthTable::CubeType append_one(TruthTable::CubeType enc);

    void hufCodes(struct MinHeapNode* root, TruthTable::CubeType enc, TruthTable::CubeVector& encInOut);

} // namespace syrec
