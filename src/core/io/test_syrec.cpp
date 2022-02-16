#include <iostream>
#include <core/io/print_circuit.hpp>
#include <core/io/print_statistics.hpp>
#include <core/io/write_realization.hpp>
#include <core/syrec/parser.hpp>
#include <core/syrec/program.hpp>
#include <core/utils/program_options.hpp>
#include <core/functions/flatten_circuit.hpp>

#include <algorithms/synthesis/syrec_synthesis.hpp>

using namespace revkit;

int main( int argc, char ** argv )
{
  std::string filename;
  unsigned crement_merge_line_count;
  unsigned if_realization;
  bool efficient_controls;
  bool modules_hierarchy;
  bool print_circ;

  revkit::program_options opts;

  opts.add_write_realization_option();
  opts.add_options()
    ( "filename", boost::program_options::value<std::string>( &filename ), "SyReC-Programm-Datei" )
    ( "crement_merge_line_count", boost::program_options::value<unsigned>( &crement_merge_line_count )->default_value( 4u ), "Crement Merge Line Count" )
    ( "if_realization", boost::program_options::value<unsigned>( &if_realization )->default_value( syrec_synthesis_if_realization_controlled ), "Type of IF realization\n  0: controlled\n  1: duplication" )
    ( "efficient_controls", boost::program_options::value<bool>( &efficient_controls )->default_value( true ), "use optimization for efficient controlled cascades\n  true: yes\n  false: no")
    ( "modules_hierarchy", boost::program_options::value<bool>( &modules_hierarchy )->default_value( false ), "create circuit modules for syrec modules" )
    ( "print_circuit", boost::program_options::value<bool>( &print_circ )->default_value( true ), "print resulting circuit" )
    ;
  opts.parse( argc, argv );
  if ( !opts.good() || !opts.is_set( "filename" ) )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  syrec::program prog;
  read_program_settings rp_settings;
  std::string error;
  if ( !read_program( prog, filename, rp_settings, &error ) )
  {
    std::cerr << "Error: " << error << std::endl;
    return 1;
  }

  std::cout << "Programm" << std::endl;
  std::cout << prog << std::endl;

  circuit circ, circflat;

  properties::ptr settings( new properties() );
  settings->set( "crement_merge_line_count", crement_merge_line_count );
  settings->set( "if_realization", if_realization );
  settings->set( "efficient_controls", efficient_controls );
  settings->set( "modules_hierarchy", modules_hierarchy );
  properties::ptr statistics( new properties() );

  syrec_synthesis( circ, prog, settings, statistics);
  flatten_circuit( circ, circflat, false );

  if ( print_circ )
  {
    print_circuit_settings pc_settings;
    pc_settings.print_inputs_and_outputs = true;
    print_circuit( circflat, pc_settings );
  }

  print_statistics( circflat );
  std::cout << "Run-time: " << statistics->get<double>( "runtime" ) << std::endl;

  if ( opts.is_write_realization_filename_set() )
  {
    if ( !write_realization( circflat, opts.write_realization_filename(), write_realization_settings(), &error ) )
    {
      std::cerr << "Error: " << error << std::endl;
      return 1;
    }
  }

  return 0;
}
