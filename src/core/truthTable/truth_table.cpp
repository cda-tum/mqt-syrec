#include "core/truthTable/truth_table.hpp"

namespace syrec {

    std::vector<truthTable::cube_type> in_cube_to_full_cubes(const truthTable::cube_type& c, std::vector<truthTable::cube_type>& result) {
        auto first = c.begin();

        auto last = c.end();

        truthTable::cube_type cube(first, last);

        std::vector<unsigned> dcPositions;

        unsigned pos = 0;

        while (first != last) {
            if (!((*first).has_value())) // if DC
            {
                std::cout << "dontcare" << std::endl;
                dcPositions.emplace_back(pos);
            }

            ++pos;

            ++first;
        }

        for (unsigned i = 0; i < (1u << dcPositions.size()); ++i) {
            for (unsigned j = 0; j < dcPositions.size(); ++j) {
                unsigned localBit = i & (1u << (dcPositions.size() - j - 1)) ? 1 : 0;

                cube.at(dcPositions.at(j)) = localBit;
            }

            result.emplace_back(cube);
        }

        return result;
    }

    truthTable::cube_type number_to_cube(unsigned number, unsigned bw) {
        truthTable::cube_type c;

        for (unsigned i = 0; i < bw; ++i) {
            c.push_back((number & (1u << (bw - 1 - i))) ? true : false);
        }

        return c;
    }

    void extend_truth_table(truthTable& tt) {
        std::map<std::vector<truthTable::cube_type>, truthTable::cube_type> newCubes;

        for (auto& it: tt.io_cube()) {
            std::vector<truthTable::cube_type> inCube;

            in_cube_to_full_cubes(it.first, inCube);

            truthTable::cube_type output(it.second.begin(), it.second.end());

            newCubes.insert(std::make_pair(inCube, output));
        }

        tt.clear();

        for (auto& it: newCubes) {
            for (auto& itCube: it.first) {
                truthTable::cube_type newInEntry = itCube;

                tt.add_entry(newInEntry, it.second);
            }
        }

        truthTable::cube_type outCube(tt.num_outputs(), false);

        unsigned currentPos = 0;

        truthTable::cube_vector ioVec = tt.io_cube();

        for (auto it = ioVec.begin();; ++it) {
            unsigned pos = 0;

            unsigned i = tt.num_inputs();

            if (it == ioVec.end()) {
                pos = 1u << tt.num_inputs();
            } else {
                for (auto& inBit: it->first) {
                    pos |= (*inBit) << --i;
                }
            }

            for (i = currentPos; i < pos; ++i) {
                truthTable::cube_type inputCube = number_to_cube(i, tt.num_inputs());

                tt.add_entry(inputCube, outCube);
            }

            currentPos = pos;

            if (it == ioVec.end()) {
                break;
            }
        }
    }

    truthTable::cube_type append_zero(truthTable::cube_type enc) {
        enc.push_back(false);

        return enc;
    }

    truthTable::cube_type insert_zero(truthTable::cube_type& inNew) {
        inNew.insert(inNew.begin(), false);

        return inNew;
    }

    truthTable::cube_type append_one(truthTable::cube_type enc) {
        enc.push_back(true);

        return enc;
    }

    void hufCodes(struct syrec::MinHeapNode* root, truthTable::cube_type enc, truthTable::cube_vector& encInOut) {
        if (!root)
            return;

        if (root->data.size() != 0) {
            encInOut.insert({root->data, enc});
        }

        hufCodes(root->left, append_zero(enc), encInOut);

        hufCodes(root->right, append_one(enc), encInOut);
    }

    void HuffmanCodes(truthTable& tt) {
        std::map<truthTable::cube_type, int> outputFreq;

        truthTable::cube_vector encInOut;

        truthTable::cube_vector inOutCube = tt.io_cube();

        std::vector<truthTable::cube_type> outCube;

        for (auto& i: tt.io_cube()) {
            outCube.push_back(i.second);
        }

        for (auto& elem: outCube) {
            auto result = outputFreq.insert(std::pair<truthTable::cube_type, int>(elem, 1));
            if (result.second == false)
                result.first->second++;
        }

        for (auto& elem: outputFreq) {
            outputFreq[elem.first] = ceil(log2(elem.second));
        }

        // if the given truth table is not reversible

        if (outputFreq.size() != inOutCube.size()) {
            std::vector<unsigned> maxEncSize;

            unsigned encSize;

            truthTable::value_type emptyVal;

            truthTable::cube_type emptyVec;

            truthTable::cube_type codedVec;

            struct syrec::MinHeapNode *left, *right, *top;

            std::priority_queue<syrec::MinHeapNode*, std::vector<syrec::MinHeapNode*>, syrec::compare> minHeap;

            for (auto& elem: outputFreq)
                minHeap.push(new syrec::MinHeapNode(elem.first, elem.second));

            while (minHeap.size() != 1) {
                left = minHeap.top();
                minHeap.pop();

                right = minHeap.top();
                minHeap.pop();

                top = new syrec::MinHeapNode(emptyVec, (std::max(left->freq, right->freq) + 1));

                top->left  = left;
                top->right = right;

                minHeap.push(top);
            }

            //encode the output if necessary

            hufCodes(minHeap.top(), codedVec, encInOut);

            //increase the lengths of the encoded outputs with dont cares if required

            for (auto& i: encInOut) {
                maxEncSize.push_back(i.second.size());
            }

            encSize = *(std::max_element(maxEncSize.begin(), maxEncSize.end()));

            for (auto& i: encInOut) {
                if (encSize > i.second.size()) {
                    for (unsigned j = 0; j <= (encSize - i.second.size()); j++) {
                        i.second.push_back(emptyVal);
                    }
                }
            }

            //replace the outputs with the corrsponding encoded outputs

            for (auto& i: inOutCube) {
                inOutCube[i.first] = encInOut[i.second];
            }
        }

        //Append zeros to the input if the output length (either encoded or not) is higher compared to the input

        for (auto& i: inOutCube) {
            if (i.second.size() > i.first.size()) {
                for (unsigned j = 0; j <= (i.second.size() > i.first.size()); j++) {
                    truthTable::cube_type dummy = i.first;

                    auto node = inOutCube.extract(i.first);

                    node.key() = insert_zero(dummy);

                    inOutCube.insert(std::move(node));
                }
            }
        }

        tt.clear();

        //update the truth table

        for (auto& i: inOutCube) {
            truthTable::cube_type firstVal = i.first;

            truthTable::cube_type secondVal = i.second;

            tt.add_entry(firstVal, secondVal);
        }
    }

} // namespace syrec
