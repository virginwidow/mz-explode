#include <vector>
#include <cstring>
#include <iostream>

#include "explode/unpklite.hh"
#include "explode/exe_file.hh"
#include "explode/io.hh"
#include "explode/struct_reader.hh"
#include "explode/exceptions.hh"

namespace
{
  bool is_supported (uint16_t h_pklite_info)
  {
    uint16_t h_pklite_info_3f = static_cast <uint16_t> (h_pklite_info & 0xFFF);

    switch (h_pklite_info_3f)
      {
      case 0x10C:
      case 0x10D:
      case 0x10E:
      case 0x10F:
      case 0x132:
      case 0x103:
	return true;
      }
  
    switch (h_pklite_info)
      {
      case 0x1114:
      case 0x3114:
      case 0x100:
      case 0x105:
      case 0x2105:
      case 0x210A:
	return true;
      }
    return false;
  }
  // ===================================================================
  void adjust_length_code_2000 (uint16_t& length_code, explode::bit_reader& f, bool uncompressed_region)
  {
    // 4627
    while (true)
      {
	bool handeled = true;
	
	switch (length_code)
	  {
	  case 4:
	    length_code = 3;
	    break;
	  case 0x0a:
	    length_code = 2;
	    break;
	  case 0x0b:
	    {
	      length_code = static_cast <uint16_t>(0x0A + static_cast <uint16_t>(f.byte ()));
	      if (length_code == 0x109)
		{
		  length_code = 0xFFFF;
		}
	      if (length_code == 0x108)
		{
		  if (uncompressed_region)
		    {
		      length_code = 0xfffd;
		    }
		}
	    }
	    break;
	  case 0xc:
	    length_code = 4;
	    break;
	  case 0xd:
	    length_code = 5;
	    break;
	  case 0x1c:
	    length_code = 6;
	    break;
	  case 0x1d:
	    length_code = 7;
	    break;
	  case 0x1e:
	    length_code = 8;
	    break;
	  case 0x1f:
	    length_code = 9;
	    break;
	  default:
	    length_code = static_cast <uint16_t> (f.bit () | (length_code << 1));
	    handeled = false;
	    break;
	  }
	// loc_4748	
	if (handeled)
	  {
	    //goto loc_4989;
	    break;
	  }
      }
  }
  // -------------------------------------------------------------------
  void adjust_length_code_n2000 (uint16_t& length_code, explode::bit_reader& f, bool uncompressed_region)
  {
    // 474e
    while (true)
      {
	bool handeled = true;
	switch (length_code)
	  {
	  case 6:
	    length_code = 2;
	    break;
	  case 7:
	    length_code = 3;
	    break;
	  case 8:
	    length_code = 4;
	    break;
	  case 0x12:
	    length_code = 5;
	    break;
	  case 0x13:
	    length_code = 6;
	    break;
	  case 0x14:
	    length_code = 7;
	    break;
	  case 0x2a:
	    length_code = 8;
	    break;
	  case 0x2b:
	    length_code = 9;
	    break;
	  case 0x2c:
	    length_code = 0xa;
	    break;
	  case 0x5a:
	    length_code = 0xb;
	    break;
	  case 0x5b:
	    length_code = 0xc;
	    break;
	  case 0x5c:
	    {
	      length_code = static_cast <uint16_t> (0x19 + static_cast <uint16_t>(f.byte ()));
	      if (length_code == 0x118)
		{
		  length_code = 0xFFFF;
		}
	      if (length_code == 0x117)
		{
		  length_code = 0xfffe;
		}
	      if (length_code == 0x116)
		{
		  if (!uncompressed_region)
		    {
		      length_code = 0xfffd;
		    }
		}
	    }
	    break;
	  case 0xba:
	    length_code = 0xd;
	    break;
	  case 0xbb:
	    length_code = 0xe;
	    break;
	  case 0xbc:
	    length_code = 0xf;
	    break;
	  case 0x17a:
	    length_code = 0x10;
	    break;
	  case 0x17b:
	    length_code = 0x11;
	    break;
	  case 0x17c:
	    length_code = 0x12;
	    break;
	  case 0x2fa:
	    length_code = 0x13;
	    break;
	  case 0x2fb:
	    length_code = 0x14;
	    break;
	  case 0x2fc:
	    length_code = 0x15;
	    break;
	  case 0x2fd:
	    length_code = 0x16;
	    break;
	  case 0x2fe:
	    length_code = 0x17;
	    break;
	  case 0x2ff:
	    length_code = 0x18;
	    break;
	  default:
	    length_code = static_cast <uint16_t> (f.bit () | (length_code << 1));
	    handeled = false;
	  } // end of switch
	if (handeled)
	  {
	    break;
	  }
      }
  }

  typedef void (*adjust_length_code_fn) (uint16_t& length_code, 
					 explode::bit_reader& f, 
					 bool uncompressed_region);

  uint16_t get_base_offset (explode::bit_reader& f)
  {
    // 4b05
    while (true)
      {
	uint16_t offs = f.bit ();
	if (offs == 1)
	  {
	    return 0;
	  }
	//	loc_4b6a:;
	offs = static_cast <uint16_t> (f.bit () | (offs << 1));
	// 4bb9
	offs = static_cast <uint16_t> (f.bit () | (offs << 1));
	// 4c08
	offs = static_cast <uint16_t> (f.bit () | (offs << 1));

	switch (offs)
	  {

	  case 0:
	    // 4c57
	    return 0x100;
	  case 1:
	    // 4c68
	    return 0x200;
	  default:
	    // 4c79
	    offs = static_cast <uint16_t> (f.bit () | (offs << 1));
	    switch (offs)
	      {
	      case 4:
		return 0x300;
	      case 5:
		return 0x400;
	      case 6:
		return 0x500;
	      case 7:
		return 0x600;
	      default:
		// 4d0c
		offs = static_cast <uint16_t> (f.bit () | (offs << 1));
		switch (offs)
		  {
		  case 0x10:
		    return 0x700;
		  case 0x11:
		    return 0x800;
		  case 0x12:
		    return 0x900;
		  case 0x13:
		    return 0xA00;
		  case 0x14:
		    return 0xB00;
		  case 0x15:
		    return 0xC00;
		  case 0x16:
		    return 0xD00;
		  default:
		    // 4dd2
		    offs = static_cast <uint16_t> (f.bit () | (offs << 1));
		    if (offs >= 0x2E)
		      {
			return static_cast <uint16_t> ((offs & 0x1f) << 8);
		      }
		  }
	      }
	  }
      }
#if !defined(__clang__)
    throw explode::decoder_error ("should not be here");
    return 0;
#endif
  }
  // ===================================================================
  void build_rellocs (uint16_t h_pklite_info,
		      explode::struct_reader <uint32_t>& fr, 
		      std::vector <explode::rellocation>& rellocs)
  {

	explode::struct_reader <uint16_t> f(fr.input ());
    uint32_t relocs_count = 0;
    uint16_t var_counter = 0;
    uint32_t length_code = 0;
    uint32_t has_bytes = 0;
    
    if ((h_pklite_info & 0x1000) == 0)
      {
	while (true)
	  {
	    // 4f3b
	    length_code = f.byte ();
	    if (length_code == 0)
	      {
		break;
	      }
	    
	    var_counter = f.word (); 
	    
	    has_bytes = 0;
	    // 4fb6
	    while (has_bytes < length_code)
	      {
		// 4f87
		uint16_t rel = f.word (); 
		uint16_t seg = var_counter;
		
		rellocs.push_back (explode::rellocation (seg, rel));
		relocs_count++;
		has_bytes++;
	      }
	  }
      }
    else
      {
	// 4fc7
	var_counter = 0;
	while (true)
	  {
	    length_code = f.word (); 
	    if (length_code == 0xFFFF)
	      {
		break;
	      }
	    if (length_code != 0)
	      {
		has_bytes = 0;
		while (has_bytes < length_code)
		  {
		    //5013:;
		    uint16_t rel = f.word (); 
		    uint16_t seg = var_counter;
		    rellocs.push_back (explode::rellocation (seg, rel));
		    relocs_count++;
		    has_bytes++;
		  }
	      }
	    // 504d
	    var_counter = static_cast <uint16_t> (var_counter + 0x0FFF);
	  }
      }
  }
} // anonymous ns
// =====================================================================
namespace explode 
{
  unpklite::unpklite (input_exe_file& inp)
    : m_file (inp.file ()),
      m_exe_file (inp),
      m_header_length (0),
      m_decomp_size (0),
      m_compressed_size (0),
      m_decompressor_size (0),
      m_data_offset (0),
      m_uncompressed_region (false),
      m_has_checksum (false),
      m_h_pklite_info (0)
  {
    static offset_type pklite_info_offset = 2*0x0E;
    m_file.seek (pklite_info_offset);
    m_file.read (m_h_pklite_info);
	m_h_pklite_info = byte_order::from_little_endian (m_h_pklite_info);
    if (!is_supported (m_h_pklite_info))
      {
	throw decoder_error ("Unsuported version");
      }
    const uint32_t header_length_para = inp [exe_file::HEADER_SIZE_PARA];
    m_header_length = (header_length_para & 0xFFFF) << 4;
    _read_parameters ();
  }
  // ------------------------------------------------------------------
  void unpklite::unpack (output_exe_file& oexe)
  {
    uint32_t bx = 0;
    
    std::vector <uint8_t> code;
    std::size_t code_pos = 0;
    
    

    if ((m_h_pklite_info & 0x0FFF) == 0x114)
      {
	// goto loc_36F0;
      }
    else
      {
	bit_reader f (m_file, m_header_length);

	f.seek (m_data_offset);

	adjust_length_code_fn adjust_length_code = 
	  ((m_h_pklite_info & 0x2000) == 0) ? adjust_length_code_2000 : adjust_length_code_n2000;

	uint16_t length_code;
	
	while (bx < m_decomp_size)
	  {
	    //loc_44dc:;
	    length_code = f.bit ();

	    if (length_code == 0)
	      {
		uint8_t byte = f.byte ();
		if ((m_h_pklite_info & 0x01000) != 0)
		  {
		    byte = static_cast <uint8_t> (byte ^ f.count ());
		  }
		code.push_back (byte);
		bx++;
	      }
	    else
	      {
		oexe.code_put (code_pos, code);
		code_pos = bx;
		code.resize (0);
		
		// loc_4578
		length_code = static_cast <uint16_t> (f.bit () | (length_code << 1));
		// loc_45c7
		length_code = static_cast <uint16_t> (f.bit () | (length_code << 1));
		// loc_4616
		adjust_length_code (length_code, f, m_uncompressed_region);
		// 4989
		if (length_code == 0xFFFF)
		  {
		    break;
		  }
		else
		  {
		    if (length_code == 0xFFFD)
		      {
			throw decoder_error ("Not implemented");
		      }
		    if (length_code != 0xFFFE)
		      {
			uint16_t base_offs = 0;
			if (length_code != 2)
			  {
			    base_offs = get_base_offset (f);
			  }
			
			base_offs = static_cast <uint16_t>(base_offs + static_cast <uint16_t>(f.byte ()));
			// should check here
			uint32_t back_offs = bx - base_offs;
			oexe.code_copy (back_offs, length_code, bx);
			bx += length_code;
			code_pos = bx;
		      }
		  }
	      } // length_code != 0
	  } // end of explode cycle
	if (!code.empty ())
	  {
	    oexe.code_put (code_pos, code);
	  }
      }
    
    struct_reader <uint32_t> sr (m_file);
    build_rellocs (m_h_pklite_info, sr, oexe.rellocations ()); 

    struct_reader <uint16_t> f (m_file);
    
    oexe [exe_file::INITIAL_SS] = f.word ();
    oexe [exe_file::INITIAL_SP] = f.word ();
    oexe [exe_file::INITIAL_CS] = f.word ();
    oexe [exe_file::INITIAL_IP] = 0;
    uint32_t temp = ((m_decomp_size - bx) + 0x0F) >> 4;
    oexe [exe_file::MIN_MEM_PARA] = static_cast <uint16_t>(temp);
    oexe [exe_file::CHECKSUM]   = f.word ();
    
    const uint16_t relloc_bytes = static_cast <uint16_t> (oexe.rellocations ().size () * 4);

    uint16_t par_size = static_cast <uint16_t> (relloc_bytes + 0x1FF);
    par_size &= 0xFE00;
    par_size = static_cast <uint16_t> (par_size >> 4);

    oexe [exe_file::HEADER_SIZE_PARA] = par_size;
    
    union
    {
      uint8_t* bytes;
      uint16_t* words;
    } extra;

    uint16_t temp_extra = byte_order::to_little_endian (m_h_pklite_info);
	extra.words = &temp_extra;

    oexe.extra_header ().push_back (extra.bytes [0]);
    oexe.extra_header ().push_back (extra.bytes [1]);
    
    oexe.eval_structures ();
  }
  // ------------------------------------------------------------------
  uint32_t unpklite::header_length () const
  {
    return m_header_length;
  }
  // ------------------------------------------------------------------
  uint32_t unpklite::decomp_size () const
  {
    return m_decomp_size;
  }
  // ------------------------------------------------------------------
  uint32_t unpklite::compressed_size () const
  {
    return m_compressed_size;
  }
  // ------------------------------------------------------------------
  uint32_t unpklite::decompressor_size () const
  {
    return m_decompressor_size;
  }
  // ------------------------------------------------------------------
  uint32_t unpklite::data_offset () const
  {
    return m_data_offset;
  }
  // ------------------------------------------------------------------
  bool unpklite::uncompressed_region () const
  {
    return m_uncompressed_region;
  }
  // ------------------------------------------------------------------
  bool unpklite::has_checksum () const
  {
    return m_has_checksum;
  }
  // ------------------------------------------------------------------
  uint16_t unpklite::pklite_info () const
  {
    return m_h_pklite_info;
  }
  // ------------------------------------------------------------------
  void unpklite::_read_parameters ()
  {
    struct_reader <uint32_t> f (m_file, m_header_length);
    uint32_t temp = 0;
    // 829
    if (m_h_pklite_info == 0x0100 || m_h_pklite_info == 0x0103 || 
	m_h_pklite_info == 0x1103 || m_h_pklite_info == 0x2103 || 
	m_h_pklite_info == 0x3103 || m_h_pklite_info == 0x0105 || 
	m_h_pklite_info == 0x2105)
      {
	// 834
	// 9B4
	// B4A
	// CD5
	// FEB
	// 1176

	f.seek (1);
	m_decomp_size =  (f () << 4);
	m_decomp_size += (f () << 0x0C);

	f.seek (4);
	m_compressed_size = (f () << 4);
	m_compressed_size = (f () << 0x0C);

	f.seek (0x21);
	m_decompressor_size =  (f () << 1);
	m_decompressor_size += (f () << 9);

	f.seek (0x27);
	m_decompressor_size += f ();
	m_decompressor_size += (f () << 8);
	if (m_h_pklite_info == 0x1103)
	  {
	    m_data_offset = 0x1E0;
	  }
	else
	  {
	    if (m_h_pklite_info == 0x2103 || m_h_pklite_info == 0x2105)
	      {
		m_data_offset = 0x290;
	      }
	    else
	      {
		if (m_h_pklite_info == 0x3103)
		  {
		    m_data_offset = 0x2A0;
		  }
		else
		  {
		    m_data_offset = 0x1D0;
		  }
	      }
	  }
      }

    if (m_h_pklite_info == 0x210A)
      {
	// 1301
	f.seek (1);
	m_decomp_size = f () << 4;

	m_decomp_size += (f () << 0x0C);
	m_decomp_size += 0x100;

	f.seek (4);

	m_compressed_size = f () << 4;
	m_compressed_size += (f () << 0x0C);

	f.seek (0x37);

	m_decompressor_size =  f () << 1;
	m_decompressor_size += (f () << 9);

	f.seek (0x3C);
	m_decompressor_size +=  f ();
	m_decompressor_size += (f () << 8);

	m_data_offset = 0x290;
      }

    if (m_h_pklite_info == 0x010C || m_h_pklite_info == 0x210C || m_h_pklite_info == 0x110C ||
	m_h_pklite_info == 0x310C || m_h_pklite_info == 0x010D || m_h_pklite_info == 0x110D || 
	m_h_pklite_info == 0x210D || m_h_pklite_info == 0x310D)
      {
	// 1495
	// 17BD
	// 1629
	// 1951
	// 1AE5
	// 1C79
	// 1E0D
	// 1FA1

	f.seek (1);
	m_decomp_size = f () << 4;

	m_decomp_size += (f () << 0x0C);

	m_decomp_size += 0x100;

	f.seek (4);

	m_compressed_size =   f () << 4;
	m_compressed_size += (f () << 0x0C);


	f.seek (0x1D);

	m_decompressor_size = f () << 1;
	m_decompressor_size += (f () << 9);
	f.seek (0x23);
	m_decompressor_size += f ();
	m_decompressor_size += (f () << 8);


	if (m_h_pklite_info == 0x210C || m_h_pklite_info == 0x310C || 
	    m_h_pklite_info == 0x210D || m_h_pklite_info == 0x310D)
	  {
	    m_data_offset = 0x290;
	  }
	else
	  {
	    if (m_h_pklite_info == 0x110C || m_h_pklite_info == 0x110D)
	      {
		m_data_offset = 0x1E0;
	      }
	    else
	      {
		m_data_offset = 0x1D0;
	      }
	  }
      }

    if (m_h_pklite_info == 0x10E || m_h_pklite_info == 0x10F || m_h_pklite_info ==  0x210F)
      {
	// 2135
	// 2785
	// 2B07
	f.seek (0);
	const uint32_t type = f();
	if (type == 0xEB && (m_h_pklite_info == 0x10F || m_h_pklite_info == 0x210F))
	  {
	    // 27AB
	    // SYS file detected
	    m_header_length += (f () + 2);
	    f.seek (1);
	  }

	m_decomp_size = f () << 4;

	m_decomp_size += (f () << 0x0C);
	m_decomp_size += 0x100;

	f.seek (4);

	m_compressed_size = f () << 4;
	m_compressed_size += (f () << 0x0C);

	f.seek (0x37);
	m_decompressor_size = f () << 1;
	m_decompressor_size += (f () << 9);

	f.seek (0x3D);
	m_decompressor_size += f ();
	m_decompressor_size += (f () << 8);
	if (m_h_pklite_info == 0x210F)
	  {
	    m_data_offset = 0x290;
	  }
	else
	  {
	    m_data_offset = 0x1D0;
	  }

      }

    if (m_h_pklite_info == 0x110E || m_h_pklite_info == 0x310E || 
	m_h_pklite_info == 0x110F || m_h_pklite_info == 0x310F)
      {
	// 22C9
	// 25F1
	// 2973
	// 2CF5
	f.seek (1);
	m_decomp_size = f () << 4;

	m_decomp_size += (f () << 0x0C);
	m_decomp_size += 0x100;

	f.seek (4);

	m_compressed_size = f () << 4;
	m_compressed_size += (f () << 0x0C);

	f.seek (0x35);
	m_decompressor_size = f () << 1;
	m_decompressor_size += (f () << 9);

	f.seek (0x38);
	m_decompressor_size += f ();
	m_decompressor_size += (f () << 8);
	if (m_h_pklite_info == 0x310E || m_h_pklite_info == 0x310F)
	  {
	    m_data_offset = 0x2C0;
	  }
	else
	  {
	    m_data_offset = 0x200;
	  }
      }

    if (m_h_pklite_info == 0x210E)
      {
	// 245D
	f.seek (1);
	m_decomp_size = f () << 4;
	m_decomp_size += (f () << 0x0C);
	m_decomp_size += 0x100;

	f.seek (4);

	m_compressed_size = f () << 4;
	m_compressed_size += (f () << 0x0C);

	f.seek (0x36);
	m_decompressor_size = f () << 1;
	m_decompressor_size += (f () << 9);
	f.seek (0x3C);
	m_decompressor_size += f ();
	m_decompressor_size += (f () << 8);

	m_data_offset = 0x290;
      }

    if (m_h_pklite_info == 0x1114)
      {
	// 2E89
	f.seek (0);
	uint32_t type = f ();
	if (type != 0x50)
	  {
	    m_decomp_size = (f () << 4);
	    m_decomp_size += (f () << 0x0C);
	    m_decomp_size += 0x100;

	    f.seek (4);
	    m_compressed_size = f ();
	    m_compressed_size += (f () << 8);

	    f.seek (0x34);
	    m_decompressor_size = f () << 1;
	    m_decompressor_size += (f () << 9);

	    f.seek (0x37);
	    temp = f ();
	    temp = f () << 8;

	    temp = temp + 0xFF10;
	    temp = temp + 0xFFFF0000;
	    temp = temp & 0xFFFFFFF0;
	    m_data_offset = temp;
	  }
	else
	  {
	    m_h_pklite_info = 0x1132;
	  }
      }

    if (m_h_pklite_info == 0x3114)
      {
	// 3044
	f.seek (0);
	uint32_t type = f ();
	if (type != 0x50)
	  {
	    m_decomp_size = (f () << 4);
	    m_decomp_size += (f () << 0x0C);
	    m_decomp_size += 0x100;

	    f.seek (4);
	    m_compressed_size = f ();
	    m_compressed_size += (f () << 8);

	    f.seek (0x3C);
	    m_decompressor_size = f () << 1;
	    m_decompressor_size += (f () << 9);

	    f.seek (0x3F);
	    temp = f ();
	    temp = f () << 8;

	    temp = temp + 0xFF10;
	    temp = temp + 0xFFFF0000;
	    temp = temp & 0xFFFFFFF0;
	    m_data_offset = temp;
	  }
	else
	  {
	    m_h_pklite_info = 0x3132;
	  }
      }

    if (m_h_pklite_info == 0x0132 || m_h_pklite_info == 0x2132)
      {
	// 320A
	f.seek (2);
	m_decomp_size = (f () << 4);
	m_decomp_size += (f () << 0x0C);
	m_decomp_size += 0x100;

	f.seek (5);
	m_compressed_size = f ();
	m_compressed_size += (f () << 8);

	f.seek (0x48);
	m_decompressor_size = f () << 1;
	m_decompressor_size += (f () << 9);

	temp = m_decompressor_size << 1;
	const uint32_t lo = temp & 0x0000FFFF;
	const uint32_t hi = temp & 0xFFFF0000;
	if (hi == 0 && (lo == 0x0E || lo == 0x13F))
	  {
	    m_uncompressed_region = true;
	  }
	m_decompressor_size += 0x62;
	m_decompressor_size = m_decompressor_size & 0xFFFFFFF0;
	m_data_offset = m_decompressor_size;
      }

    if (m_h_pklite_info == 0x1132 || m_h_pklite_info == 0x3132)
      {
	// 33A6
	f.seek (2);
	m_decomp_size = (f () << 4);
	m_decomp_size += (f () << 0x0C);
	m_decomp_size += 0x100;

	f.seek (5);
	m_compressed_size = f ();
	m_compressed_size += (f () << 8);

	f.seek (0x56);
	m_decompressor_size = f () << 1;
	m_decompressor_size += (f () << 9);

	f.seek (0x59);
	temp = f ();

	const uint32_t lo = temp & 0x0000FFFF;
	const uint32_t hi = temp & 0xFFFF0000;

	if (hi == 0 && 
	    (lo == 0x36A || lo == 0x334 || lo == 0x42A || lo == 0x3F4 || 
	     lo == 0x35C || lo == 0x41A) )
	  {
	    if ((lo == 0x36A || lo == 0x334 || lo == 0x42A || lo == 0x3F4))
	      {
		m_has_checksum = true;
	      } 
	    if ((lo == 0x36A || lo == 0x42A || lo == 0x35C || lo == 0x41A))
	      {
		m_uncompressed_region = true;
	      }
	  }

	temp = temp + 0xFF10;
	temp = temp + 0xFFFF0000;
	temp = temp & 0xFFFFFFF0;
	m_data_offset = temp;
      }
   
  }
} // ns explode

