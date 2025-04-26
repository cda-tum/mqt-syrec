/*
 * Copyright (c) 2023 - 2025 Chair for Design Automation, TUM
 * Copyright (c) 2025 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#include "core/syrec/program.hpp"

namespace syrec {

    bool Program::readFile(const std::string& filename, const ReadProgramSettings settings, std::string& error) {
        std::string content;
        std::string line;

        std::ifstream is;
        is.open(filename.c_str(), std::ios::in);

        while (getline(is, line)) {
            content += line + '\n';
        }

        return readProgramFromString(content, settings, error);
    }

    std::string Program::read(const std::string& filename, const ReadProgramSettings settings) {
        if (std::string errorMessage; !(readFile(filename, settings, errorMessage))) {
            return errorMessage;
        }
        return {};
    }

} // namespace syrec
