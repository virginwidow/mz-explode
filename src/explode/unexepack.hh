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
		explicit unexepack(input_exe_file& inp);

		void unpack(output_exe_file& oexe);
		uint32_t decomp_size() const;
	private:
		enum header_t
		{
			eREAL_START_OFFSET,
			eREAL_START_SEGMENT,
			eMEM_START_SEGMENT,
			eUNPACKER_LEN,
			eREAL_STACK_OFFSET,
			eREAL_STACK_SEGMENT,
			eDEST_LEN,
			eSKIP_LEN,
			eSIGNATURE,

			eMAX_HEADER_VAL
		};
	private:
		input&          m_file;
		input_exe_file& m_exe_file;

		uint16_t m_header[eMAX_HEADER_VAL];

		uint32_t m_exe_data_start;
		uint32_t m_extra_data_start;
		uint32_t m_packed_data_len;

	};
} // ns explode

#endif
