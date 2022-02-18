#include <pybind11/chrono.h>
#include <pybind11/complex.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
//#include <boost/python.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/iterator/transform_iterator.hpp>
//#include <boost/python/stl_iterator.hpp>
//#include <boost/python/dict.hpp>
//#include <boost/python/extract.hpp>
#include "Dummy.hpp"

#include <algorithms/simulation/simple_simulation.hpp>
#include <algorithms/synthesis/synthesis.hpp>
#include <algorithms/synthesis/syrec_synthesis.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <core/circuit.hpp>
#include <core/functions/control_lines.hpp>
#include <core/functions/target_lines.hpp>
#include <core/gate.hpp>
#include <core/pattern.hpp>
#include <core/properties.hpp>
#include <core/syrec/parser.hpp>
#include <core/syrec/program.hpp>
#include <core/target_tags.hpp>
#include <core/truth_table.hpp>
#include <core/utils/costs.hpp>

//using namespace boost::python;
namespace py = pybind11;
using namespace revkit;
using namespace revkit::syrec;
// Algorithms / Optimization

circuit::const_iterator (circuit::*begin1)() const = &circuit::begin;
circuit::const_iterator (circuit::*end1)() const   = &circuit::end;
circuit::iterator (circuit::*begin2)()             = &circuit::begin;
circuit::iterator (circuit::*end2)()               = &circuit::end;

circuit::const_reverse_iterator (circuit::*rbegin1)() const = &circuit::rbegin;
circuit::const_reverse_iterator (circuit::*rend1)() const   = &circuit::rend;
circuit::reverse_iterator (circuit::*rbegin2)()             = &circuit::rbegin;
circuit::reverse_iterator (circuit::*rend2)()               = &circuit::rend;

/*
bus_collection& (circuit::*inputbuses1)() = &circuit::inputbuses;
bus_collection& (circuit::*outputbuses1)() = &circuit::outputbuses;
bus_collection& (circuit::*statesignals1)() = &circuit::statesignals;
void (circuit::*add_module1)(const std::string&, const circuit&) = &circuit::add_module;

void circuit_append_gate( circuit& c, const gate& g )
{
  c.append_gate() = g;
}

void circuit_prepend_gate( circuit& c, const gate& g )
{
  c.prepend_gate() = g;
}

void circuit_insert_gate( circuit& c, unsigned pos, const gate& g )
{
  c.insert_gate( pos ) = g;
}

dict circuit_modules( const circuit& c )
{
  dict d;

  typedef std::pair<std::string, std::shared_ptr<circuit> > pair_t;
  foreach_ ( const pair_t& p, c.modules() )
  {
    d[p.first] = p.second;
  }

  return d;
}
*/

py::dict circuit_annotations(const circuit& c, const gate& g) {
    py::dict d;

    boost::optional<const std::map<std::string, std::string>&> annotations = c.annotations(g);

    if (annotations) {
        typedef std::pair<std::string, std::string> pair_t;
        for (const pair_t p: *annotations) {
            d[py::cast(p.first)] = p.second;
        }
    }

    return d;
}

template<typename T>
py::list my_append(py::list l, T a) {
    l.append(a);
    return l;
}

template<typename T, typename C>
py::list list_getter(const C& circ, std::function<const std::vector<T>&(const C*)> fgetter) {
    py::list l;
    std::for_each(fgetter(&circ).begin(), fgetter(&circ).end(), std::bind(my_append<T>, l, std::placeholders::_1));
    return l;
}

/*template<typename T, typename C>
void list_setter( C& circ, py::object o, std::function<void (C*, const std::vector<T>&)> fsetter )
{
  stl_input_iterator<T> begin( o ), end;
  std::vector<T> v;
  std::copy( begin, end, std::back_inserter( v ) );
  fsetter( &circ, v );
}
*/
py::list inputs_getter(const circuit& circ) {
    return list_getter<std::string, circuit>(circ, &circuit::inputs);
}
/*
py::list pybind_inputs_getter( const circuit& circ ) 
{
  list temp = list_getter<std::string, circuit>( circ, &circuit::inputs ); 
  py::list l;
  for(int i=0;i<boost::python::len(temp);i++)
  { 
    //std::string maa;
    l.append(temp[i]);
    //std::cout<< maa <<std::endl;
    //l.append(maa);
  }
  
  return l;
}*/

py::list outputs_getter(const circuit& circ) {
    return list_getter<std::string, circuit>(circ, &circuit::outputs);
}
/*
py::list pybind_outputs_getter( const circuit& circ ) 
{
  list temp = list_getter<std::string, circuit>( circ, &circuit::outputs);
  py::list l;
  for(int i=0;i<boost::python::len(temp);i++)
  { 
    //std::string maa;
    //maa = boost::python::extract<std::string>(temp[i]);
    //std::cout<< maa <<std::endl;
    l.append(temp[i]);
  }
  
  return l;
}*/

py::list constants_getter(const circuit& circ) {
    return list_getter<constant, circuit>(circ, &circuit::constants);
}

std::vector<gate> new_gates(const circuit& circ) {
    std::vector<gate>       my_gates;
    circuit::const_iterator first = circ.begin();
    circuit::const_iterator last  = circ.end();
    while (first != last) {
        my_gates.push_back(*first);
        ++first;
    }
    return my_gates;
}

/*
py::list pybind_constants_getter( const circuit& circ ) 
{
  list temp = list_getter<constant, circuit>( circ, &circuit::constants ); 
  py::list l;
  for(int i=0;i<boost::python::len(temp);i++)
  { 
    //std::string maa;
    //maa = boost::python::extract<std::string>(temp[i]);
    //std::cout<< maa <<std::endl;
    l.append(temp[i]);
  }
  
  return l;
}
*/
py::list garbage_getter(const circuit& circ) {
    return list_getter<bool, circuit>(circ, &circuit::garbage);
}

/*
py::list pybind_garbage_getter( const circuit& circ ) 
{
  list temp = list_getter<bool, circuit>( circ, &circuit::garbage );  
  py::list l;
  for(int i=0;i<boost::python::len(temp);i++)
  { 
    //std::string maa;
    //maa = boost::python::extract<std::string>(temp[i]);
    //std::cout<< maa <<std::endl;
    l.append(temp[i]);
  }
  
  return l;
}
*/
//list inputs_getter( const circuit& circ ) { return list_getter<std::string, circuit>( circ, &circuit::inputs ); }
//void inputs_setter( circuit& circ, py::object o ) { return list_setter<std::string, circuit>( circ, o, &circuit::set_inputs ); }
//list outputs_getter( const circuit& circ ) { return list_getter<std::string, circuit>( circ, &circuit::outputs ); }
//void outputs_setter( circuit& circ, py::object o ) { return list_setter<std::string, circuit>( circ, o, &circuit::set_outputs ); }
//list constants_getter( const circuit& circ ) { return list_getter<constant, circuit>( circ, &circuit::constants ); }
//void constants_setter( circuit& circ, py::object o ) { return list_setter<constant, circuit>( circ, o, &circuit::set_constants ); }
//list garbage_getter( const circuit& circ ) { return list_getter<bool, circuit>( circ, &circuit::garbage ); }
//void garbage_setter( circuit& circ, py::object o ) { return list_setter<bool, circuit>( circ, o, &circuit::set_garbage ); }

/*
const gate& circuit_get_gate( const circuit& circ, const size_t& i )
{
  return *( circ.begin() + i );
}

circuit subcircuit1( const circuit& base, unsigned from, unsigned to )
{
  return subcircuit( base, from, to );
}

circuit subcircuit2( const circuit& base, unsigned from, unsigned to, object list )
{
  stl_input_iterator<unsigned> begin( list ), end;
  std::vector<unsigned> filter( begin, end );
  return subcircuit( base, from, to, filter );
}
*/
/*list circuit_filter( const circuit& circ )
{
  list l;

  unsigned lines;
  std::vector<unsigned> filter;

  boost::tie( lines, filter ) = circ.filter();

  l.append<unsigned>( lines );

  list lfilter;
  std::for_each( filter.begin(), filter.end(), boost::bind( &list::append<unsigned>, &lfilter, boost::placeholders::_1 ) );

  l.append<list>( lfilter );

  return l;
}
*/
/*py::list pybind_circuit_filter( const circuit& circ )
{
  list l;
  py::list l1;
  unsigned lines;
  std::vector<unsigned> filter;

  boost::tie( lines, filter ) = circ.filter();

  l.append<unsigned>(lines);
  l1.append(lines);
  list lfilter;
  py::list lfilter1;
  std::for_each( filter.begin(), filter.end(), boost::bind( &list::append<unsigned>, &lfilter, boost::placeholders::_1 ) );

  l.append<list>(lfilter);

  for(int i =0;i<boost::python::len(lfilter);i++)
  {
    lfilter1.append(lfilter[i]);  
  }

  l1.append(lfilter1);
  
 
  return l1;
}

*/
/*
template<typename K, typename T>
void multimap_to_map( const std::multimap<K, T>& mmap, std::map<K, std::vector<T> >& map )
{
  for ( typename std::multimap<K, T>::const_iterator it = mmap.begin(); it != mmap.end(); ++it )
  {
    if ( map.find( it->first ) == map.end() )
    {
      map.insert( std::make_pair( it->first, std::vector<T>()  ) );
    }

    map[it->first].push_back( it->second );
  }
}
*/
int add_new(int i, int j) {
    return i + j;
}

///Properties

template<typename T>
void properties_set(properties& prop, const std::string& key, const T& value) {
    prop.set(key, value);
}

template<typename T>
T properties_get1(properties& prop, const std::string& key) {
    return prop.get<T>(key);
}

//Program

std::string py_read_program(program& prog, const std::string& filename, const read_program_settings& settings) {
    std::string error_message;

    if (!read_program(prog, filename, settings, &error_message)) {
        return error_message;
    } else {
        return std::string();
    }
}

//costs

//costs_by_gate_func quantum_costs1()
//{
//  return quantum_costs();
//}

//costs_by_gate_func transistor_costs1()
//{
//  return transistor_costs();
//}

//unsigned costs1( const circuit& circ, const costs_by_circuit_func& f )
//{
//  return costs( circ, f );
//}

//unsigned costs2( const circuit& circ, const costs_by_gate_func& f )
//{
//  return costs( circ, f );
//}

//dynamic bitset

std::string bitset_to_string(boost::dynamic_bitset<> const& bitset) {
    std::string res;
    for (unsigned i = 0; i < bitset.size(); i++) {
        res += bitset[i] ? "1" : "0";
    }
    return res;
}

//gates

namespace gate_types {
    enum _types {
        toffoli,
        peres,
        fredkin,
        v,
        vplus,
        module
    };
}

void gate_set_type(gate& g, unsigned value) {
    switch (value) {
        case gate_types::toffoli:
            g.set_type(toffoli_tag());
            break;
        case gate_types::peres:
            g.set_type(peres_tag());
            break;
        case gate_types::fredkin:
            g.set_type(fredkin_tag());
            break;
        case gate_types::v:
            g.set_type(v_tag());
            break;
        case gate_types::vplus:
            g.set_type(vplus_tag());
            break;
        case gate_types::module:
            g.set_type(module_tag());
            break;
        default:
            assert(false);
            break;
    }
}

unsigned gate_get_type(const gate& g) {
    if (is_toffoli(g)) {
        return gate_types::toffoli;
    } else if (is_peres(g)) {
        return gate_types::peres;
    } else if (is_fredkin(g)) {
        return gate_types::fredkin;
    } else if (is_v(g)) {
        return gate_types::v;
    } else if (is_vplus(g)) {
        return gate_types::vplus;
    } else if (is_module(g)) {
        return gate_types::module;
    }

    assert(false);
}

std::string gate_module_name(const gate& g) {
    if (is_module(g)) {
        return boost::any_cast<module_tag>(g.type()).name;
    } else {
        return std::string();
    }
}

/* control and target lines */
py::list control_lines1(const gate& g) {
    gate::line_container c;
    py::list             l;
    control_lines(g, std::insert_iterator<gate::line_container>(c, c.begin()));
    std::for_each(c.begin(), c.end(), std::bind(my_append<unsigned>, l, std::placeholders::_1));
    return l;
}

py::list target_lines1(const gate& g) {
    gate::line_container c;
    py::list             l;
    target_lines(g, std::insert_iterator<gate::line_container>(c, c.begin()));
    std::for_each(c.begin(), c.end(), std::bind(my_append<unsigned>, l, std::placeholders::_1));
    return l;
}

PYBIND11_MODULE(pysyrec, m) {
    //m.doc() = "pybind11 example plugin"; // optional module docstring

    m.def("py_syrec_synthesisi", &syrec_synthesis);
    m.def("py_simple_simulationi", static_cast<bool (*)(boost::dynamic_bitset<>&, const circuit&, const boost::dynamic_bitset<>&, properties::ptr, properties::ptr)>(simple_simulation));
    //m.def("py_syrec_synthesis_func", &syrec_synthesis_func);
    //m.def("py_syrec_synthesis", (bool (*)(circuit& circ, const syrec::program&, properties::ptr, properties::ptr)) &syrec_synthesis,"doc", pybind11::arg("circ"), //pybind11::arg("program"),pybind11::arg("settings")= properties::ptr(), pybind11::arg("statistics")= properties::ptr());
    //m.def("py_syrec_synthesis_func", (hdl_synthesis_func (*)(circuit& circ, const syrec::program&)) &syrec_synthesis_func);

    //m.def("foo", (std::string (*)(int, int) ) &foo, "doc", pybind11::arg("a"), pybind11::arg("b"));
    //m.def("foo", (std::string (*)(int, int, int) ) &foo, "doc", pybind11::arg("a"), pybind11::arg("b"), pybind11::arg("c"));
    //return m.ptr();

    py::class_<circuit, std::shared_ptr<circuit>>(m, "circuiti")
            .def(py::init<>())
            .def(py::init<unsigned>())

            .def_property("lines", &circuit::lines, &circuit::set_lines)
            .def_property_readonly("num_gates", &circuit::num_gates)
            .def("gates", new_gates)
            //.def_property_readonly( "rgates", boost::python::range<py::return_value_policy::reference>( rbegin1, rend1 ) )
            .def_property_readonly("inputs", inputs_getter)
            .def_property_readonly("outputs", outputs_getter)
            .def_property_readonly("constants", constants_getter)
            .def_property_readonly("garbage", garbage_getter)
            //.def_property( "circuit_name", make_function( &circuit::circuit_name, py::return_value_policy::copy ), &circuit::set_circuit_name )
            //.def_property_readonly( "filter", pybind_circuit_filter )
            //.def_property_readonly( "offset", &circuit::offset )

            //.def( "append_gate", circuit_append_gate )
            //.def( "prepend_gate", circuit_prepend_gate )
            //.def( "insert_gate", circuit_insert_gate )
            //.def( "remove_gate_at", &circuit::remove_gate_at )
            //.def( "is_subcircuit", &circuit::is_subcircuit )
            //.def( "inputbuses", inputbuses1, py::return_value_policy::reference_internal )
            //.def( "outputbuses", outputbuses1, py::return_value_policy::reference_internal )
            //.def( "statesignals", statesignals1, py::return_value_policy::reference_internal )
            //.def( "add_module", add_module1 )
            //.def( "modules", circuit_modules )
            //.def( "annotation", &circuit::annotation, py::return_value_policy::copy)
            .def("annotations", circuit_annotations)
            //.def( "annotate", &circuit::annotate )

            //.def( boost::python::self_ns::str( self ) )
            //.def( boost::python::self_ns::repr( self ) )
            //.def( "__getitem__", circuit_get_gate, py::return_value_policy::reference )
            //.def( "__iter__", boost::python::range<py::return_value_policy::reference >( begin1, end1 ) )
            ;
    //m.def( "subcircuit", subcircuit1 );
    //m.def( "subcircuit", subcircuit2 );

    m.def("add_new", &add_new, "A function that adds two numbers");

    py::class_<properties, std::shared_ptr<properties>>(m, "propertiesi")
            .def(py::init<>())
            .def("set_string", &properties_set<std::string>)
            .def("set_bool", &properties_set<bool>)
            .def("set_int", &properties_set<int>)
            .def("set_unsigned", &properties_set<unsigned>)
            .def("set_double", &properties_set<double>)
            .def("set_char", &properties_set<char>)
            //.def( "set_bitset_map", &properties_set_bitset_map )
            //.def( "set_cost_function", &properties_set_cost_function<costs_by_circuit_func> )
            //.def( "set_cost_function", &properties_set_cost_function<costs_by_gate_func> )
            /*.def( "set_truth_table_synthesis_func", &properties_set<truth_table_synthesis_func> )
    .def( "set_gate_decomposition_func", &properties_set<gate_decomposition_func> )
    .def( "set_swop_step_func", &properties_set<swop_step_func> )
    .def( "set_simulation_func", &properties_set<simulation_func> )
    .def( "set_window_synthesis_func", &properties_set<window_synthesis_func> )
    .def( "set_optimization_func", &properties_set<optimization_func> )
    .def( "set_select_window_func", &properties_set<select_window_func> )
    .def( "set_step_result_func", &properties_set<step_result_func> )
    .def( "set_cube_reordering_func", &properties_set<cube_reordering_func> )*/
            //.def( "set_vector_unsigned", &properties_set_vector<unsigned> )
            //.def( "set_line_mapping", &properties_set_line_mapping )

            .def("get_string", &properties_get1<std::string>)
            //.def( "get_bool", &properties_get1<bool> )
            //.def( "get_int", &properties_get1<int> )
            //.def( "get_unsigned", &properties_get1<unsigned> )
            .def("get_double", &properties_get1<double>)
            //.def( "get_char", &properties_get1<char> )
            //.def( "get_bitset_map", &properties_get1<std::map<std::string, boost::dynamic_bitset<> > > )
            /*.def( "get_truth_table_synthesis_func", &properties_get1<truth_table_synthesis_func> )
    .def( "get_gate_decomposition_func", &properties_get1<gate_decomposition_func> )
    .def( "get_swop_step_func", &properties_get1<swop_step_func> )
    .def( "get_simulation_func", &properties_get1<simulation_func> )
    .def( "get_window_synthesis_func", &properties_get1<window_synthesis_func> )
    .def( "get_optimization_func", &properties_get1<optimization_func> )
    .def( "get_select_window_func", &properties_get1<select_window_func> )
    .def( "get_step_result_func", &properties_get1<step_result_func> )
    .def( "get_cube_reordering_func", &properties_get1<cube_reordering_func> )*/
            //.def( "get_vector_unsigned", &properties_get_vector1<unsigned> )
            //.def( "get_counterexample", &properties_get_counterexample )

            //.def( "get_string", &properties_get2<std::string> )
            //.def( "get_bool", &properties_get2<bool> )
            //.def( "get_int", &properties_get2<int> )
            //.def( "get_unsigned", &properties_get2<unsigned> )
            //.def( "get_double", &properties_get2<double> )
            //.def( "get_char", &properties_get2<char> )
            //.def( "get_bitset_map", &properties_get2<std::map<std::string, boost::dynamic_bitset<> > > )
            /*.def( "get_truth_table_synthesis_func", &properties_get2<truth_table_synthesis_func> )
    .def( "get_gate_decomposition_func", &properties_get2<gate_decomposition_func> )
    .def( "get_swop_step_func", &properties_get2<swop_step_func> )
    .def( "get_simulation_func", &properties_get2<simulation_func> )
    .def( "get_window_synthesis_func", &properties_get2<window_synthesis_func> )
    .def( "get_optimization_func", &properties_get2<optimization_func> )
    //.def( "get_select_window_func", &properties_get2<select_window_func> )
    //.def( "get_step_result_func", &properties_get2<step_result_func> )
    .def( "get_cube_reordering_func", &properties_get2<cube_reordering_func> )*/
            //.def( "get_vector_unsigned", &properties_get_vector2<unsigned> )
            ;

    py::class_<program>(m, "syrec_programi")
            .def(py::init<>())
            //.def( boost::python::self_ns::str( self ) )
            //.def( boost::python::self_ns::repr( self ) )
            .def("add_module", &program::add_module);

    m.def("py_read_programi", py_read_program);

    py::class_<read_program_settings>(m, "read_program_settingsi")
            .def(py::init<>())
            .def_readwrite("default_bitwidth", &read_program_settings::default_bitwidth);

    m.def("quantum_costsi", final_quantum_cost);
    m.def("transistor_costsi", final_transistor_cost);

    //m.def( "py_costsi", costs1 );
    //m.def( "py_costsi", costs2 );

    py::class_<boost::dynamic_bitset<>>(m, "bitseti")
            .def(py::init<>())
            .def(py::init<int>())
            .def(py::init<int, unsigned long>())
            .def("__str__", bitset_to_string);

    py::enum_<gate_types::_types>(m, "gate_typei")
            .value("toffoli", gate_types::toffoli)
            .value("peres", gate_types::peres)
            .value("fredkin", gate_types::fredkin)
            .value("v", gate_types::v)
            .value("vplus", gate_types::vplus)
            .value("module", gate_types::module)
            .export_values();

    py::class_<gate>(m, "gatei")
            .def(py::init<>())
            .def_property("type", gate_get_type, gate_set_type)
            .def_property_readonly("module_name", gate_module_name)

            ;

    py::class_<dum::Dummy>(m, "Dummy")
            .def(py::init<>())
            .def(py::init<double>())
            .def("setVal", &dum::Dummy::setVal)
            .def("getVal", &dum::Dummy::getVal);

    m.def("control_linesi", control_lines1);
    m.def("target_linesi", target_lines1);
}
