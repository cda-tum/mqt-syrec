#include "core/truthTable/truth_table.hpp"

namespace syrec {

    truthTable::CubeTypeVec truthTable::cube_type::in_cube_to_full_cubes() const {

        truthTable::CubeTypeVec result;

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

        for (std::size_t i = 0; i < (1 << dcPositions.size()); ++i) {
            for (std::size_t j = 0; j < dcPositions.size(); ++j) {
                cube.at(dcPositions.at(j))  = ((i & (1u << (dcPositions.size() - j - 1))) != 0);


            }
            truthTable::cube_type cubeObj;
            cubeObj.c = cube;
            result.emplace_back(cubeObj);
        }

        return result;
    }

    truthTable::cube_type truthTable::number_to_cube(std::size_t number, std::size_t bw) {
        truthTable::cube_type ct;

        for (std::size_t i = 0; i < bw; ++i) {
            ct.c.emplace_back((number & (1u << (bw - 1 - i))) != 0);
        }

        return ct;
    }

    void truthTable::extend_truth_table() {
        std::map<truthTable::CubeTypeVec, truthTable::cube_type> newCubes;

        for (auto const& [key, value]: io_cube()) {
            newCubes.try_emplace(key.in_cube_to_full_cubes(), value);
        }

        clear();

        for (auto const& [key, value]: newCubes) {
            for (auto const& itCube: key) {
                add_entry(itCube, value);
            }
        }

        truthTable::cube_type outCube;

        for (std::size_t outSize = 0; outSize < num_outputs(); outSize++){
            outCube.c.emplace_back(false);
        }

        unsigned currentPos = 0;

        const truthTable::cube_vector ioVec = io_cube();

        for (auto it = io_cube().begin();; ++it) {
            unsigned pos = 0;

            std::size_t i = num_inputs();

            if (it == io_cube().end()) {
                pos = 1u << num_inputs();
            } else {
                for (auto& inBit: it->first.c) {
                    pos |= (*inBit) << --i;
                }
            }

            for (i = currentPos; i < pos; ++i) {
                const truthTable::cube_type inputCube = number_to_cube(i, num_inputs());

                add_entry(inputCube, outCube);
            }

            currentPos = pos;

            if (it == io_cube().end()) {
                break;
            }
        }
    }


    truthTable::cube_type append_zero(truthTable::cube_type enc) {
        enc.c.emplace_back(false);

        return enc;
    }

    void truthTable::cube_type::insert_zero() {
        c.insert(c.begin(), false);
    }

    truthTable::cube_type append_one(truthTable::cube_type enc) {
        enc.c.emplace_back(true);

        return enc;
    }

    void hufCodes(struct syrec::MinHeapNode* root, truthTable::cube_type enc, truthTable::cube_vector& encInOut) {
        if (!root)
            return;

        if (!(root->data.c.empty())) {
            encInOut.try_emplace(root->data, enc);
        }

        hufCodes(root->left, append_zero(enc), encInOut);

        hufCodes(root->right, append_one(enc), encInOut);
    }

    void truthTable::HuffmanCodes() {
        std::map<truthTable::cube_type, std::size_t> outputFreq;

        truthTable::cube_vector encInOut;

        truthTable::cube_vector inOutCube = io_cube();

        for (auto const& [in, out]: io_cube()) {
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
            encSize = minHeap.top()->freq;
            hufCodes(minHeap.top(), codedVec, encInOut);

            //increase the lengths of the encoded outputs with dont cares if required

            for (auto& [key, value]: encInOut) {
                if (encSize > value.c.size()) {
                    std::fill_n(std::back_inserter(value.c), (encSize - value.c.size()), emptyVal);
                }
            }

            //replace the outputs with the corrsponding encoded outputs

            for (auto const& [key, value]: inOutCube) {
                inOutCube[key] = encInOut[value];
            }
        }

        //Append zeros to the input if the output length (either encoded or not) is higher compared to the input
        for (auto const& [key, value]: inOutCube) {
            const std::size_t outSize = value.c.size();

            const std::size_t inSize = key.c.size();

            if(outSize > inSize) {  //not needed, just a safe condition
                for (std::size_t j = 0; j < (outSize - inSize); j++) {
                    truthTable::cube_type dummy = key;

                    dummy.insert_zero();
                    auto node = inOutCube.extract(key);
                    node.key() = dummy;

                    inOutCube.insert(std::move(node));
                }
            }

        }

        clear();

        //update the truth table

        for (auto const& [key, value]: inOutCube) {

            add_entry(key, value);
        }
    }

} // namespace syrec
