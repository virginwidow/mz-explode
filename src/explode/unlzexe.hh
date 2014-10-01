#ifndef __EXPLODE_UNLZEXE_HH__
#define __EXPLODE_UNLZEXE_HH__

#include <stdint.h>
#include <stddef.h>

namespace explode
{
  class input_exe_file;
  class output_exe_file;
  class input;
  
  class unlzexe
  {
  public:
    unlzexe (input_exe_file& inp);

    uint32_t unpak (output_exe_file& oexe);
    
    uint32_t header_length () const;
    uint32_t decomp_size () const;
    uint32_t compressed_size () const;
    uint32_t decompressor_size () const;
    uint32_t data_offset () const;
    bool     uncompressed_region () const;
    bool     has_checksum () const;

  private:
    enum header_t
      {
	eIP,                    // 0
	eCS,                    // 1
	eSP,                    // 2
	eSS,                    // 3
	eCOMPRESSED_SIZE,       // 4
	eINC_SIZE,              // 5
	eDECOMPRESSOR_SIZE,     // 6
	eCHECKSUM,              // 7

	eHEADER_MAX
      };
  private:
    input&    m_file;
    input_exe_file& m_exe_file;
    
    int m_ver;
    uint16_t m_header [eHEADER_MAX];

    uint32_t m_rellocs_offset;
	uint32_t m_code_offset;
  };
} // ns explode

#endif