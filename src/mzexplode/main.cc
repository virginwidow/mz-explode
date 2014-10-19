#include <stdexcept>
#include <iostream>
#include <iomanip>
#include "explode/io.hh"
#include "explode/exe_file.hh"
#include "explode/unlzexe.hh"
#include "explode/unpklite.hh"
#include "explode/unexepack.hh"
#include "explode/knowledge_dynamics.hh"

#if defined(__clang__)
#define PRIVATE static
#else
#define PRIVATE
#endif
// ===================================================================
template <typename T>
void dump_info(std::ostream& os, const char* name, T v)
{
	os << std::left << std::setw(32) << name << ":\t" << std::hex << v << "\t" << std::dec << v << std::endl;
}
// ---------------------------------------------------------------------------------------------------------
PRIVATE void dump_info(std::ostream& os, const char* name, uint16_t seg, uint16_t offs, bool addr = true)
{
	if (addr)
	{
		os << std::left << std::setw(32) << name << ":\t" << std::hex << seg << ":" << std::hex << offs << std::endl;
	}
	else
	{
		os << std::left << std::setw(32) << name << ":\t" << std::dec << seg << "." << offs << std::endl;
	}
}
// ---------------------------------------------------------------------------------------------------------
PRIVATE void dump_info(std::ostream& os, const char* name, const char* txt)
{
	os << std::left << std::setw(32) << name << ":\t" << txt << std::endl;
}
// ---------------------------------------------------------------------------------------------------------
PRIVATE void dump_exe_parameters(std::ostream& os,
	const char* file,
	const explode::exe_file& header,
	bool is_input)
{
	using namespace explode;

	const uint32_t exe_size = 512 * (header[exe_file::NUM_OF_PAGES] - 1) + header[exe_file::NUM_OF_BYTES_IN_LAST_PAGE];
	if (is_input)
	{
		dump_info(os, "Input file", file);
	}
	else
	{
		dump_info(os, "Output file", file);
	}
	dump_info(os, ".EXE size (bytes)", exe_size);
	dump_info(os, "Initial CS:IP", header[exe_file::INITIAL_CS], header[exe_file::INITIAL_IP]);
	dump_info(os, "Initial SS:SP", header[exe_file::INITIAL_SS], header[exe_file::INITIAL_SP]);
	dump_info(os, "Minimum allocation (para)", header[exe_file::MIN_MEM_PARA]);
	dump_info(os, "Maximum allocation (para)", header[exe_file::MAX_MEM_PARA]);
	dump_info(os, "Header Size (para)", header[exe_file::HEADER_SIZE_PARA]);
	dump_info(os, "Relocation table offset", header[exe_file::RELLOC_OFFSET]);
	dump_info(os, "Relocation entries", header[exe_file::RELLOCATION_ENTRIES]);
}
// ---------------------------------------------------------------------------------------------------------
PRIVATE void dump_exe_parameters(std::ostream& os,
	const char* ifile,
	const explode::exe_file& header,
	const explode::unpklite& decoder)
{
	using namespace explode;

	dump_exe_parameters(os, ifile, header, true);
		
	dump_info(os, "PKLITE version", decoder.ver_major (), decoder.ver_minor (), false);

	const char* s_pklite_method = (!decoder.extended ()) ? "Standard" : "Extra";
	dump_info(os, "Compression Technique", s_pklite_method);

	const char* s_pklite_compression_model = (!decoder.large_exe ()) ? "Small .EXE" : "Large .EXE";
	dump_info(os, "Compression Model", s_pklite_compression_model);


	if (decoder.uncompressed_region())
	{
		dump_info(os, "PKLite -g Uncompressed Region", "true");
	}
	else
	{
		dump_info(os, "PKLite -g Uncompressed Region", "false");
	}

	if (decoder.has_checksum())
	{
		dump_info(os, "PKLite -c Image Checksum", "true");
	}
	else
	{
		dump_info(os, "PKLite -c Image Checksum", "false");
	}

	dump_info(os, "Compressed image size (bytes)", decoder.compressed_size());
	dump_info(os, "Decompressor size (bytes)", decoder.decompressor_size());
	dump_info(os, "Decompressed image size (bytes)", decoder.decomp_size());
	dump_info(os, "Offset to compressed image", decoder.data_offset());
}
// ---------------------------------------------------------------------------------------------------------
PRIVATE void dump_exe_parameters(std::ostream& os,
	const char* ifile,
	const explode::exe_file& header,
	const explode::unlzexe& /*decoder*/)
{
	dump_exe_parameters(os, ifile, header, true);
}
// ---------------------------------------------------------------------------------------------------------
PRIVATE void dump_exe_parameters(std::ostream& os,
	const char* ifile,
	const explode::exe_file& header,
	const explode::unexepack& /*decoder*/)
{
	dump_exe_parameters(os, ifile, header, true);
}
// -------------------------------------------------------------------
PRIVATE void dump_exe_parameters(std::ostream& os,
	const char* ifile,
	const explode::exe_file& header,
	const explode::knowledge_dynamics& /*decoder*/)
{
	dump_exe_parameters(os, ifile, header, true);
}
// -------------------------------------------------------------------
template <typename DECODER>
PRIVATE void decode(explode::input_exe_file& iexe, const char* ifile, const char* ofile)
{
	DECODER decoder(iexe);
	dump_exe_parameters(std::cout, ifile, dynamic_cast <explode::exe_file&> (iexe), decoder);
	explode::full_exe_file fo(decoder.decomp_size());
	decoder.unpack(fo);
	std::cout << std::endl;
	dump_exe_parameters(std::cout, ofile, fo, false);
	explode::file_output ow(ofile);
	fo.write(ow);
}
// ===================================================================

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cerr << "USAGE: " << argv[0] << " <input> <output>" << std::endl;
		return 1;
	}
	const char* ifile = argv[1];
	const char* ofile = argv[2];

	try
	{
		explode::file_input input(ifile);
		explode::input_exe_file iexe(input);
		if (explode::unlzexe::accept (iexe))
		{
			decode <explode::unlzexe>(iexe, ifile, ofile);
		}
		else
		{
			if (explode::unpklite::accept (iexe))
			{
				decode <explode::unpklite>(iexe, ifile, ofile);
			}
			else
			{
				if (iexe.is_exepack())
				{
					decode <explode::unexepack>(iexe, ifile, ofile);
				}
				else
				{
					if (explode::knowledge_dynamics::accept(iexe))
					{
						decode <explode::knowledge_dynamics>(iexe, ifile, ofile);
					}
					else
					{
						std::cerr << "Unsupported exe format" << std::endl;
						return 1;
					}
				}
			}
		}
	}
	catch (std::runtime_error& e)
	{
		std::cerr << "ERROR: " << e.what() << std::endl;
	}

	return 0;
}

