#include <iostream>
#include <cstring>
#include <algorithm>

#include "explode/exe_file.hh"
#include "explode/io.hh"
#include "explode/exceptions.hh"

static const uint16_t MSDOS_MAGIC = 0x5A4D;

namespace explode
{
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

    if (m_header[SIGNATURE] != MSDOS_MAGIC)
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

    return (u.words [0] == 0x4B50) && (u.words [1] == 0x494C);

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
  // ===================================================================
  full_exe_file::full_exe_file (uint32_t code_size)
    : m_real_size (0)
  {
    m_code.resize (code_size);
  }
  // -------------------------------------------------------------------
  void full_exe_file::code_put (std::size_t position, const std::vector <uint8_t>& code)
  {
    m_real_size = std::max (m_real_size, position + code.size ());
    std::memcpy (m_code.data () + position, code.data (), code.size ());
  }
  // -------------------------------------------------------------------
  void full_exe_file::code_copy (std::size_t from, std::size_t length, std::size_t to)
  {
    m_real_size = std::max (m_real_size, to + length);
    if (from + length < to)
      {
	std::memcpy (m_code.data () + to, m_code.data () + from, length);
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
    if (!m_set [exe_file::RELLOCATION_ENTRIES])
      {
	this->operator [] (exe_file::RELLOCATION_ENTRIES) = m_rellocs.size ();
      }
    if (!m_set [exe_file::RELLOC_OFFSET])
      {
	this->operator [] (exe_file::RELLOC_OFFSET) = exe_file::MAX_HEADER_VAL*sizeof (uint16_t) + m_extra_header.size ();
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
    union 
    {
      const char* bytes;
      const uint16_t* words;
    } h;
    h.words = m_header;
    out.write (h.bytes, sizeof (m_header));
    if (!m_extra_header.empty ())
      {
	out.write ((char*)m_extra_header.data (), m_extra_header.size ());
      }
    union 
    {
      const char* bytes;
      const uint32_t* words;
    } r;
    r.words = m_rellocs.data ();
    out.write (r.bytes, m_header [exe_file::RELLOCATION_ENTRIES]*4);
    
    const std::size_t sz = m_header [exe_file::HEADER_SIZE_PARA]*16 - out.tell ();
    if (sz)
      {
	std::vector <char> dummy (sz, 0);
	out.write (dummy.data (), sz);
      }

    out.write ((char*)m_code.data (), m_code.size ());
  }
} // ns explode

