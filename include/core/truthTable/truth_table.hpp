#ifndef TRUTH_TABLE_HPP
#define TRUTH_TABLE_HPP

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <map>
#include <optional>
#include <queue>
#include <vector>

namespace syrec {

    class truthTable {
    public:
        typedef std::optional<bool> value_type;

        typedef std::vector<value_type> cube_type;

        typedef std::map<cube_type, cube_type> cube_vector;

        unsigned num_inputs() const {
            if (cubes.size()) {
                return cubes.begin()->first.size();
            } else {
                return 0;
            }
        }

        unsigned num_outputs() const {
            if (cubes.size()) {
                return cubes.begin()->second.size();
            } else {
                return 0;
            }
        }

        void add_entry(cube_type& input, cube_type& output) {
            cubes.insert(std::make_pair(input, output));
        }

        void clear() {
            cubes.clear();
        }

        cube_vector io_cube() {
            return cubes;
        }

    private:
        cube_vector cubes;
    };

    std::vector<truthTable::cube_type> in_cube_to_full_cubes(const truthTable::cube_type& c, std::vector<truthTable::cube_type>& result);

    truthTable::cube_type number_to_cube(unsigned number, unsigned bw);

    void extend_truth_table(truthTable& tt);

    struct MinHeapNode {
        truthTable::cube_type data;

        int freq;

        MinHeapNode *left, *right;

        MinHeapNode(truthTable::cube_type data, unsigned freq)

        {
            left = right = NULL;
            this->data   = data;
            this->freq   = freq;
        }
    };

    struct compare {
        bool operator()(MinHeapNode* l, MinHeapNode* r)

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

#endif /* TRUTH_TABLE_HPP */
