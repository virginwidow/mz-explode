#include "explode/unexepack.hh"
#include "explode/exe_file.hh"
#include "explode/io.hh"
#include "explode/struct_reader.hh"
#include "explode/exceptions.hh"
#include "explode/byte_order.hh"

namespace explode
{
	unexepack::unexepack(input_exe_file& inp)
		: m_file(inp.file()),
		m_exe_file(inp)
	{

	}
	// ------------------------------------------------------------------
	void unexepack::unpack(output_exe_file& oexe)
	{
		
	}
	// ------------------------------------------------------------------
	uint32_t unexepack::decomp_size() const
	{
		return 0;
	}
}
