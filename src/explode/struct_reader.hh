#ifndef __EXPLODE_STRUCT_READER_HH__
#define __EXPLODE_STRUCT_READER_HH__

#include <stdint.h>

#include "explode/io.hh"
#include "explode/byte_order.hh"

namespace explode
{
	struct register_
	{
		struct low_word
		{
			uint8_t al;
			uint8_t ah;
		};

		union reg16
		{
			uint16_t ax;
			low_word r8;
		};

		struct reg32
		{
			reg16    r16;
			uint16_t hi;
		};
		union
		{
			uint8_t bytes[4];
			reg32    r32;
			uint32_t eax;
		} data;

		register_()
		{
			data.eax = 0;
		}

		operator uint32_t () const
		{
			return data.eax;
		}

		operator uint16_t () const
		{
			return data.r32.r16.ax;
		}

		operator uint8_t () const
		{
			return data.r32.r16.r8.al;
		}
	};
	// ---------------------------------------------------------------------
	class byte_reader
	{
	public:
		byte_reader(explode::input& input, uint32_t header_length)
			: m_input(input),
			m_header_length(header_length)
		{
		}

		explicit byte_reader(explode::input& input)
			: m_input(input),
			m_header_length(0)
		{
		}

		void seek(uint32_t offs)
		{
			m_input.seek(m_header_length + offs);
		}

		register_ byte()
		{
			register_ r;
			m_input.read(r.data.r32.r16.r8.al);
			return r;
		}
		uint16_t word()
		{
			uint16_t x;
			m_input.read(x);
			return byte_order::from_little_endian (x);
		}
		explode::offset_type tell()
		{
			return m_input.tell();
		}

	private:
		explode::input& m_input;
		const uint32_t  m_header_length;
	};
	// =====================================================================
	class bit_reader : public byte_reader
	{
	public:
		bit_reader(explode::input& input, uint32_t header_length)
			: byte_reader(input, header_length),
			m_word(0),
			m_count(0)
		{
		}

		explicit bit_reader(explode::input& input)
			: byte_reader(input),
			m_word(0),
			m_count(0)
		{
		}

		uint16_t bit()
		{
			if (m_count == 0)
			{
				m_word = word();
				m_count = 0x10;
			}
			uint16_t x = m_word & 1;
			m_word = (uint16_t)(m_word >> 1);
			m_count--;
			if (m_count == 0)
			{
				m_word = word();
				m_count = 0x10;
			}
			return x;
		}
		uint8_t count() const
		{
			return (uint8_t)(m_count & 0xFF);
		}

	private:
		uint16_t m_word;
		uint32_t m_count;
	};
	// =====================================================================
	template <typename Word>
	class struct_reader : public byte_reader
	{
	public:
		struct_reader(explode::input& input, uint32_t header_length)
			: byte_reader(input, header_length)
		{
		}

		explicit struct_reader(explode::input& input)
			: byte_reader(input)
		{
		}

		Word operator ()  ()
		{
			return static_cast <Word>(byte());
		}
	};
}

#endif
