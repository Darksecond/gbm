#pragma once

template<unsigned bitno, unsigned nbits=1, typename T=uint8_t>
struct RegBit
{
	T data;
	enum { mask = (1u << nbits) - 1u };
	template<typename T2>
		RegBit& operator=(T2 val)
		{
			data = (data & ~(mask << bitno)) | ((nbits > 1 ? val & mask : !!val) << bitno);
			return *this;
		}
	operator unsigned() const { return (data >> bitno) & mask; }
	RegBit& operator++ ()     { return *this = *this + 1; }
	unsigned operator++ (int) { unsigned r = *this; ++*this; return r; }
};
