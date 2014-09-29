#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <assert.h>

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
static void dump_exe_parameters (std::ostream& os, const explode::exe_file& header)
{
  using namespace explode;

  const uint32_t exe_size = 512 * (header [exe_file::NUM_OF_PAGES]-1) + header [exe_file::NUM_OF_BYTES_IN_LAST_PAGE];
  
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

enum dump_mode_t
{
	eDUMP_EXTRA,
	eDUMP_RELOC,
	eDUMP_CODE,
	eDUMP_NONE
};
// --------------------------------------------------------------------
static void dump_extra(const explode::exe_file& header, const char* ofile, explode::input& input)
{
	const explode::offset_type end_of_header  = explode::exe_file::MAX_HEADER_VAL * sizeof(uint16_t);
	const explode::offset_type rellocs_offset = header[explode::exe_file::RELLOC_OFFSET];
	if (rellocs_offset < end_of_header)
	{
		std::ostringstream os;
		os << "rellocs_offset = " << rellocs_offset << " < end_of_header = " << end_of_header;
		throw std::runtime_error(os.str());
	}
	if (rellocs_offset == end_of_header)
	{
		std::cout << "No extra information found" << std::endl;
		return;
	}
	const std::size_t sz = rellocs_offset - end_of_header;
	input.seek(end_of_header);
	std::vector <char> extra(sz);
	input.read(extra);
	explode::file_output output(ofile);

	output.write(&extra[0], extra.size());
	std::cout << "Extra information has been saved to " << ofile << " (" << sz << " bytes)" << std::endl;
}
// --------------------------------------------------------------------
static void dump_rellocs(const explode::exe_file& header, const char* ofile, explode::input& input)
{
	const explode::offset_type end_of_mz_header = header[explode::exe_file::HEADER_SIZE_PARA] * 16;
	if (header[explode::exe_file::RELLOCATION_ENTRIES] == 0)
	{
		std::cout << "No rellocation entries found" << std::endl;
	}
	const explode::offset_type rellocs_offset = header[explode::exe_file::RELLOC_OFFSET];
	if (rellocs_offset >= end_of_mz_header)
	{
		std::ostringstream os;
		os << "rellocs_offset = " << rellocs_offset << " >= end_of_header = " << end_of_mz_header;
		throw std::runtime_error(os.str());
	}
	
	const std::size_t sz = end_of_mz_header - rellocs_offset;
	input.seek(rellocs_offset);
	std::vector <char> extra(sz);
	input.read(extra);
	explode::file_output output(ofile);

	output.write(&extra[0], extra.size ());
	std::cout << "Rellocations has been saved to " << ofile << " (" << sz << " bytes)" << std::endl;
}
// --------------------------------------------------------------------
static void dump_code(const explode::exe_file& header, const char* ofile, explode::input& input)
{
	const explode::offset_type end_of_mz_header = header[explode::exe_file::HEADER_SIZE_PARA] * 16;
	
	input.seek(end_of_mz_header);

	const std::size_t sz = input.bytes_remains ();
	
	std::vector <char> extra(sz);
	input.read(extra);
	explode::file_output output(ofile);

	output.write(&extra[0], extra.size ());
	std::cout << "Code has been saved to " << ofile << " (" << sz << " bytes)" << std::endl;
}
// --------------------------------------------------------------------
int main (int argc, char* argv [])
{
  if (argc != 4)
    {
	  std::cerr << "USAGE: " << argv[0] << "<-e|-r|-c> <input> <output>" << std::endl
		  << "\t-e : dump extra information" << std::endl
		  << "\t-r : dump rellocation table" << std::endl
		  << "\t-c : dump code" << std::endl;
      return 1;
    }

  const std::string s_mode = argv[1];

  dump_mode_t mode = eDUMP_NONE;
  if (s_mode == "-e") 
  {
	  mode = eDUMP_EXTRA;
  }
  else
  {
	  if (s_mode == "-r")
	  {
		  mode = eDUMP_RELOC;
	  }
	  else
	  {
		  if (s_mode == "-c")
		  {
			  mode = eDUMP_CODE;
		  }
	  }
  }

  if (mode == eDUMP_NONE)
  {
	  std::cerr << "Illegal option: " << s_mode << std::endl;
	  return 1;
  }

  const char* ifile = argv [2];
  const char* ofile = argv [3];
	
  try
    {
      explode::file_input input (ifile);
      explode::input_exe_file iexe (input);
	  dump_exe_parameters(std::cout, iexe);

	  input.seek(0);
	  switch (mode)
	  {
	  case eDUMP_CODE:
		  dump_code(iexe, ofile, input);
		  break;
	  case eDUMP_RELOC:
		  dump_rellocs(iexe, ofile, input);
		  break;
	  case eDUMP_EXTRA:
		  dump_extra(iexe, ofile, input);
		  break;
	  default:
		  assert(false);
	  }
    }
  catch (std::runtime_error& e)
    {
      std::cerr << "ERROR: " << e.what () << std::endl;
	  return 1;
    }

  return 0;
}

