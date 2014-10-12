#include <iostream>
#include <cstring>
#include <algorithm>

#include "explode/exe_file.hh"
#include "explode/io.hh"
#include "explode/exceptions.hh"
#include "explode/byte_order.hh"

static const uint16_t MSDOS_MAGIC   = 0x5A4D;
static const uint16_t MSDOS_MAGIC_1 = 0x4D5A;

static const char* header_to_string(explode::exe_file::header_t h)
{
	switch (h)
	{
	case explode::exe_file::SIGNATURE: return "SIGNATURE";
	case explode::exe_file::NUM_OF_BYTES_IN_LAST_PAGE: return "NUM_OF_BYTES_IN_LAST_PAGE";
	case explode::exe_file::NUM_OF_PAGES: return "NUM_OF_PAGES";
	case explode::exe_file::RELLOCATION_ENTRIES: return "RELLOCATION_ENTRIES";
	case explode::exe_file::HEADER_SIZE_PARA: return "HEADER_SIZE_PARA";
	case explode::exe_file::MIN_MEM_PARA: return "MIN_MEM_PARA";
	case explode::exe_file::MAX_MEM_PARA: return "MAX_MEM_PARA";
	case explode::exe_file::INITIAL_SS: return "INITIAL_SS";
	case explode::exe_file::INITIAL_SP: return "INITIAL_SP";
	case explode::exe_file::CHECKSUM: return "CHECKSUM";
	case explode::exe_file::INITIAL_IP: return "INITIAL_IP";
	case explode::exe_file::INITIAL_CS: return "INITIAL_CS";
	case explode::exe_file::RELLOC_OFFSET: return "RELLOC_OFFSET";
	case explode::exe_file::OVERLAY_NUM: return "OVERLAY_NUM";
	}
	return "";
}

namespace explode
{
	std::ostream& operator << (std::ostream& os, exe_file::header_t h)
	{
		os << header_to_string(h);
		return os;
	}
	// =======================================================
  exe_file::exe_file ()
  {
    std::memset (m_header, 0, sizeof (m_header));
  }
  // --------------------------------------------------------
  uint16_t exe_file::operator [] (header_t hv) const
  {
    return m_header [hv];
  }
  // ========================================================
  input_exe_file::input_exe_file (input& file)
    : m_file (file)
  {
    union 
    {
      char* bytes;
      uint16_t* words;
    } u;
    u.words = m_header;
    m_file.read (u.bytes, sizeof (uint16_t)*MAX_HEADER_VAL);
    
    for (int i=0; i<MAX_HEADER_VAL; i++)
      {
	m_header [i] = byte_order::from_little_endian (m_header [i]);
      }

    if ((m_header[SIGNATURE] != MSDOS_MAGIC) && (m_header[SIGNATURE] != MSDOS_MAGIC_1))
      {
	throw exefile_error ();
      }
  }
  // --------------------------------------------------------
  bool input_exe_file::is_pklite () const
  {
    static const offset_type pklite_ver_offset = 2*0xF;
    m_file.seek (pklite_ver_offset);

    union 
    {
      char     bytes [4];
      uint16_t words [2];
    } u;

    m_file.read (u.bytes, 2*sizeof (uint16_t));
    
    u.words [0] = byte_order::from_little_endian (u.words [0]);
    u.words [1] = byte_order::from_little_endian (u.words [1]);

    return (u.words [0] == 0x4B50) && (u.words [1] == 0x494C);

  }
  // --------------------------------------------------------
  
  bool input_exe_file::is_lzexe () const
  {
    static const uint8_t sig90 [] = 
      {			
	0x06, 0x0E, 0x1F, 0x8B, 0x0E, 0x0C, 0x00, 0x8B,
	0xF1, 0x4E, 0x89, 0xF7, 0x8C, 0xDB, 0x03, 0x1E,
	0x0A, 0x00, 0x8E, 0xC3, 0xB4, 0x00, 0x31, 0xED,
	0xFD, 0xAC, 0x01, 0xC5, 0xAA, 0xE2, 0xFA, 0x8B,
	0x16, 0x0E, 0x00, 0x8A, 0xC2, 0x29, 0xC5, 0x8A,
	0xC6, 0x29, 0xC5, 0x39, 0xD5, 0x74, 0x0C, 0xBA,
	0x91, 0x01, 0xB4, 0x09, 0xCD, 0x21, 0xB8, 0xFF,
	0x4C, 0xCD, 0x21, 0x53, 0xB8, 0x53, 0x00, 0x50,
	0xCB, 0x2E, 0x8B, 0x2E, 0x08, 0x00, 0x8C, 0xDA,
	0x89, 0xE8, 0x3D, 0x00, 0x10, 0x76, 0x03, 0xB8,
	0x00, 0x10, 0x29, 0xC5, 0x29, 0xC2, 0x29, 0xC3,
	0x8E, 0xDA, 0x8E, 0xC3, 0xB1, 0x03, 0xD3, 0xE0,
	0x89, 0xC1, 0xD1, 0xE0, 0x48, 0x48, 0x8B, 0xF0,
	0x8B, 0xF8, 0xF3, 0xA5, 0x09, 0xED, 0x75, 0xD8,
	0xFC, 0x8E, 0xC2, 0x8E, 0xDB, 0x31, 0xF6, 0x31,
	0xFF, 0xBA, 0x10, 0x00, 0xAD, 0x89, 0xC5, 0xD1,
	0xED, 0x4A, 0x75, 0x05, 0xAD, 0x89, 0xC5, 0xB2,
	0x10, 0x73, 0x03, 0xA4, 0xEB, 0xF1, 0x31, 0xC9,
	0xD1, 0xED, 0x4A, 0x75, 0x05, 0xAD, 0x89, 0xC5,
	0xB2, 0x10, 0x72, 0x22, 0xD1, 0xED, 0x4A, 0x75,
	0x05, 0xAD, 0x89, 0xC5, 0xB2, 0x10, 0xD1, 0xD1,
	0xD1, 0xED, 0x4A, 0x75, 0x05, 0xAD, 0x89, 0xC5,
	0xB2, 0x10, 0xD1, 0xD1, 0x41, 0x41, 0xAC, 0xB7,
	0xFF, 0x8A, 0xD8, 0xE9, 0x13, 0x00, 0xAD, 0x8B,
	0xD8, 0xB1, 0x03, 0xD2, 0xEF, 0x80, 0xCF, 0xE0,
	0x80, 0xE4, 0x07, 0x74, 0x0C, 0x88, 0xE1, 0x41,
	0x41, 0x26, 0x8A, 0x01, 0xAA, 0xE2, 0xFA, 0xEB,
	0xA6, 0xAC, 0x08, 0xC0, 0x74, 0x40, 0x3C, 0x01,
	0x74, 0x05, 0x88, 0xC1, 0x41, 0xEB, 0xEA, 0x89
      };

    static const uint8_t sig91 [] = 
      {
	0x06, 0x0E, 0x1F, 0x8B, 0x0E, 0x0C, 0x00, 0x8B,
	0xF1, 0x4E, 0x89, 0xF7, 0x8C, 0xDB, 0x03, 0x1E,
	0x0A, 0x00, 0x8E, 0xC3, 0xFD, 0xF3, 0xA4, 0x53,
	0xB8, 0x2B, 0x00, 0x50, 0xCB, 0x2E, 0x8B, 0x2E,
	0x08, 0x00, 0x8C, 0xDA, 0x89, 0xE8, 0x3D, 0x00,
	0x10, 0x76, 0x03, 0xB8, 0x00, 0x10, 0x29, 0xC5,
	0x29, 0xC2, 0x29, 0xC3, 0x8E, 0xDA, 0x8E, 0xC3,
	0xB1, 0x03, 0xD3, 0xE0, 0x89, 0xC1, 0xD1, 0xE0,
	0x48, 0x48, 0x8B, 0xF0, 0x8B, 0xF8, 0xF3, 0xA5,
	0x09, 0xED, 0x75, 0xD8, 0xFC, 0x8E, 0xC2, 0x8E,
	0xDB, 0x31, 0xF6, 0x31, 0xFF, 0xBA, 0x10, 0x00,
	0xAD, 0x89, 0xC5, 0xD1, 0xED, 0x4A, 0x75, 0x05,
	0xAD, 0x89, 0xC5, 0xB2, 0x10, 0x73, 0x03, 0xA4,
	0xEB, 0xF1, 0x31, 0xC9, 0xD1, 0xED, 0x4A, 0x75,
	0x05, 0xAD, 0x89, 0xC5, 0xB2, 0x10, 0x72, 0x22,
	0xD1, 0xED, 0x4A, 0x75, 0x05, 0xAD, 0x89, 0xC5,
	0xB2, 0x10, 0xD1, 0xD1, 0xD1, 0xED, 0x4A, 0x75,
	0x05, 0xAD, 0x89, 0xC5, 0xB2, 0x10, 0xD1, 0xD1,
	0x41, 0x41, 0xAC, 0xB7, 0xFF, 0x8A, 0xD8, 0xE9,
	0x13, 0x00, 0xAD, 0x8B, 0xD8, 0xB1, 0x03, 0xD2,
	0xEF, 0x80, 0xCF, 0xE0, 0x80, 0xE4, 0x07, 0x74,
	0x0C, 0x88, 0xE1, 0x41, 0x41, 0x26, 0x8A, 0x01,
	0xAA, 0xE2, 0xFA, 0xEB, 0xA6, 0xAC, 0x08, 0xC0,
	0x74, 0x34, 0x3C, 0x01, 0x74, 0x05, 0x88, 0xC1,
	0x41, 0xEB, 0xEA, 0x89, 0xFB, 0x83, 0xE7, 0x0F,
	0x81, 0xC7, 0x00, 0x20, 0xB1, 0x04, 0xD3, 0xEB,
	0x8C, 0xC0, 0x01, 0xD8, 0x2D, 0x00, 0x02, 0x8E,
	0xC0, 0x89, 0xF3, 0x83, 0xE6, 0x0F, 0xD3, 0xEB,
	0x8C, 0xD8, 0x01, 0xD8, 0x8E, 0xD8, 0xE9, 0x72
      };

	static const uint8_t sig91e [] = {
	0x50, 0x06, 0x0E, 0x1F, 0x8B, 0x0E, 0x0C, 0x00, 
	0x8B, 0xF1, 0x4E, 0x89, 0xF7, 0x8C, 0xDB, 0x03, 
	0x1E, 0x0A, 0x00, 0x8E, 0xC3, 0xFD, 0xF3, 0xA4, 
	0x53, 0xB8, 0x2C, 0x00, 0x50, 0xCB, 0x2E, 0x8B, 
	0x2E, 0x08, 0x00, 0x8C, 0xDA, 0x89, 0xE8, 0x3D, 
	0x00, 0x10, 0x76, 0x03,	0xB8, 0x00, 0x10, 0x29, 
	0xC5, 0x29, 0xC2, 0x29, 0xC3, 0x8E, 0xDA, 0x8E, 
	0xC3, 0xB1, 0x03, 0xD3, 0xE0, 0x89, 0xC1, 0x48, 
	0xD1, 0xE0, 0x8B, 0xF0, 0x8B, 0xF8, 0xF3, 0xA5, 
	0x09, 0xED, 0x75, 0xD9, 0xFC, 0x8E, 0xC2, 0x8E, 
	0xDB, 0x31, 0xF6, 0x31, 0xFF, 0xBA, 0x10, 0x00,
	0xAD, 0x89, 0xC5, 0xD1, 0xED, 0x4A, 0x75, 0x05, 
	0xAD, 0x89, 0xC5, 0xB2, 0x10, 0x73, 0x03, 0xA4, 
	0xEB, 0xF1, 0x31, 0xC9, 0xD1, 0xED, 0x4A, 0x75, 
	0x05, 0xAD, 0x89, 0xC5, 0xB2, 0x10, 0x72, 0x22, 
	0xD1, 0xED, 0x4A, 0x75, 0x05, 0xAD, 0x89, 0xC5, 
	0xB2, 0x10, 0xD1, 0xD1, 0xD1, 0xED, 0x4A, 0x75, 
	0x05, 0xAD, 0x89, 0xC5, 0xB2, 0x10, 0xD1, 0xD1, 
	0x41, 0x41, 0xAC, 0xB7, 0xFF, 0x8A, 0xD8, 0xE9, 
	0x13, 0x00, 0xAD, 0x8B, 0xD8, 0xB1, 0x03, 0xD2, 
	0xEF, 0x80, 0xCF, 0xE0, 0x80, 0xE4, 0x07, 0x74, 
	0x0C, 0x88, 0xE1, 0x41, 0x41, 0x26, 0x8A, 0x01,
	0xAA, 0xE2, 0xFA, 0xEB, 0xA6, 0xAC, 0x08, 0xC0, 
	0x74, 0x34, 0x3C, 0x01, 0x74, 0x05, 0x88, 0xC1, 
	0x41, 0xEB, 0xEA, 0x89, 0xFB, 0x83, 0xE7, 0x0F, 
	0x81, 0xC7, 0x00, 0x20, 0xB1, 0x04, 0xD3, 0xEB, 
	0x8C, 0xC0, 0x01, 0xD8, 0x2D, 0x00, 0x02, 0x8E, 
	0xC0, 0x89, 0xF3, 0x83, 0xE6, 0x0F, 0xD3, 0xEB, 
	0x8C, 0xD8, 0x01, 0xD8, 0x8E, 0xD8, 0xE9, 0x72
	};


    uint8_t sigbuff [sizeof (sig90)];
    if ((m_header [RELLOC_OFFSET] != 0x1c) || (m_header [OVERLAY_NUM] != 0))
      {
	return false;
      }
    const offset_type entry = ((uint32_t)(m_header [HEADER_SIZE_PARA] + m_header [INITIAL_CS]) << 4) + m_header [INITIAL_IP];
    m_file.seek (entry);
    m_file.read ((char*)sigbuff, sizeof (sigbuff));
	
    if (std::memcmp (sigbuff, sig90, sizeof (sig90)) == 0)
      {
		return true;
      }
    if (std::memcmp (sigbuff, sig91, sizeof (sig91)) == 0)
      {
		return true;
      }
	if (std::memcmp(sigbuff, sig91e, sizeof(sig91e)) == 0)
	{
		return true;
	}
    return false;
  }
  // --------------------------------------------------------
  bool input_exe_file::is_exepack() const
  {
	  const offset_type exe_data_start = m_header [HEADER_SIZE_PARA] * 16L;
	  offset_type extra_data_start = m_header [NUM_OF_PAGES] * 512L;
	  if (m_header [NUM_OF_BYTES_IN_LAST_PAGE])
	  {
		  extra_data_start -= (512 - m_header[NUM_OF_BYTES_IN_LAST_PAGE]);
	  }
	  const offset_type first_offset = m_header [INITIAL_CS] * 0x10;// + exe_data_start;
	  const offset_type exe_len = first_offset;
	  try
	  {
		  offset_type exepack_hdr_start = exe_data_start + exe_len;
		  m_file.seek(exepack_hdr_start + 0x12 - 2);
		  char magic[2];
		  m_file.read(magic, 2);

		  const bool has_sig = (magic[0] == 'R' && magic[1] == 'B');
		  if (!has_sig)
		  {
			  return false;
		  }
		  const offset_type str_offs = exepack_hdr_start + 0x12 + 0x105; // exepack_hdr_start + unpk_len;
		  m_file.seek(str_offs);
		  char str[0x16];
		  m_file.read(str, sizeof(str));
		  const int rc = std::memcmp (str, "Packed file is corrupt", 0x16);
		  return rc == 0;
	  }
	  catch (...)
	  {
		  return false;
	  }

	  return true;

  }
  // --------------------------------------------------------
  input& input_exe_file::file ()
  {
    return m_file;
  }
  // ===================================================================
  output_exe_file::output_exe_file ()
    
  {
    std::memset (m_set, 0, sizeof (m_set));
    this->operator [] (SIGNATURE) = MSDOS_MAGIC;
  }
  // -------------------------------------------------------------------
  output_exe_file::~output_exe_file ()
  {
  }
  // -------------------------------------------------------------------
  uint16_t& output_exe_file::operator [] (header_t hv)
  {
    m_set [hv] = true;
    return m_header [hv];
  }
  // -------------------------------------------------------------------
  output_exe_file::rellocations_t& output_exe_file::rellocations ()
  {
    return m_rellocs;
  }
  // -------------------------------------------------------------------
  const output_exe_file::rellocations_t& output_exe_file::rellocations () const
  {
    return m_rellocs;
  }
  // -------------------------------------------------------------------
  std::vector <uint8_t>& output_exe_file::extra_header ()
  {
    return m_extra_header;
  }
  // -------------------------------------------------------------------
  const std::vector <uint8_t>& output_exe_file::extra_header () const
  {
    return m_extra_header;
  }
  // -------------------------------------------------------------------
  void output_exe_file::code_put(std::size_t position, const std::vector <uint8_t>& code)
  {
	  if (code.empty())
	  {
		  return;
	  }
	  this->code_put(position, &code[0], code.size());
  }
  // ===================================================================
  full_exe_file::full_exe_file (uint32_t code_size)
    : m_real_size (0)
  {
    m_code.resize (code_size);
  }
  // -------------------------------------------------------------------
  void full_exe_file::code_set(uint8_t word, std::size_t length)
  {
	  if (length != m_code.size())
	  {
		  m_code.resize(length, word);
	  }
	  else
	  {
		  std::memset(&m_code[0], word, length);
	  }
  }
  // -------------------------------------------------------------------
  void full_exe_file::code_put(std::size_t position, const uint8_t* code, std::size_t size)
  {
    if (size > 0)
      {
		m_real_size = std::max(m_real_size, position + size);
		if (m_code.size() < position + size)
		{
			m_code.resize (position + size);
		}
		std::memcpy(&m_code[position], code, size);
      }
  }
  // -------------------------------------------------------------------
  void full_exe_file::code_fill(std::size_t position, uint8_t code, std::size_t length)
  {
	  if (length > 0)
	  {
		  m_real_size = std::max(m_real_size, position + length);
		  if (m_code.size() < position + length)
		  {
			  m_code.resize(position + length);
		  }
		  std::memset(&m_code[position], code, length);
	  }
  }
  // -------------------------------------------------------------------
  void full_exe_file::code_copy (std::size_t from, std::size_t length, std::size_t to)
  {
    m_real_size = std::max (m_real_size, to + length);
	if (m_code.size() < to + length)
	{
		m_code.resize(to + length);
	}
    if (from + length < to)
      {
		std::memcpy (&m_code[to], &m_code[from], length);
      }
    else
      {
	for (std::size_t i = 0; i < length; i++)
	  {
	    m_code [to + i] = m_code [from + i];
	  }
      }
  }
  // ------------------------------------------------------------------  
  void full_exe_file::eval_structures ()
  {
    m_code.resize (m_real_size);
    if (!m_set [exe_file::OVERLAY_NUM])
      {
	this->operator [] (exe_file::OVERLAY_NUM) = 0;
      }
   
	this->operator [] (exe_file::RELLOCATION_ENTRIES) = (uint16_t)m_rellocs.size ();
   
   
	this->operator [] (exe_file::RELLOC_OFFSET) = (uint16_t)(exe_file::MAX_HEADER_VAL*sizeof (uint16_t) + 
								 m_extra_header.size ());
   
	if (!m_set[exe_file::HEADER_SIZE_PARA])
	{
		std::size_t hsize = sizeof(m_header) + m_rellocs.size () * 4;
		std::size_t hp = hsize / 16;
		if (hsize % 16)
		{
			hp++;
		}
		this->operator[] (exe_file::HEADER_SIZE_PARA) = hp;
	}

    if (!m_set [exe_file::NUM_OF_PAGES])
      {
	std::size_t total_size = m_header [exe_file::HEADER_SIZE_PARA]*16 + m_code.size ();
	this->operator [] (exe_file::NUM_OF_PAGES) = (uint16_t)(total_size / 512);
	this->operator [] (exe_file::NUM_OF_BYTES_IN_LAST_PAGE) = (uint16_t)(total_size % 512);
	if (m_header [exe_file::NUM_OF_BYTES_IN_LAST_PAGE])
	  {
	    m_header [exe_file::NUM_OF_PAGES]++;
	  }
      }
    if (!m_set [exe_file::MAX_MEM_PARA])
      {
	this->operator [] (exe_file::MAX_MEM_PARA) = 0xFFFF;
      }
	
  }
  // ------------------------------------------------------------------  
  void full_exe_file::write (output& out) const
  {
    const uint32_t relloc_entries = ((uint32_t)m_header [exe_file::RELLOCATION_ENTRIES]) & 0xFFFF;
    const uint32_t para_size      = ((uint32_t)m_header [exe_file::HEADER_SIZE_PARA]) & 0xFFFF;

    uint16_t new_header [MAX_HEADER_VAL];
    
    for (int i=0; i<MAX_HEADER_VAL; i++)
      {
	new_header [i] = byte_order::to_little_endian (m_header [i]);
      }

    union 
    {
      const char* bytes;
      const uint16_t* words;
    } h;
    h.words = new_header;
    out.write (h.bytes, sizeof (m_header));

    if (!m_extra_header.empty ())
      {
	out.write ((char*)&m_extra_header [0], m_extra_header.size ());
      }
	if (!m_rellocs.empty())
	{
		union
		{
			const char* bytes;
			const uint16_t* words;
		} r;
		
		std::vector <uint16_t> new_rel (relloc_entries*2);
		for (std::size_t i = 0; i < relloc_entries; i++)
		{
			const uint16_t rel = byte_order::to_little_endian (m_rellocs[i].rel);
			const uint16_t seg = byte_order::to_little_endian(m_rellocs[i].seg);
			new_rel[2*i]   = rel;
			new_rel[2*i+1] = seg;
		}
		r.words = &new_rel[0];
		out.write(r.bytes, relloc_entries * 4);
	}
	const std::size_t now = out.tell();
	if (now > para_size * 16)
	{
		throw decoder_error("bad header size");
	}
    const std::size_t sz = para_size*16 - now;
    if (sz)
      {
	std::vector <char> dummy (sz, 0);
	out.write (&dummy[0], sz);
      }

    out.write ((char*)&m_code[0], m_code.size ());
  }
} // ns explode

