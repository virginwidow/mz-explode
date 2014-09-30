#include <vector>
#include <cstring>
#include <iostream>
#include "explode/unlzexe.hh"
#include "explode/exe_file.hh"
#include "explode/io.hh"
#include "explode/struct_reader.hh"
#include "explode/exceptions.hh"
#include "explode/byte_order.hh"

static void build_rellocs_90 (explode::input& file, std::vector <uint32_t>& rellocs)
{
  uint16_t seg = 0;
  do
    {
      uint16_t t;
      file.read (t);
      int c = t & 0xFFFF;

      for (; c>0; c--)
	{
	  uint16_t offs;
	  file.read (offs);
	  uint32_t x = seg;
	  x <<= 16;
	  x |= offs;
	  rellocs.push_back (x);
	}
      seg += 0x1000;
    } while (seg != (0xF000+0x1000));
}
// ----------------------------------------------------------------
static void build_rellocs_91 (explode::input& file, std::vector <uint32_t>& rellocs)
{
  uint16_t seg  = 0;
  uint16_t offs = 0;
  uint16_t span = 0;
  while (true)
    {
      uint8_t s;
      file.read (s);
      span = s & 0xFF;
      if (span == 0)
	{
	  seg += 0x0FFF;
	  continue;
	}
      else
	{
	  if (span == 1)
	    {
	      break;
	    }
	}
      offs += span;
      seg += (offs & ~0x0f)>>4;
      offs &= 0x0f;
      uint32_t x = seg;
      x <<= 16;
      x |= offs;
      rellocs.push_back (x);
    };
}

static uint32_t unpak_code(explode::output_exe_file& oexe, explode::input& input, uint32_t offset)
{
	input.seek(offset);
	explode::bit_reader bitstream(input);

	uint8_t data[0x4500], *p = data;
	std::size_t opos = 0;
	uint16_t len = 0;
	uint16_t span;

	while (true)
	{
		if (p - data >= 0x4000)
		{
			oexe.code_put(opos, data, 0x2000);
			opos += 0x2000;
			std::memcpy(data, data + 0x2000, p - data);
		}
		if (bitstream.bit())
		{
			const uint8_t x = bitstream.byte();
			*p++ = x;
			continue;
		}

		if (!bitstream.bit())
		{
			len = bitstream.bit() << 1;
			len |= bitstream.bit();
			len += 2;
			span = ((uint16_t)bitstream.byte() & 0xFFFF) | 0xFF00;
		}
		else
		{
			span = (uint8_t)((uint16_t)bitstream.byte() & 0xFFFF);
			len = (uint16_t)bitstream.byte() & 0xFF;
			span |= ((len & ~0x07) << 5) | 0xe000;
			len = (len & 0x07) + 2;
			if (len == 2)
			{
				len = (uint16_t)bitstream.byte() & 0xFF;
				if (len == 0)
				{
					break;
				}
				if (len == 1)
				{
					continue;
				}
				else
				{
					len++;
				}
			}
		}
		for (; len>0; len--, p++)
		{
			*p = *(p + span);
		}
	}
	if (p != data)
	{
		oexe.code_put(opos, data, p-data);
		opos += (p-data);
	}

	return opos;
}
// ----------------------------------------------------------------
namespace explode
{
  unlzexe::unlzexe (input_exe_file& inp)
    : m_file (inp.file ()),
      m_exe_file (inp),
      m_ver (0)
  {
    static const offset_type magic_offs = 0x0E;
    
	union
	{
		char bytes[4];
		uint32_t word;
	} magic;

	magic.word = 0;

    m_file.seek (magic_offs);
    m_file.read (magic.bytes, 4);

	magic.word = byte_order::from_little_endian(magic.word);

    if (std::memcmp (magic.bytes, "LZ09", 4) == 0)
      {
		m_ver = 90;
      }
    else
      {
	if (std::memcmp (magic.bytes, "LZ91", 4) == 0)
	  {
	    m_ver = 91;
	  }
	else
	  {
	    throw decoder_error ("Unsuported version");
	  }
      }
    const offset_type header_pos = (inp [exe_file::HEADER_SIZE_PARA] + (inp [exe_file::INITIAL_CS] << 4));
    m_file.seek (header_pos);
    union
    {
      char* bytes;
      uint16_t* words;
    } u;
    u.words = m_header;
    m_file.read (u.bytes, sizeof (m_header));

	for (int i = 0; i < eHEADER_MAX; i++)
	{
		m_header[i] = byte_order::from_little_endian(m_header[i]);
	}

    if (m_ver == 90)
      {
		m_rellocs_offset = header_pos + 0x19D;
      }
    else
      {
		m_rellocs_offset = header_pos + 0x158;
      }
	m_code_offset = ((uint32_t)inp[exe_file::INITIAL_CS] - (uint32_t)m_header[eCOMPRESSED_SIZE] +
		(uint32_t)inp[exe_file::HEADER_SIZE_PARA]) << 4;
  }
  // ------------------------------------------------------------------------
  uint32_t unlzexe::unpak (output_exe_file& oexe)
  {
    m_file.seek (m_rellocs_offset);
    if (m_ver == 90)
      {
		build_rellocs_90 (m_file, oexe.rellocations ());
      }
    else
      {
		build_rellocs_91 (m_file, oexe.rellocations ());
      }
	
	const uint32_t load_size = unpak_code(oexe, m_file, m_code_offset);

    for (int i=0; i<exe_file::MAX_HEADER_VAL; i++)
      {
		const exe_file::header_t v = static_cast <exe_file::header_t> (i);
		oexe [v] = m_exe_file [v];
      }
    oexe [exe_file::INITIAL_IP]    = m_header [eIP];
    oexe [exe_file::INITIAL_CS]    = m_header [eCS];
    oexe [exe_file::INITIAL_SS]    = m_header [eSS];
    oexe [exe_file::INITIAL_SP]    = m_header [eSP];
    oexe [exe_file::RELLOC_OFFSET] = 0x1C;

	if (m_exe_file[exe_file::MAX_MEM_PARA] != 0)
	{

	}

    offset_type orig_data_offs = 0xE;
    m_file.seek (orig_data_offs);
    for (; orig_data_offs < 0xF; orig_data_offs++)
      {
	uint8_t b;
	m_file.read (b);
	oexe.extra_header ().push_back (b);
      }
	return load_size;
  }
} // ns explode
