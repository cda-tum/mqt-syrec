/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2010  The RevKit Developers <revkit@informatik.uni-bremen.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "core/syrec/grammar.hpp"

#include <fstream>

namespace syrec {

    bool parse(applications::ast_program& prog, const std::string& filename) {
        std::string content, line;

        std::ifstream is;
        is.open(filename.c_str(), std::ios::in);

        while (getline(is, line)) {
            content += line + '\n';
        }

        return parse_string(prog, content);
    }

    bool parse_string(applications::ast_program& prog, const std::string& program) {
        if (!applications::parse(prog, program.begin(), program.end())) {
            return false;
        }

        return true;
    }

} // namespace syrec
