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
        using value_type = std::optional<bool>;

        struct cube_type {
            std::vector<value_type> c;

            bool operator<(const cube_type& cv) const{
                return (this->c < cv.c);
            }

            [[nodiscard]] std::vector<cube_type> in_cube_to_full_cubes() const;

            void insert_zero();
        };



        using cube_vector = std::map<cube_type, cube_type>;

        using CubeTypeVec = std::vector<cube_type>;


        [[nodiscard]] std::size_t num_inputs() const {
            if (!cubes.empty()) {
                return cubes.begin()->first.c.size();
            } else {
                return 0;
            }
        }

        [[nodiscard]] std::size_t num_outputs() const {
            if (!cubes.empty()) {
                return cubes.begin()->second.c.size();
            } else {
                return 0;
            }
        }

        void add_entry(cube_type const& input, cube_type const& output) {
            cubes.try_emplace(input, output);
        }

        void clear() {
            cubes.clear();
        }

        [[nodiscard]] const cube_vector& io_cube() const {
            return cubes;
        }

        static cube_type number_to_cube(std::size_t number, std::size_t bw);

        void extend_truth_table();

        void HuffmanCodes();

    private:
        cube_vector cubes;
    };

    struct MinHeapNode {
        TruthTable::cube_type data;

        std::size_t freq;

        MinHeapNode* left;
        MinHeapNode* right;

        MinHeapNode(TruthTable::cube_type data, std::size_t const freq):
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



    TruthTable::cube_type append_zero(TruthTable::cube_type enc);

    TruthTable::cube_type append_one(TruthTable::cube_type enc);

    void hufCodes(struct MinHeapNode* root, TruthTable::cube_type enc, TruthTable::cube_vector& encInOut);

} // namespace syrec
