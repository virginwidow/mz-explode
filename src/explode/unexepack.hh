#ifndef __EXPLODE_UNEXEPACK_HH__
#define __EXPLODE_UNEXEPACK_HH__

#include <stdint.h>
#include <stddef.h>

namespace explode
{
	class input_exe_file;
	class output_exe_file;
	class input;

	class unexepack
	{
	public:
		unexepack(input_exe_file& inp);

		void unpack(output_exe_file& oexe);
		uint32_t decomp_size() const;
	private:
		input&          m_file;
		input_exe_file& m_exe_file;
	};
} // ns explode

#endif
