#ifndef __EXPLODE_KNOWLEDGE_DYNAMICS_HH__
#define __EXPLODE_KNOWLEDGE_DYNAMICS_HH__

#include <stdint.h>
#include <stddef.h>

#include "explode/exe_file.hh"

namespace explode
{
	class input_exe_file;
	class output_exe_file;
	class input;

	class knowledge_dynamics
	{
	public:
		explicit knowledge_dynamics(input_exe_file& inp);

		static bool accept(input_exe_file& inp);

		void unpack(output_exe_file& oexe);
		uint32_t decomp_size() const;
	private:
		input&          m_file;
		input_exe_file& m_exe_file;

		uint32_t m_expected_size;
		uint32_t m_code_offs;
		
		uint16_t m_header[exe_file::MAX_HEADER_VAL];


	};
} // ns explode

#endif
