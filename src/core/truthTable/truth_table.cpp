#include "core/truthTable/truth_table.hpp"

namespace syrec {

    void in_cube_to_full_cubes(const truthTable::cube_type& c, truthTable::CubeTypeVec& result) {

        auto cube = c ;

        std::vector<std::size_t> dcPositions;

        std::size_t pos = 0;

        for(auto const& it : cube) {
            if (!(it.has_value())) // if DC
            {
                dcPositions.emplace_back(pos);
            }

            ++pos;
        }

        for (std::size_t i = 0; i < (1u << dcPositions.size()); ++i) {
            for (std::size_t j = 0; j < dcPositions.size(); ++j) {
                cube.at(dcPositions.at(j))  = ((i & (1u << (dcPositions.size() - j - 1))) != 0);


            }

            result.emplace_back(cube);
        }

        return;
    }

    truthTable::cube_type number_to_cube(std::size_t number, std::size_t bw) {
        truthTable::cube_type c;

        for (std::size_t i = 0; i < bw; ++i) {
            c.emplace_back((number & (1u << (bw - 1 - i))) != 0);
        }

        return c;
    }

    void extend_truth_table(truthTable& tt) {
        std::map<truthTable::CubeTypeVec, truthTable::cube_type> newCubes;

        for (auto const& [key, value]: tt.io_cube()) {
            truthTable::CubeTypeVec inCube;

            in_cube_to_full_cubes(key, inCube);

            newCubes.try_emplace(inCube, value);
        }

        tt.clear();

        for (auto const& [key, value]: newCubes) {
            for (auto const& itCube: key) {
                tt.add_entry(itCube, value);
            }
        }

        const truthTable::cube_type outCube(tt.num_outputs(), false);

        unsigned currentPos = 0;

        const truthTable::cube_vector ioVec = tt.io_cube();

        for (auto it = ioVec.begin();; ++it) {
            unsigned pos = 0;

            std::size_t i = tt.num_inputs();

            if (it == ioVec.end()) {
                pos = 1u << tt.num_inputs();
            } else {
                for (auto& inBit: it->first) {
                    pos |= (*inBit) << --i;
                }
            }

            for (i = currentPos; i < pos; ++i) {
                const truthTable::cube_type inputCube = number_to_cube(i, tt.num_inputs());

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

        if (!root->data.empty()) {
            encInOut.try_emplace(root->data, enc);
        }

        hufCodes(root->left, append_zero(enc), encInOut);

        hufCodes(root->right, append_one(enc), encInOut);
    }

    void HuffmanCodes(truthTable& tt) {
        std::map<truthTable::cube_type, std::size_t> outputFreq;

        truthTable::cube_vector encInOut;

        truthTable::cube_vector inOutCube = tt.io_cube();

        for (auto const& [in, out]: tt.io_cube()) {
            auto [key, value] = outputFreq.try_emplace(out, 1);
            if (!value)
                key->second++;
        }

        // if the given truth table is not reversible

        if (outputFreq.size() != inOutCube.size()) {
            std::vector<std::size_t> maxEncSize;

            std::size_t encSize;

            truthTable::value_type emptyVal;

            truthTable::cube_type emptyVec;

            truthTable::cube_type codedVec;

            struct syrec::MinHeapNode* left;
            struct syrec::MinHeapNode* right;
            struct syrec::MinHeapNode* top;

            std::priority_queue<syrec::MinHeapNode*, std::vector<syrec::MinHeapNode*>, syrec::compare> minHeap;

            for (auto const& [key, value]: outputFreq)
                minHeap.push(new syrec::MinHeapNode(key, (std::size_t)ceil(log2((double)value))));

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

            for (auto const& [key, value]: encInOut) {
                maxEncSize.push_back(value.size());
            }

            encSize = *(std::max_element(maxEncSize.begin(), maxEncSize.end()));

            for (auto& [key, value]: encInOut) {
                if (encSize > value.size()) {
                    std::fill_n(std::back_inserter(value), (encSize - value.size()), emptyVal);
                }
            }

            //replace the outputs with the corrsponding encoded outputs

            for (auto const& [key, value]: inOutCube) {
                inOutCube[key] = encInOut[value];
            }
        }

        //Append zeros to the input if the output length (either encoded or not) is higher compared to the input
        for (auto const& [key, value]: inOutCube) {
            const std::size_t outSize = value.size();

            const std::size_t inSize = key.size();

            if(outSize > inSize) {  //not needed, just a safe condition
                for (std::size_t j = 0; j < (outSize - inSize); j++) {
                    truthTable::cube_type dummy = key;

                    auto node = inOutCube.extract(key);

                    node.key() = insert_zero(dummy);

                    inOutCube.insert(std::move(node));
                }
            }

        }

        tt.clear();

        //update the truth table

        for (auto const& [key, value]: inOutCube) {

            tt.add_entry(key, value);
        }
    }

} // namespace syrec
