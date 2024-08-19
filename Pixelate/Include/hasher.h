#pragma once

#include "internal_pixelate_include.h"

namespace Pixelate
{
	constexpr uint64_t FNV_PRIME = 0x100000001b3ull; // FNV-1a 64-bit salt
	
	class Hasher
	{
	public:

		void Hash(char value)
		{
			m_Hash = (m_Hash ^ value) * FNV_PRIME;
		}

		void Hash(uint16_t value)
		{
			Hash((char)(value & 01));
			Hash((char)(value >> 8));
		}

		void Hash(uint32_t value)
		{
			Hash((uint16_t)value);
			Hash((uint16_t)(value >> 16));
		}

		void Hash(uint64_t value)
		{
			Hash((uint32_t)value);
			Hash((uint32_t)(value >> 32));
		}

		void Hash(const char*& string)
		{
			for (const char* character = string; *character != '\0'; ++character)
				Hash(*character);
		}

		void Hash(const std::string& string)
		{
			Hash(string.c_str());
		}

		uint64_t GetValue()
		{
			return m_Hash;
		}

	private:
		uint64_t m_Hash = 0xcbf29ce484222325ull; // FNV-1a 64-bit initial hash value
	};

}
