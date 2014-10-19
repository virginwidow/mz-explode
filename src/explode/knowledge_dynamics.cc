#include "explode/knowledge_dynamics.hh"
#include "explode/io.hh"
#include "explode/exceptions.hh"
#include "explode/byte_order.hh"
#include "explode/exe_file.hh"
#include "explode/struct_reader.hh"


namespace explode
{
  knowledge_dynamics::knowledge_dynamics(input_exe_file& inp)
    : m_file(inp.file())
  {
    offset_type extra_data_start = inp [exe_file::NUM_OF_PAGES] * 512L;
    if (inp[exe_file::NUM_OF_BYTES_IN_LAST_PAGE])
      {
	extra_data_start = static_cast <offset_type> (extra_data_start - (512 - inp[exe_file::NUM_OF_BYTES_IN_LAST_PAGE]));
      }
    unsigned char mzHeader2[0x25];
    m_file.seek(extra_data_start);
    m_file.read_buff(reinterpret_cast<char*>(mzHeader2), 0x25);

    explode::inmem_input mio(mzHeader2, 0x25);
    input_exe_file inner (mio);
    offset_type exe_data_start2 = inner[exe_file::HEADER_SIZE_PARA] * 16L; 
    offset_type extra_data_start2 = inner[exe_file::NUM_OF_PAGES] * 512L;
    if (inner[exe_file::NUM_OF_BYTES_IN_LAST_PAGE])
      {
	extra_data_start2 = static_cast <offset_type> (extra_data_start2 - (512 - inner[exe_file::NUM_OF_BYTES_IN_LAST_PAGE]));
      }
    m_expected_size = static_cast <uint32_t>  (extra_data_start2 - exe_data_start2);
    m_code_offs = static_cast <uint32_t>(extra_data_start + exe_data_start2);
    for (int i = 0; i < exe_file::MAX_HEADER_VAL; i++)
      {
	m_header[i] = inner[static_cast <exe_file::header_t> (i)];
      }
  }
  // ------------------------------------------------------------
  bool knowledge_dynamics::accept(input_exe_file& inp)
  {
    input& f = inp.file();
    uint8_t bytes[3];
    f.seek(0x200);
    // e9 99 00
    f.read_buff(reinterpret_cast <char*>(bytes), 3);
    if (bytes[0] != 0xE9 || bytes[1] != 0x99 || bytes[2] != 0x00)
      {
	return false;
      }
    return true;
  }
  // ------------------------------------------------------------
  void knowledge_dynamics::unpack(output_exe_file& oexe)
  {
    m_file.seek(m_code_offs);

    static const std::size_t MBUFFER_SIZE = 1024;
    static const std::size_t MBUFFER_EDGE = (MBUFFER_SIZE - 3);

    char mbuffer[MBUFFER_SIZE];
    m_file.read_buff(mbuffer, MBUFFER_SIZE);

    std::size_t pos = 0;
    bool reset_hack = false;
    std::size_t step = 9;
    std::size_t bx = 0;
    /*
     * The dictionary.
     * Each entry consists of an index to a previous entry
     * and a value, forming a tree.
     */
    uint16_t dict_key[768 * 16];
    uint8_t  dict_val[768 * 16];
    uint16_t dict_index = 0x0102; /* Start populating dictionary from 0x0102 */
    uint16_t dict_range = 0x0200; /* Allow that much entries before increasing step */

    /* Since data stored this way is backwards, we need a small queue */
    uint8_t queue[0xFF];
    std::size_t queued = 0;
    uint16_t next_index = 0;	/* block of data we currently examine */

    uint8_t last_char = 0;	/* value from previous iteration */
    uint16_t last_index = 0;	/* block from previous iteration */

    uint32_t big_index;	/* temp. variable to safely load and shift 3 bytes */
    uint16_t keep_index;	/* temp. variable to keep "next_index" before making it "last_index" */
    while (true)
      {
	if (reset_hack)
	  {
	    step = 9;
	    dict_range = 0x0200;
	    dict_index = 0x0102;
	  }
	std::size_t byte_pos = pos / 8;
	std::size_t bit_pos = pos % 8;

	pos += step;	/* And advance to the next chunk */

	if (byte_pos >= MBUFFER_EDGE)
	  {
	    std::size_t bytes_extra = MBUFFER_SIZE - byte_pos;//~= 3
	    std::size_t bytes_left = MBUFFER_SIZE - bytes_extra;//~= 1021

	    /* Copy leftovers */
	    for (std::size_t j = 0; j < bytes_extra; j++) mbuffer[j] = mbuffer[bytes_left + j];

	    /* Read in the rest */
	    std::size_t remains = static_cast <std::size_t> (m_file.bytes_remains());
	    if (remains < bytes_left)
	      {
		m_file.read_buff(mbuffer + bytes_extra, remains);
	      }
	    else
	      {
		m_file.read_buff(mbuffer + bytes_extra, bytes_left);
	      }
				

	    /* Reset cursor */
	    pos = bit_pos + step;	/* Add all unused bits */
	    byte_pos = 0;
	    /* On dictionary reset, use byte offset as bit offset*/
	    if (reset_hack)
	      {
		bit_pos = bytes_extra;
	      }
	  }
	
	big_index =
	  static_cast <uint32_t>((static_cast <uint8_t>(mbuffer[byte_pos + 2]) & 0x00FF) << 16) |
	  static_cast <uint32_t>((static_cast <uint8_t>(mbuffer[byte_pos + 1]) & 0x00FF) << 8) |
	  static_cast <uint32_t>(static_cast <uint8_t>(mbuffer[byte_pos]) & 0x00FF);

	big_index >>= bit_pos;

	next_index = static_cast <uint16_t> (big_index & 0xFFFF);

	static /* Those masks help us get the relevant bits */ 
	  const uint16_t keyMask[4] = {
	  0x01FF, 	// 0001 1111 
	  0x03FF,		// 0011 1111
	  0x07FF,		// 0111 1111
	  0x0FFF,		// 1111 1111
	};

	if (step - 9 >= sizeof (keyMask))
	  {
	    throw decoder_error ("Overflow");
	  }
	next_index &= keyMask[(step - 9)];
	/* Apply the value as-is, continuing with dictionary reset, C) */
	if (reset_hack)
	  {
	    /* Save index */
	    last_index = next_index;
	    /* Output char value */
	    last_char = static_cast <uint8_t>(next_index & 0x00FF);
	    oexe.code_fill(bx++, last_char, 1);
				
	    /* We're done with the hack */
	    reset_hack = false;
	    continue;
	  }
	if (next_index == 0x0101)	/* End Of File */
	  {
	    /* DONE */
	    break;
	  }

	if (next_index == 0x0100) 	/* Reset dictionary */
	  {
	    /* Postpone it into next iteration */
	    reset_hack = true;
	    continue;
	  }

	/* Remember *real* "next_index" */
	keep_index = next_index;

	/* No dictionary entry to query, step back */
	if (next_index >= dict_index)
	  {
	    next_index = last_index;
	    if (queued >= sizeof (queue))
	      {
		throw decoder_error ("Overflow");
	      }
	    /* Queue 1 char */
	    queue[queued++] = last_char;
	  }

	/* Quering dictionary? */
	while (next_index > 0x00ff)
	  {
			  
	    /* Queue 1 char */
	    if (queued >= sizeof (queue))
	      {
		throw decoder_error ("Overflow");
	      }
	    if (next_index >= sizeof (dict_val))
	      {
		throw decoder_error ("Overflow");
	      }
	    queue[queued++] = dict_val[next_index];
	    /* Next query: */
	    next_index = dict_key[next_index];
	  }

	/* Queue 1 char */
	last_char = static_cast <uint8_t> (next_index & 0x00FF);
	if (queued >= sizeof (queue))
	  {
	    throw decoder_error ("Overflow");
	  }
	queue[queued++] = last_char;

	/* Unqueue */
	while (queued)
	  {
	    oexe.code_fill(bx++, queue[--queued], 1);
	  }

	/* Save value to the dictionary */
	if (next_index >= sizeof (dict_val))
	  {
	    throw decoder_error ("Overflow");
	  }
	dict_key[dict_index] = last_index; /* "goto prev entry" */
	dict_val[dict_index] = last_char;  /* the value */
	dict_index++;

	/* Save *real* "next_index" */
	last_index = keep_index;

	/* Edge of dictionary, increase the bit-step, making range twice as large. */
	if (dict_index >= dict_range && step < 12)
	  {
	    step += 1;
	    dict_range = static_cast <uint16_t> (dict_range*2);
	  }
      }
    oexe[exe_file::INITIAL_CS] = m_header[exe_file::INITIAL_CS];
    oexe[exe_file::INITIAL_IP] = m_header[exe_file::INITIAL_IP];
    oexe[exe_file::INITIAL_SS] = m_header[exe_file::INITIAL_SS];
    oexe[exe_file::INITIAL_SP] = m_header[exe_file::INITIAL_SP];
    oexe[exe_file::MAX_MEM_PARA] = m_header[exe_file::MAX_MEM_PARA];
    oexe[exe_file::MIN_MEM_PARA] = static_cast <uint16_t> ((m_expected_size + 0x20) / 64);
    oexe.eval_structures();
  }
  // ------------------------------------------------------------
  uint32_t knowledge_dynamics::decomp_size() const
  {
    return m_expected_size;
  }
} // ns explode
