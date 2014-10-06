#include <vector>
#include <sstream>
#include <iomanip>

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
		m_exe_data_start = inp [exe_file::HEADER_SIZE_PARA] * 16L;
		m_extra_data_start = inp[exe_file::NUM_OF_PAGES] * 512L;
		if (inp [exe_file::NUM_OF_BYTES_IN_LAST_PAGE])
		{
			m_extra_data_start -= (512 - inp [exe_file::NUM_OF_BYTES_IN_LAST_PAGE]);
		}
		m_packed_data_len = inp [exe_file::INITIAL_CS] * 0x10;// + exe_data_start;
		
		const offset_type exepack_hdr_start = m_exe_data_start + m_packed_data_len;
		m_file.seek(exepack_hdr_start);
		
		union
		{
			char* bytes;
			uint16_t* words;
		} u;
		u.words = m_header;
		m_file.read(u.bytes, sizeof(m_header));
		for (int i = 0; i < eMAX_HEADER_VAL; i++)
		{
			m_header[i] = byte_order::from_little_endian(m_header[i]);
		}
	}
	// ------------------------------------------------------------------
	void unexepack::unpack(output_exe_file& oexe)
	{
		std::vector <uint8_t> buffer(m_packed_data_len);
		m_file.seek(m_exe_data_start);
		m_file.read((char*)&buffer[0], m_packed_data_len);
		
		const std::size_t out_len = decomp_size ();
		oexe.code_set(0xFF, out_len);
		oexe.code_put(0, buffer);
		
		std::size_t dst_pos = out_len;
		std::size_t src_pos = m_packed_data_len - 1;
		std::size_t last_pos = src_pos;
		while (buffer[src_pos] == 0xFF)
		{
			src_pos--;
		}
		uint16_t length;
		uint8_t  fill_byte;
		uint8_t  cmd;

		do
		{
			cmd = buffer[src_pos--];
			
			switch (cmd & 0xFE)
			{
			case 0xb0:
				length = (((uint16_t)buffer[src_pos--]) & 0x00FF) * 0x100;
				length += (((uint16_t)buffer[src_pos--]) & 0x00FF);
				fill_byte = buffer[src_pos--];
				dst_pos -= length;
				oexe.code_fill(dst_pos-1, fill_byte, length);
				
				break;
			case 0xb2:
				length = (((uint16_t)buffer[src_pos--]) & 0x00FF) * 0x100;
				length += (((uint16_t)buffer[src_pos--]) & 0x00FF);
				
				dst_pos -= length;
				src_pos -= length;

				oexe.code_put(dst_pos, &buffer[src_pos], length);
				
				break;
			default:
				throw explode::decoder_error("Unknown command");
				break;
			}
		} while ((cmd & 1) != 1);
		static const int unpackerDataLen = 0x12;
		static const int unpackerLen = 0x105; /* size of unpacker code */
		static const int errLen = 0x16;

		int reloc_table_size = m_header [eUNPACKER_LEN] - errLen - unpackerLen - unpackerDataLen;
		int reloc_num_entries = (reloc_table_size - 16 * sizeof(unsigned short)) / 2;
		int reloc_table_full = reloc_num_entries * 2 * sizeof(unsigned short);

		int section = 0;
		int relocSize = 0;
		int pack_buffer = m_header[eUNPACKER_LEN] - unpackerDataLen;

		/*
		for (section = 0; section < 16; section++) {

			int num_entries = READ_WORD(pbuffer, p); p += 2;

			if (num_entries == 0) break;

			int k;
			for (k = 0; k < num_entries; k++) {

				int entry = READ_WORD(pbuffer, p); p += 2;

				unsigned short patchSegment = 0x1000 * section;
				unsigned short patchOffset = entry;

				WRITE_WORD(rout, relocSize, patchOffset); relocSize += 2;
				WRITE_WORD(rout, relocSize, patchSegment); relocSize += 2;

			}
		}
		*/
		oexe[exe_file::INITIAL_SP] = m_header[eREAL_STACK_SEGMENT];
		oexe[exe_file::INITIAL_SS] = m_header[eREAL_STACK_OFFSET];
		oexe[exe_file::INITIAL_CS] = m_header[eREAL_START_SEGMENT];
		oexe[exe_file::INITIAL_IP] = m_header[eREAL_START_OFFSET];

		oexe.eval_structures();
	}
	// ------------------------------------------------------------------
	uint32_t unexepack::decomp_size() const
	{
		return 16 * (uint32_t)m_header[eDEST_LEN];
	}
}
