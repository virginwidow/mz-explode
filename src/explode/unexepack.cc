#include <vector>
#include <sstream>

#include "explode/unexepack.hh"
#include "explode/exe_file.hh"
#include "explode/io.hh"
#include "explode/struct_reader.hh"
#include "explode/exceptions.hh"
#include "explode/byte_order.hh"


namespace explode
{
	static const int unpackerDataLen = 0x12;
	static const int unpackerLen = 0x105; /* size of unpacker code */
	static const int errLen = 0x16;
	// ----------------------------------------------------------------------------------------
	unexepack::unexepack(input_exe_file& inp)
		: m_file(inp.file()),
		m_exe_file(inp)
	{
		const uint16_t ip = inp[exe_file::INITIAL_IP];
		if (ip != 0x10 && ip != 0x12 && ip != 0x14)
		{
			throw decoder_error ("unsupported version");
		}
		m_exe_data_start = inp[exe_file::HEADER_SIZE_PARA] * 16L;
		uint32_t var_2c = inp[exe_file::INITIAL_CS] * 16L;
		var_2c += m_exe_data_start;
		m_file.seek(var_2c);
		union
		{
			char* bytes;
			uint16_t* words;
		} u;
		u.words = m_header;
		m_file.read_buff (u.bytes, sizeof(m_header));
		for (int i = 0; i < eMAX_HEADER_VAL; i++)
		{
			m_header[i] = byte_order::from_little_endian(m_header[i]);
		}

		uint16_t sig_ptr = inp[exe_file::INITIAL_IP];
		sig_ptr += 0x0FFFE;
		sig_ptr = sig_ptr >> 1;

		uint16_t signature = u.words[sig_ptr];

		if (signature == 0x4252)
		{
			uint32_t len = 0;
			uint32_t var_36 = 0;
			// 10729
			if (inp[exe_file::INITIAL_IP] == 0x10)
			{
				// 10733
				uint16_t p = inp[exe_file::INITIAL_IP] >> 1;
				p = p << 1;
				uint16_t w = u.words[p];
				if (w == 0xe88b)
				{
					// 10750
					// second generation
					len = 0x132;
				}
				else
				{
					len = 0x125;
				}
			}
			if (inp[exe_file::INITIAL_IP] == 0x12)
			{
				len = 0x12d;
			}
			
			if (inp[exe_file::INITIAL_IP] == 0x14)
			{
				var_36 = 1;
				len = 0x131;
			}
			uint16_t lo = var_2c & 0xFFFF;
			uint16_t hi = (var_2c & 0xFFFF0000)>> 16;
			lo += (uint16_t)len;
			if (lo > 7)
			{
				hi++;
			}
			lo = lo + (uint16_t)0xFFF9;
			hi = hi + (uint16_t)0xFFFF;

			uint32_t p = ((uint32_t)hi << 16) | lo;
			m_file.seek(p);
			char ch;
			m_file.read_buff(&ch, 1);

			if (ch != 0x63)
			{
				throw decoder_error("not an exepack");
			}

			int x = 0;
		}
		// goto 107f5
		
		return;
	}

	void unexepack::unpack(output_exe_file& oexe)
	{
	}

	uint32_t unexepack::decomp_size() const
	{
		return 16 * (uint32_t)m_header[eDEST_LEN];
	}
#if 0
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
		
		m_rellocs_start = exepack_hdr_start + sizeof(m_header) + unpackerLen + errLen;

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

		const std::size_t out_len = decomp_size();
		oexe.code_set(0xFF, out_len);
		oexe.code_put(0, buffer);

		std::size_t dst_pos = out_len - 1;
		std::size_t src_pos = m_packed_data_len - 1;
		std::size_t last_pos = src_pos;

		std::size_t s = src_pos;
		std::size_t d = dst_pos;

		while (buffer[src_pos] == 0xFF)
		{
			src_pos--;
		}
		//printf("Start: %d\n", s - src_pos);
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
				//printf("fill: s:%d d:%d  ", s - src_pos, d - dst_pos);
				dst_pos -= length;
				oexe.code_fill(dst_pos, fill_byte, length);
				//printf("now: s:%d d:%d\n", s - src_pos, d - dst_pos);
				break;
			case 0xb2:
				length = (((uint16_t)buffer[src_pos--]) & 0x00FF) * 0x100;
				length += (((uint16_t)buffer[src_pos--]) & 0x00FF);
				//printf("copy: s:%d d:%d  ", s - src_pos, d - dst_pos);
				dst_pos -= length;
				src_pos -= length;
				oexe.code_put(dst_pos, &buffer[src_pos], length);
				//printf("now: s:%d d:%d\n", s - src_pos, d - dst_pos);
				break;
			default:
				throw explode::decoder_error("Unknown command");
				break;
			}
		} while ((cmd & 1) != 1);


		

		

		int reloc_table_size = m_header [eUNPACKER_LEN] - errLen - unpackerLen - unpackerDataLen;
		int reloc_num_entries = (reloc_table_size - 16 * sizeof(unsigned short)) / 2;
		int reloc_table_full = reloc_num_entries * 2 * sizeof(unsigned short);

		int section = 0;
		int relocSize = 0;
		int pack_buffer = m_header[eUNPACKER_LEN] - unpackerDataLen;

		byte_reader rdr(m_file);
		rdr.seek(m_rellocs_start);
		for (int i = 0; i < 16; i++)
		{
			uint16_t count = rdr.word ();
			
			if (count == 0)
			{
				break;
			}
			for (uint16_t j = 0; j < count; j++)
			{
				uint16_t entry = rdr.word ();
				oexe.rellocations().push_back(rellocation(i * 0x1000, entry));
			}
		}


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
#endif
}
