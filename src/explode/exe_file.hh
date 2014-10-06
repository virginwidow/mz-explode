#ifndef __EXPLODE_EXE_FILE_HH__
#define __EXPLODE_EXE_FILE_HH__

#include <stdint.h>
#include <vector>

namespace explode
{
  class input;

  class exe_file
  {
  public:
    enum header_t
      {
	SIGNATURE = 0,              // 0x0
	NUM_OF_BYTES_IN_LAST_PAGE,  // 0x1
	NUM_OF_PAGES,               // 0x2
	RELLOCATION_ENTRIES,        // 0x3  
	HEADER_SIZE_PARA,           // 0x4
	MIN_MEM_PARA,               // 0x5
	MAX_MEM_PARA,               // 0x6
	INITIAL_SS,                 // 0x7
	INITIAL_SP,                 // 0x8
	CHECKSUM,                   // 0x9
	INITIAL_IP,                 // 0xA
	INITIAL_CS,                 // 0xB
	RELLOC_OFFSET,              // 0xC
	OVERLAY_NUM,                // 0xD
	MAX_HEADER_VAL              // 0xE
      };
    exe_file ();
    uint16_t operator [] (header_t hv) const;
  protected:
    uint16_t m_header [MAX_HEADER_VAL];
  };
  // ==============================================================
  class input_exe_file : public exe_file
  {
  public:
    explicit input_exe_file (input& file);
    bool is_pklite () const;
    bool is_lzexe () const;
	bool is_exepack() const;
    input& file ();
  private:
    input_exe_file (const input_exe_file&);
    input_exe_file& operator = (const input_exe_file&);
  private:
    input&   m_file;
  };
  // ==============================================================
  class output;

  struct rellocation
  {
	  rellocation()
		  : seg(0),
		  rel(0)
	  {

	  }

	  rellocation(uint16_t s, uint16_t r)
		  : seg(s),
		  rel(r)
	  {

	  }
	  uint16_t rel;
	  uint16_t seg;
  };

  class output_exe_file : public exe_file
  {
  public:
    typedef std::vector <rellocation> rellocations_t;
  public:
    output_exe_file ();
    virtual ~output_exe_file ();

    uint16_t& operator [] (header_t hv);

    rellocations_t& rellocations ();
    const rellocations_t& rellocations () const;
    
    std::vector <uint8_t>& extra_header ();
    const std::vector <uint8_t>& extra_header () const;

	virtual void code_set(uint8_t word, std::size_t length) = 0;
	virtual void code_put(std::size_t position, const uint8_t* code, std::size_t length) = 0;
	virtual void code_fill(std::size_t position, uint8_t code, std::size_t length) = 0;

    void code_put (std::size_t position, const std::vector <uint8_t>& code);
    virtual void code_copy (std::size_t from, std::size_t length, std::size_t to) = 0;
    
    virtual void eval_structures () = 0;

    virtual void write (output& out) const = 0;
  private:
    output_exe_file (const output_exe_file&);
    output_exe_file& operator = (const output_exe_file&);
  protected:
    rellocations_t m_rellocs;
    std::vector <uint8_t> m_extra_header;
    bool m_set [MAX_HEADER_VAL];
  };
  // ====================================================================
  class full_exe_file : public output_exe_file
  {
  public:
    explicit full_exe_file (uint32_t code_size);
	
	virtual void code_set(uint8_t word, std::size_t length);
	virtual void code_put(std::size_t position, const uint8_t* code, std::size_t length);
	virtual void code_fill(std::size_t position, uint8_t code, std::size_t length);
    virtual void code_copy (std::size_t from, std::size_t length, std::size_t to);
    
    virtual void eval_structures ();
    
    virtual void write (output& out) const;
  private:
    std::vector <uint8_t> m_code;
    std::size_t m_real_size;
  };
} // ns explode


#endif
