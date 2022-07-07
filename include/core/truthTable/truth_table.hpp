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

    class truthTable {
    public:
        using value_type = std::optional<bool>;

        using cube_type = std::vector<value_type>;

        using cube_vector = std::map<cube_type, cube_type>;


        [[nodiscard]] std::size_t num_inputs() const {
            if (!cubes.empty()) {
                return cubes.begin()->first.size();
            } else {
                return 0;
            }
        }

        [[nodiscard]] std::size_t num_outputs() const {
            if (!cubes.empty()) {
                return cubes.begin()->second.size();
            } else {
                return 0;
            }
        }

        void add_entry(cube_type const& input, cube_type& output) {
            cubes.try_emplace(input, output);
        }

        void clear() {
            cubes.clear();
        }

        [[nodiscard]] cube_vector& io_cube() {
            return cubes;
        }

        [[nodiscard]] const cube_vector& io_cube() const {
            return cubes;
        }

    private:
        cube_vector cubes;
    };

    void in_cube_to_full_cubes(const truthTable::cube_type& c, std::vector<truthTable::cube_type>& result);

    truthTable::cube_type number_to_cube(std::size_t number, std::size_t bw);

    void extend_truth_table(truthTable& tt);

    struct MinHeapNode {
        truthTable::cube_type data;

        std::size_t freq;

        MinHeapNode* left;
        MinHeapNode* right;

        MinHeapNode(truthTable::cube_type data, std::size_t const freq):
            data(std::move(data)), freq(freq)

        {
            left = right = nullptr;
        }
    };

    struct compare {
        bool operator()(MinHeapNode const* l, MinHeapNode const* r) const

        {
            return (l->freq < r->freq);
        }
    };

    truthTable::cube_type append_zero(truthTable::cube_type enc);

    truthTable::cube_type insert_zero(truthTable::cube_type& inNew);

    truthTable::cube_type append_one(truthTable::cube_type enc);

    void hufCodes(struct MinHeapNode* root, truthTable::cube_type enc, truthTable::cube_vector& encInOut);

    void HuffmanCodes(truthTable& tt);

} // namespace syrec
