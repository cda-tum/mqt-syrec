/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2011  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

/**
 * @file write_systemc.hpp
 *
 * @brief Writes SyReC to SystemC
 */

#ifndef WRITE_SYSTEMC_HPP
#define WRITE_SYSTEMC_HPP

#include <core/properties.hpp>
#include <fstream>
#include <iostream>

namespace revkit {
    class pattern;

    namespace syrec {
        class program;
    }

    void write_systemc(const syrec::program& prog, std::ostream& os, properties::ptr settings, const pattern& p);
    void write_systemc(const syrec::program& prog, const std::string& filename, properties::ptr settings, const pattern& p);

} // namespace revkit

#endif /* WRITE_SYSTEMC_HPP */
