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
	eCOMPARE,
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
	const std::size_t sz = static_cast <std::size_t> (rellocs_offset - end_of_header);
	input.seek(end_of_header);
	std::vector <char> extra(sz);
	input.read_buff (&extra [0], sz);
	explode::file_output output(ofile);
	if (!extra.empty ())
	  {
	    output.write_buff (&extra[0], extra.size());
	  }
	std::cout << "Extra information has been saved to " << ofile << " (" << sz << " bytes)" << std::endl;
}
// --------------------------------------------------------------------
static void load_rellocs(const explode::exe_file& header, explode::input& input, std::vector <char>& out)
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

	const std::size_t sz = header[explode::exe_file::RELLOCATION_ENTRIES]*4;
	if (sz)
	{
		input.seek(rellocs_offset);
		out.resize(sz);
		input.read_buff (&out[0], sz);
	}
}
// --------------------------------------------------------------------
static void dump_rellocs(const explode::exe_file& header, const char* ofile, explode::input& input)
{
	std::vector <char> rels;
	load_rellocs(header, input, rels);
	explode::file_output output(ofile);

	output.write_buff (&rels[0], rels.size ());
	std::cout << "Rellocations has been saved to " << ofile << " (" << rels.size () << " bytes)" << std::endl;
}
// --------------------------------------------------------------------
static bool transform_rellocs(const std::vector <char> raw, std::vector <explode::rellocation>& out)
{
	const std::size_t sz = raw.size();
	if (sz % 4)
	{
		std::cout << "rellocations area size should be divisible by 4" << std::endl;
		return false;
	}
	for (std::size_t i = 0; i < sz / 4; i++)
	{
		const char* rel_b = &raw[4 * i];
		const char* seg_b = &raw[4 * i + 2];
		union
		{
			const uint16_t* words;
			const char*     bytes;
		} u;
		u.bytes = rel_b;
		uint16_t rel = *u.words;
		u.bytes = seg_b;
		uint16_t seg = *u.words;
		out.push_back(explode::rellocation(seg, rel));
	}
	return true;
}
// --------------------------------------------------------------------
static bool compare_rellocs(explode::input_exe_file& iexe1, explode::input_exe_file& iexe2)
{
	std::cout << "Rellocations table check ";

	std::vector <char> raw_rellocs1;
	std::vector <char> raw_rellocs2;
	load_rellocs(iexe1, iexe1.file(), raw_rellocs1);
	load_rellocs(iexe2, iexe2.file(), raw_rellocs2);

	std::vector <explode::rellocation> rel1;
	std::vector <explode::rellocation> rel2;
	
	if (!transform_rellocs(raw_rellocs2, rel2) || !transform_rellocs(raw_rellocs1, rel1))
	{
		return false;
	}

	if (rel1.size() != rel2.size())
	{
		std::cout << "Rellocation tables size differs " << rel1.size() << ", " << rel2.size() << std::endl;
		return false;
	}
	bool first = true;
	for (std::size_t i = 0; i < rel1.size(); i++)
	{
		const explode::rellocation& r1 = rel1[i];
		const explode::rellocation& r2 = rel2[i];

		if (r1.seg != r2.seg || r1.rel != r2.rel)
		{
			if (first)
			{
				std::cout << "Rellocation table entries differs" << std::endl;
				first = false;
			}
			std::cout << "Entry " << i << " (" << std::setw(4) << std::setfill('0') << std::hex << r1.seg << ","
				<< std::setw(4) << std::setfill('0') << std::hex << r1.rel << ") != ("
				<< std::setw(4) << std::setfill('0') << std::hex << r2.seg << ","
				<< std::setw(4) << std::setfill('0') << std::hex << r2.rel << ")" << std::dec << std::endl;

		}
	}
	if (first == false)
	{
		return false;
	}
	std::cout << "OK" << std::endl;
	return true;
}
// --------------------------------------------------------------------
static void load_code(const explode::exe_file& header, explode::input& input, std::vector <char>& out)
{
	const explode::offset_type end_of_mz_header = header[explode::exe_file::HEADER_SIZE_PARA] * 16;
	input.seek(end_of_mz_header);
	const std::size_t sz = static_cast <std::size_t> (input.bytes_remains());
	out.resize(sz);
	input.read_buff(&out[0], sz);
}
// --------------------------------------------------------------------
static void dump_code(const explode::exe_file& header, const char* ofile, explode::input& input)
{
	std::vector <char> code;
	load_code(header, input, code);
	explode::file_output output(ofile);
	output.write_buff(&code[0], code.size ());
	std::cout << "Code has been saved to " << ofile << " (" << code.size () << " bytes)" << std::endl;
}
// --------------------------------------------------------------------
static bool compare_code(explode::input_exe_file& iexe1, explode::input_exe_file& iexe2)
{
	std::cout << "Code check ";
	std::vector <char> code1;
	std::vector <char> code2;
	load_code(iexe1, iexe1.file(), code1);
	load_code(iexe2, iexe2.file(), code2);
	const std::size_t sz = code1.size();
	if (code2.size() != sz)
	{
		std::cout << "Different sizes: " << sz << ", " << code2.size() << std::endl;
		return false;
	}
	bool first = true;
	for (std::size_t i = 0; i < sz; i++)
	{
		if (code1[i] != code2[i])
		{
			if (first)
			{
				std::cout << "Different bytes" << std::endl;
				first = false;
			}
			std::cout << "offset " << i << ": " << (static_cast <int>(code1[i]) & 0xFF) << " != "
				<< (static_cast <int>(code2[i]) & 0xFF) << std::endl;
		}
	}
	if (first == false)
	{
		return false;
	}
	std::cout << " OK" << std::endl;
	return true;
}
// --------------------------------------------------------------------
static bool compare_headers(explode::input_exe_file& iexe1, explode::input_exe_file& iexe2)
{
	std::cout << "Header check ";
	bool ok = true;
	for (int i = 0; i < explode::exe_file::MAX_HEADER_VAL; i++)
	{
	  explode::exe_file::header_t h = static_cast <explode::exe_file::header_t> (i);

		if (iexe1[h] != iexe2[h])
		{
			ok = false;
			std::cout << std::endl;
			std::cout << "DIFF: (" << h << ") " << iexe1[h] << " : " << iexe2[h] << std::endl;
			
		}
	}
	if (!ok)
	{
		return false;
	}
	std::cout << " OK" << std::endl;
	return true;
}
// --------------------------------------------------------------------
static void compare_files(const char* file1, const char* file2)
{
	explode::file_input inp1 (file1);
	explode::file_input inp2 (file2);

	if (inp1.bytes_remains() != inp2.bytes_remains())
	{
		std::cerr << "File size differs" << std::endl;
		return;
	}
	explode::input_exe_file iexe1 (inp1);
	explode::input_exe_file iexe2 (inp2);

	if (!compare_headers(iexe1, iexe2))
	{
		return;
	}
	if (!compare_rellocs(iexe1, iexe2))
	{
		return;
	}
	if (!compare_code(iexe1, iexe2))
	{
		return;
	}
}
// --------------------------------------------------------------------
int main (int argc, char* argv [])
{
  if (argc != 4)
    {
	  std::cerr << "USAGE: " << argv[0] << "<-e|-r|-c> <input> <output>" << std::endl
		  << "\t-e : dump extra information" << std::endl
		  << "\t-r : dump rellocation table" << std::endl
		  << "\t-c : dump code" << std::endl
		  << "\t-m : intelligent compare" << std::endl;
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
		  else 
		  {
			  if (s_mode == "-m")
			  {
				  mode = eCOMPARE;
			  }
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
	  if (mode != eCOMPARE)
	  {
		  dump_exe_parameters(std::cout, iexe);
	  }

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
	  case eCOMPARE:
		  compare_files(ifile, ofile);
		  break;
	  case eDUMP_NONE:
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

