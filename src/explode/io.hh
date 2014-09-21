#ifndef __EXPLODE_IO_HH__
#define __EXPLODE_IO_HH__

#include <stdio.h>
#include <cstddef>
#include <stdint.h>

#include <sys/types.h>


namespace explode
{
  typedef off_t offset_type;

  class input
  {
  public:
    input ();
    virtual ~input ();
    virtual void read (char* buffer, std::size_t size) = 0;
    virtual offset_type tell () = 0;
    virtual offset_type bytes_remains () = 0;
    virtual void seek (offset_type offset) = 0;

    template <typename T>
    void read (T& x)
    {
      union 
      {
	char* bytes;
	T*    words;
      } u;
      u.words = &x;
      read (u.bytes, sizeof (T));
    }

  private:
    input (const input&);
    input& operator = (const input&);
  };

  // ============================================================

  class output
  {
  public:
    output ();
    virtual ~output ();
    virtual void write (const char* buffer, std::size_t size) = 0;
    virtual offset_type tell () = 0;
    virtual void seek (offset_type offset) = 0;

    template <typename T>
    void write (T& x)
    {
      union 
      {
	char* bytes;
	T*    words;
      } u;
      u.words = &x;
      write (u.bytes, sizeof (T));
    }
  private:
    output (const output&);
    output& operator = (const output&);
  };

  // ============================================================

  class file_input : public input
  {
  public:
    explicit file_input (const char* path);
    explicit file_input (FILE* file);

    ~file_input ();

    virtual void read (char* buffer, std::size_t size);
    virtual offset_type tell ();
    virtual offset_type bytes_remains ();
    virtual void seek (offset_type offset);

  private:
    bool m_owner;
    FILE* m_file;
  };

  // ============================================================
  class file_output : public output
  {
  public:
    explicit file_output (const char* path);
    explicit file_output (FILE* file);

    ~file_output ();

    virtual void write (const char* buffer, std::size_t size);
    virtual offset_type tell ();
    virtual void seek (offset_type offset);

  private:
    bool m_owner;
    FILE* m_file;
  };

} // ns explode



#endif
