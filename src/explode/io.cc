#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstring>

#include "explode/io.hh"
#include "explode/exceptions.hh"

namespace explode
{
  input::input ()
  {
  }
  // -------------------------------------------------------------
  input::~input ()
  {
  }
  // =============================================================
  file_input::file_input (const char* path)
    : m_owner (true)
  {
    m_file = fopen (path, "rb");
    if (!m_file)
      {
	throw input_error ();
      }
  }
  // -------------------------------------------------------------
  file_input::file_input (FILE* file)
    : m_owner (false),
      m_file  (file)
  {
  }
  // -------------------------------------------------------------
  file_input::~file_input ()
  {
    if (m_owner)
      {
	fclose (m_file);
      }
  }
  // -------------------------------------------------------------
  void file_input::read (char* buffer, std::size_t size)
  {
    if (fread (buffer, size, 1, m_file) != 1)
      {
	throw input_error ();
      }
  }
  // -------------------------------------------------------------
  offset_type file_input::tell ()
  {
    const long pos = ftell (m_file);
    if (pos <= 0)
      {
	throw input_error ();
      }
    return static_cast <offset_type> (pos);
  }
  // -------------------------------------------------------------
  offset_type file_input::bytes_remains ()
  {
    offset_type current = tell ();
    if (fseek (m_file, 0, SEEK_END) != 0)
      {
	throw input_error ();
      }
    offset_type end = tell ();
    seek (current);
    return static_cast <offset_type> (end - current);
  }
  // -------------------------------------------------------------
  void file_input::seek (offset_type offset)
  {
    if (fseek (m_file, offset, SEEK_SET) != 0)
      {
	throw input_error ();
      }
  }
  // ==============================================================
  inmem_input::inmem_input(const unsigned char* data, std::size_t size)
	  : m_data(data),
	    m_size(size),
	    m_ptr(0)
  {
  }
  // -------------------------------------------------------------
  void inmem_input::read(char* buffer, std::size_t size)
  {
	  if (m_ptr + size > m_size)
	  {
		  throw input_error();
	  }
	  std::memcpy(buffer, m_data + m_ptr, size);
	  m_ptr += size;
  }
  // -------------------------------------------------------------
  offset_type inmem_input::tell()
  {
	  return m_ptr;
  }
  // -------------------------------------------------------------
  offset_type inmem_input::bytes_remains()
  {
	  return m_size - m_ptr;
  }
  // -------------------------------------------------------------
  void inmem_input::seek(offset_type offset)
  {
	  if (offset >= (offset_type)m_size)
	  {
		  throw input_error();
	  }
	  m_ptr = offset;
  }
  // =============================================================
  output::output ()
  {
  }
  // -------------------------------------------------------------
  output::~output ()
  {
  }
  // =============================================================
  file_output::file_output (const char* path)
    : m_owner (true)
  {
    m_file = fopen (path, "wb");
    if (!m_file)
      {
	throw input_error ();
      }
  }
  // -------------------------------------------------------------
  file_output::file_output (FILE* file)
    : m_owner (false),
      m_file  (file)
  {
  }
  // -------------------------------------------------------------
  file_output::~file_output ()
  {
    if (m_owner)
      {
	fclose (m_file);
      }
  }
  // -------------------------------------------------------------
  void file_output::write (const char* buffer, std::size_t size)
  {
    if (fwrite (buffer, size, 1, m_file) != 1)
      {
	throw input_error ();
      }
  }
  // -------------------------------------------------------------
  offset_type file_output::tell ()
  {
    const long pos = ftell (m_file);
    if (pos <= 0)
      {
	throw input_error ();
      }
    return static_cast <offset_type> (pos);
  }
  // -------------------------------------------------------------
  void file_output::seek (offset_type offset)
  {
    if (fseek (m_file, offset, SEEK_SET) != 0)
      {
	throw input_error ();
      }
  }
  // =============================================================
  inmem_output::inmem_output(std::vector <char>& out_buff)
	  : m_buff(out_buff),
	    m_ptr(out_buff.size())
  {

  }
  // -------------------------------------------------------------
  void inmem_output::write(const char* buffer, std::size_t size)
  {
	  if (m_ptr + size >= m_buff.size())
	  {
		  m_buff.resize(m_ptr + size);
	  }
	  std::memcpy(&m_buff[m_ptr], buffer, size);
	  m_ptr += size;
  }
  // -------------------------------------------------------------
  offset_type inmem_output::tell()
  {
	  return m_ptr;
  }
  // -------------------------------------------------------------
  void inmem_output::seek(offset_type offset)
  {
	  if (offset >= (off_t)m_buff.size())
	  {
		  throw input_error();
	  }
	  m_ptr = offset;
  }
} // ns explode
