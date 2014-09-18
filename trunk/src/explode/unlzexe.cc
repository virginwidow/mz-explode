#include <vector>
#include <cstring>
#include <iostream>
#include "explode/unlzexe.hh"
#include "explode/exe_file.hh"
#include "explode/io.hh"
#include "explode/exceptions.hh"

static void build_rellocs_90 (explode::input& file, std::vector <uint32_t>& rellocs)
{
  uint16_t seg;
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
// ----------------------------------------------------------------
namespace explode
{
  unlzexe::unlzexe (input_exe_file& inp)
    : m_file (inp.file ()),
      m_exe_file (inp),
      m_ver (0)
  {
    static const offset_type magic_offs = 0x0E;
    char magic [4] = {0};
    m_file.seek (magic_offs);
    m_file.read (magic, 4);

    if (std::memcmp (magic, "LZ09", 4) == 0)
      {
	m_ver = 90;
      }
    else
      {
	if (std::memcmp (magic, "LZ91", 4) == 0)
	  {
	    m_ver = 91;
	  }
	else
	  {
	    throw decoder_error ("Unsuported version");
	  }
      }
    const offset_type header_pos = (inp [exe_file::HEADER_SIZE_PARA] + inp [exe_file::INITIAL_CS] << 4);
    m_file.seek (header_pos);
    union
    {
      char* bytes;
      uint16_t* words;
    } u;
    u.words = m_header;
    m_file.read (u.bytes, sizeof (m_header));
    if (m_ver == 90)
      {
	m_rellocs_offset = header_pos + 0x19D;
      }
    else
      {
	m_rellocs_offset = header_pos + 0x158;
      }
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

    offset_type orig_data_offs = 0xE;
    m_file.seek (orig_data_offs);
    for (; orig_data_offs < 0xF; orig_data_offs++)
      {
	uint8_t b;
	m_file.read (b);
	oexe.extra_header ().push_back (b);
      }
   
  }
} // ns explode
