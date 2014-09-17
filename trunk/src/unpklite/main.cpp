#include <stdexcept>
#include <iostream>
#include <iomanip>
#include "explode/io.hh"
#include "explode/exe_file.hh"
#include "explode/unpklite.hh"

// ===================================================================
template <typename T>
static void dump_info (std::ostream& os, const char* name, T v)
{
  os << std::left << std::setw (32) << name << ":\t" << std::hex << v << "\t" << std::dec << v << std::endl;
}
// ---------------------------------------------------------------------------------------------------------
static void dump_info (std::ostream& os, const char* name, uint16_t seg, uint16_t offs, bool addr = true)
{
  if (addr)
    {
      os << std::left << std::setw (32) << name << ":\t" << std::hex << seg << ":" << std::hex << offs << std::endl;
    }
  else
    {
      os << std::left << std::setw (32) << name << ":\t" << std::dec << seg << "." << offs << std::endl;
    }
}
// ---------------------------------------------------------------------------------------------------------
static void dump_info (std::ostream& os, const char* name, const char* txt)
{
  os << std::left << std::setw (32) << name << ":\t" << txt << std::endl;
}
// ---------------------------------------------------------------------------------------------------------
static void dump_exe_parameters (std::ostream& os, 
				 const char* ifile, 
				 const explode::exe_file& header, 
				 const explode::unpklite& decoder)
{
  using namespace explode;

  const uint32_t exe_size = 512 * (header [exe_file::NUM_OF_PAGES]-1) + header [exe_file::NUM_OF_BYTES_IN_LAST_PAGE];
  
  dump_info (os, "Input file", ifile);
  dump_info (os, ".EXE size (bytes)", exe_size);
  dump_info (os, "Initial CS:IP", header [exe_file::INITIAL_CS], header [exe_file::INITIAL_IP]);
  dump_info (os, "Initial SS:SP", header [exe_file::INITIAL_SS], header [exe_file::INITIAL_SP]);
  dump_info (os, "Minimum allocation (para)", header [exe_file::MIN_MEM_PARA]);
  dump_info (os, "Maximum allocation (para)", header [exe_file::MAX_MEM_PARA]);
  dump_info (os, "Header Size (para)", header [exe_file::HEADER_SIZE_PARA]);
  dump_info (os, "Relocation table offset", header [exe_file::RELLOC_OFFSET]);
  dump_info (os, "Relocation entries", header [exe_file::RELLOCATION_ENTRIES]);

  const uint16_t PKLITE_INFO = decoder.pklite_info ();

  const uint16_t pklite_ver_minor = PKLITE_INFO & 0xFF;
  const uint16_t pklite_ver_major = (PKLITE_INFO & 0x0F00) >> 8;
  
  dump_info (os, "PKLITE version", pklite_ver_major, pklite_ver_minor, false);


  const uint16_t pklite_method = PKLITE_INFO & 0x1000;

  const char* s_pklite_method = (pklite_method == 0) ? "Standard" : "Extra";
  dump_info (os, "Compression Technique", s_pklite_method);

  const uint16_t pklite_compression_model = PKLITE_INFO & 0x2000;


  const char* s_pklite_compression_model = (pklite_compression_model == 0) ? "Small .EXE" : "Large .EXE";
  dump_info (os, "Compression Model", s_pklite_compression_model);


  if (decoder.uncompressed_region ())
    {
      dump_info (os, "PKLite -g Uncompressed Region", "true");
    }
  else
    {
      dump_info (os, "PKLite -g Uncompressed Region", "false");
    }

  if (decoder.has_checksum ())
    {
      dump_info (os, "PKLite -c Image Checksum", "true");
    }
  else
    {
      dump_info (os, "PKLite -c Image Checksum", "false");
    }

  dump_info (os, "Compressed image size (bytes)"  , decoder.compressed_size ());
  dump_info (os, "Decompressor size (bytes)"      , decoder.decompressor_size ());
  dump_info (os, "Decompressed image size (bytes)", decoder.decomp_size ());
  dump_info (os, "Offset to compressed image"     , decoder.data_offset ());
}
// ---------------------------------------------------------------------------------------------------------
static void dump_exe_parameters (std::ostream& os, 
				 const char* ofile, 
				 const explode::exe_file& header)
{
  using namespace explode;

  const uint32_t exe_size = 512 * (header [exe_file::NUM_OF_PAGES]-1) + header [exe_file::NUM_OF_BYTES_IN_LAST_PAGE];
  
  dump_info (os, "Output file", ofile);
  dump_info (os, ".EXE size (bytes)", exe_size);
  dump_info (os, "Initial CS:IP", header [exe_file::INITIAL_CS], header [exe_file::INITIAL_IP]);
  dump_info (os, "Initial SS:SP", header [exe_file::INITIAL_SS], header [exe_file::INITIAL_SP]);
  dump_info (os, "Minimum allocation (para)", header [exe_file::MIN_MEM_PARA]);
  dump_info (os, "Maximum allocation (para)", header [exe_file::MAX_MEM_PARA]);
  dump_info (os, "Header Size (para)", header [exe_file::HEADER_SIZE_PARA]);
  dump_info (os, "Relocation table offset", header [exe_file::RELLOC_OFFSET]);
  dump_info (os, "Relocation entries", header [exe_file::RELLOCATION_ENTRIES]);
}
// ===================================================================

int main (int argc, char* argv [])
{
  if (argc != 3)
    {
      std::cerr << "USAGE: " << argv [0] << " <input> <output>" << std::endl;
      return 1;
    }
  const char* ifile = argv [1];
  const char* ofile = argv [2];

  try
    {
      explode::file_input input (ifile);
      explode::input_exe_file iexe (input);
      if (iexe.is_pklite ())
	{
	  explode::unpklite decoder (iexe);
	  dump_exe_parameters (std::cout, ifile, iexe, decoder);

	  explode::full_exe_file fo (decoder.decomp_size ());
	  decoder.unpak (fo);
	  std::cout << std::endl;
	  dump_exe_parameters (std::cout, ofile, fo);
	  explode::file_output ow (ofile);
	  fo.write (ow);
	}
      else
	{
	  std::cerr << "Not a PKLITE compressed file" << std::endl;
	}
    }
  catch (std::runtime_error& e)
    {
      std::cerr << "ERROR: " << e.what () << std::endl;
    }

  return 0;
}

