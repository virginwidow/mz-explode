#include <vector>
#include <cstring>
#include <iostream>
#include "explode/unlzexe.hh"
#include "explode/exe_file.hh"
#include "explode/io.hh"
#include "explode/struct_reader.hh"
#include "explode/exceptions.hh"
#include "explode/byte_order.hh"

static void build_rellocs_90 (explode::input& file, std::vector <explode::rellocation>& rellocs)
{
  int16_t seg = 0;
  do
    {
      uint16_t t;
      file.read (t);
      int c = t & 0xFFFF;

      for (; c>0; c--)
	{
	  uint16_t offs;
	  file.read (offs);
	  offs = explode::byte_order::from_little_endian(offs);
	  rellocs.push_back (explode::rellocation (static_cast <uint16_t> (seg), offs));
	}
      seg = static_cast <int16_t> (seg + 0x1000);
    } while (seg != static_cast <int16_t>(0xF000+0x1000));
}
// ----------------------------------------------------------------
static void build_rellocs_91 (explode::input& file, std::vector <explode::rellocation>& rellocs)
{
  int16_t seg  = 0;
  int16_t offs = 0;
  int16_t span = 0;
  while (true)
    {
      uint8_t s;
      file.read (s);
      span = static_cast <int16_t> (static_cast <uint16_t> (s) & 0xFF);
      if (span == 0)
	{
	  file.read(span);
	  if (span == 0)
	    {
	      seg = static_cast <int16_t>(seg + 0x0FFF);
	      continue;
	    }
	  else
	    {
	      if (span == 1)
		{
		  break;
		}
	    }
	}
      offs = static_cast <int16_t>(offs + span);
      seg = static_cast <int16_t>(seg + static_cast <int16_t>((offs & ~0x0f)>>4));
      offs &= 0x0f;
      rellocs.push_back (explode::rellocation (static_cast <uint16_t> (seg), static_cast <uint16_t>(offs)));
    };
}

static uint32_t unpak_code(explode::output_exe_file& oexe, explode::input& input, uint32_t offset)
{
  input.seek(offset);
  explode::bit_reader bitstream(input);

  uint8_t data[0x4500], *p = data;
  std::size_t opos = 0;
  int16_t len = 0;
  int16_t span = 0;

  while (true)
    {
      if (p - data >= 0x4000)
	{
	  oexe.code_put(opos, data, 0x2000);
	  opos += 0x2000;
	  p -= 0x2000;
	  std::memmove (data, data + 0x2000, static_cast <std::size_t> (p - data));
	}
      if (bitstream.bit())
	{
	  const uint8_t x = bitstream.byte();
	  *p++ = x;
	  continue;
	}

      if (!bitstream.bit())
	{
	  len = static_cast <int16_t>(bitstream.bit() << 1);
	  len = static_cast <int16_t>(len | bitstream.bit());
	  len = static_cast <int16_t>(len + 2);
	  span = static_cast <int16_t>((static_cast <uint16_t>(bitstream.byte()) & 0xFFFF) | 0xFF00);
	}
      else
	{
	  span = static_cast<uint8_t>(static_cast<uint16_t> (bitstream.byte()) & 0xFFFF);
	  len = static_cast <int16_t>(static_cast <uint16_t>(bitstream.byte()) & 0xFF);
	  span = static_cast <int16_t>(span | static_cast<int16_t>(((len & ~0x07) << 5) | 0xe000));
	  len = static_cast<int16_t>((len & 0x07) + 2);
	  if (len == 2)
	    {
	      len = static_cast <int16_t> (static_cast <uint16_t> (bitstream.byte()) & 0xFF);
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
      const std::size_t sz = static_cast <std::size_t> (p-data);
      oexe.code_put(opos, data, sz);
      opos += sz;
    }

  return static_cast <uint32_t> (opos);
}
// ----------------------------------------------------------------
namespace explode
{
  unlzexe::unlzexe (input_exe_file& inp)
    : m_file (inp.file ()),
      m_exe_file (inp),
      m_ver (0)
  {
    static const offset_type magic_offs = 2*0x0E;
    
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
    const offset_type header_pos = (inp [exe_file::HEADER_SIZE_PARA] + inp [exe_file::INITIAL_CS]) << 4;
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
	m_rellocs_offset = static_cast <uint32_t>(header_pos + 0x19D);
      }
    else
      {
	m_rellocs_offset = static_cast <uint32_t>(header_pos + 0x158);
      }
    m_code_offset = (static_cast <uint32_t>(inp[exe_file::INITIAL_CS]) - 
		     static_cast <uint32_t>(m_header[eCOMPRESSED_SIZE]) +
		     static_cast <uint32_t>(inp[exe_file::HEADER_SIZE_PARA])) << 4;
  }
  // ------------------------------------------------------------------------
  void unlzexe::unpack (output_exe_file& oexe)
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

    uint32_t fpos = static_cast <uint32_t>(0x1C + oexe.rellocations().size()*4);
    uint32_t i = (0x200 - static_cast <int> (fpos)) & 0x1ff;
    oexe[exe_file::HEADER_SIZE_PARA] = static_cast <uint16_t>((fpos + i) >> 4);

    if (m_exe_file[exe_file::MAX_MEM_PARA] != 0)
      {
	int32_t delta = m_header[eINC_SIZE] + ((m_header[eDECOMPRESSOR_SIZE] + 16 - 1) >> 4) + 9;
	oexe[exe_file::MIN_MEM_PARA] = static_cast <uint16_t>(oexe[exe_file::MIN_MEM_PARA] - delta);
	if (m_exe_file[exe_file::MAX_MEM_PARA] != static_cast <uint16_t>(0xFFFF))
	  {
	    oexe[exe_file::MAX_MEM_PARA] = static_cast <uint16_t>(oexe[exe_file::MAX_MEM_PARA] - (m_header[eINC_SIZE] - oexe[exe_file::MIN_MEM_PARA]));
	  }
      }

    oexe[exe_file::NUM_OF_BYTES_IN_LAST_PAGE] = static_cast <uint16_t> ((static_cast <uint16_t> (load_size) + (oexe[exe_file::HEADER_SIZE_PARA] << 4)) & 0x1ff);
    oexe[exe_file::NUM_OF_PAGES] = static_cast <uint16_t>((load_size + (static_cast <uint32_t>(oexe[exe_file::HEADER_SIZE_PARA]) << 4) + 0x1ff) >> 9);


    
    oexe.eval_structures();
  }
  // ======================================================================
  uint32_t unlzexe::decomp_size() const
  {
    return 0;
  }
} // ns explode
