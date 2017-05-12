#pragma once

#include <Psapi.h>

template <typename T>
class Pattern
{
public:
	Pattern(const BYTE* bMask, const char* szMask) : bMask(bMask), szMask(szMask) 
	{
		bSuccess = findPattern();
	}

	inline T get(int offset = 0)
	{
		return pResult + offset;
	}

	bool bSuccess;

private:
	bool findPattern()
	{
		MODULEINFO module = {};

		GetModuleInformation(GetCurrentProcess(), GetModuleHandle(nullptr), &module, sizeof(MODULEINFO));

		auto *address = reinterpret_cast<const char *>(module.lpBaseOfDll);

		auto address_end = address + module.SizeOfImage;

		for (;address < address_end; address++)
		{
			if (bCompare((BYTE*)(address), bMask, szMask))
			{
				pResult = (T)address;
				return true;
			}
		}

		pResult = NULL;
		return false;
	}

	inline bool bCompare(const BYTE* pData, const BYTE* bMask, const char* szMask)
	{
		for (; *szMask; ++szMask, ++pData, ++bMask)
			if (*szMask == 'x' && *pData != *bMask)
				return 0;
		return (*szMask) == NULL;
	}

	const BYTE *bMask; const char* szMask;
	T pResult;
};

class BytePattern : public Pattern<uintptr_t>
{
public:
	BytePattern(const BYTE* bMask, const char* szMask) : 
		Pattern<uintptr_t>(bMask, szMask) {}
};