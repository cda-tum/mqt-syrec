#ifndef TRUTH_TABLE_HPP
#define TRUTH_TABLE_HPP

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <map>
#include <math.h>
#include <queue>
#include <stdio.h>
#include <string>
#include <vector>

namespace syrec {

    /*struct MinHeapNode {
        std::string data;

        int freq;

        MinHeapNode *left, *right;

        MinHeapNode(std::string data, unsigned freq)

        {
            left = right = NULL;
            this->data   = data;
            this->freq   = freq;
        }
    };

    struct compare {
        bool operator()(MinHeapNode* l, MinHeapNode* r)

        {
            return (l->freq > r->freq);
        }
    };
*/
    class truth_table {
    public:
        typedef std::string io_type;

        typedef std::vector<io_type> io_vector;

        void set_iocombination(io_vector& s) {
            _iocombination.clear();
            _iocombination = s;
        }

        io_vector get_iocombination() {
            return _iocombination;
        }

        /* unsigned num_inputs() const {
            if (_input.size()) {
                return _input[0].size();
            } else {
                return 0;
            }
        }

        unsigned num_outputs() const {
            if (_output.size()) {
                return _output[0].size();
            } else {
                return 0;
            }
        }

        void add_entry(const io_type& input, const io_type& output) {
            _input.push_back(input);
            _output.push_back(output);
        }



        void hufCodes(struct MinHeapNode* root, std::string str, std::map<std::string, std::string>& check_map) {
            if (!root)
                return;

            if (root->data != "$") {
                check_map.insert({root->data, str});
            }

            hufCodes(root->left, str + "0", check_map);
            hufCodes(root->right, str + "1", check_map);
        }

        void HuffmanCodes() {
            std::map<std::string, int> output_freq;

            for (auto& elem: _output) {
                auto result = output_freq.insert(std::pair<std::string, int>(elem, 1));
                if (result.second == false)
                    result.first->second++;
            }

            for (auto& elem: output_freq) {
                output_freq[elem.first] = ceil(log2(elem.second));
            }

            struct MinHeapNode *left, *right, *top;

            std::priority_queue<MinHeapNode*, std::vector<MinHeapNode*>, compare> minHeap;

            for (auto& elem: output_freq)
                minHeap.push(new MinHeapNode(elem.first, elem.second));

            while (minHeap.size() != 1) {
                left = minHeap.top();
                minHeap.pop();

                right = minHeap.top();
                minHeap.pop();

                top = new MinHeapNode("$", std::max(left->freq, right->freq) + 1);

                top->left  = left;
                top->right = right;

                minHeap.push(top);
            }

            hufCodes(minHeap.top(), "", _enc);
        }

        void ioCombination() {
            int bitwidth = _input[0].size();

            for (auto& elem: _enc) {
                int dont_care = bitwidth - elem.second.size();

                if (dont_care) {
                    for (int i = 0; i < dont_care; i++) {
                        elem.second.push_back('_');
                    }
                }
            }

            for (int i = 0; i < static_cast<int>(_output.size()); i++) {
                _output[i] = _enc[_output[i]];
            }

            for (int i = 0; i < static_cast<int>(_output.size()); i++) {
                std::string io;

                for (int j = 0; j < static_cast<int>(_input[0].size()); j++) {
                    io += _input[i][j];
                    io += _output[i][j];
                }

                _iocombination.push_back(io);
            }

        }

        */

    private:
        /*io_vector _input;

        io_vector _output;

        std::map<io_type, io_type> _enc;
        */

        io_vector _iocombination;
    };

} // namespace syrec
#endif /* TRUTH_TABLE_HPP */
