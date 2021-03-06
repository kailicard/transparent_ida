// patch_ida.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#include <list>

static int IS_IDA_64 = -1;


class PatchInfo
{
public:
	PatchInfo(size_t len,size_t offset,UINT8* bytes)
	{
		this->len = len;
		this->offset = offset;
		this->patched_bytes = (UINT8*)malloc(sizeof(UINT8)*(len+1));
		memcpy(this->patched_bytes, bytes, len);
	};
	PatchInfo(const PatchInfo& info)
	{
		this->len = info.len;
		this->offset = info.offset;
		this->patched_bytes = (UINT8*)malloc(sizeof(UINT8)*(len + 1));
		memcpy(this->patched_bytes, info.patched_bytes, len);
	}

	~PatchInfo()
	{
		free(this->patched_bytes);
	};

	size_t len = 0;
	size_t offset = 0;
	UINT8* patched_bytes = NULL;
};


MEMORY_BASIC_INFORMATION getModule()
{
	PBYTE pb = NULL;
	MEMORY_BASIC_INFORMATION mbi;
	MEMORY_BASIC_INFORMATION res;
	while (VirtualQuery(pb, &mbi, sizeof(mbi)) == sizeof(mbi))
	{
		WCHAR szModeName[MAX_PATH] = { 0 };
		if (mbi.State == MEM_FREE)
		{
			mbi.AllocationBase = mbi.BaseAddress;
		}
		if (
			mbi.AllocationBase != mbi.BaseAddress ||
			mbi.AllocationBase == NULL)
		{

		}
		else
		{
			GetModuleFileName((HINSTANCE)mbi.AllocationBase, szModeName, sizeof(szModeName));
			if (wcslen(szModeName) > 0)
			{
				wprintf(L"name : %s", szModeName);
				if (wcsstr(szModeName, L"ida")&&(wcsstr(szModeName, L"exe")))
				{
					printf("*");
					memcpy_s(&res, sizeof(MEMORY_BASIC_INFORMATION), &mbi, sizeof(MEMORY_BASIC_INFORMATION));
					if (wcsstr(szModeName, L"64"))
					{
						IS_IDA_64 = 1;
					}
					else
					{
						IS_IDA_64 = 0;
					}
				}
				puts("");

			}
		}
		pb += mbi.RegionSize;
	}
	return res;
}

void ida32Patch(PVOID base)
{
	std::list<PatchInfo> infos;

	UINT8 patch1[] = { 0x48,0x8D,0x4D,0x27,0x48,0x31,0xD2,0xFF,0x15,0x39,0x8F,0x1C,0x00,0x48,0x8D,0x4D,0x07,0x48,0x89,0xC2,0x90,0xFF,0x15,0x4B,0x8E,0x1C,0x00,0x48,0x8D,0x55,0x07,0x48,0x8D,0x4D,0xDF,0xFF,0x15,0xBD,0x91,0x1C,0x00,0x90,0x90 };
	infos.emplace_back(PatchInfo{ 0x2b ,0x3E6D2 ,patch1 });

	UINT8 patch2[] = { 0x48,0xC7,0xC2,0x7F,0x7F,0x7F,0x00,0xEB,0xC9 };
	infos.emplace_back(PatchInfo{ 0x9 ,0x33EDF ,patch2 });

	UINT8 patch3[] = { 0x5D ,0x37 };
	infos.emplace_back(PatchInfo{ 0x2 ,0x33EB7 ,patch3 });


	for(auto i = infos.begin();i!=infos.end();i++)
	{
		WriteProcessMemory(GetCurrentProcess(), (LPVOID)((size_t)base+i->offset), i->patched_bytes, i->len, 0);
	}
}

void ida64Patch(PVOID base)
{

	std::list<PatchInfo> infos;

	UINT8 patch1[] = { 0x48,0x8D,0x4D,0x27,0x48,0x31,0xD2,0xFF,0x15,0x29,0xAE,0x1C,0x00,0x48,0x8D,0x4D,0x07,0x48,0x89,0xC2,0xFF,0x15,0x3C,0xAD,0x1C,0x00,0x48,0x8D,0x55,0x07,0x48,0x8D,0x4D,0xDF,0xFF,0x15,0xAE,0xB0,0x1C,0x00,0x90,0x90,0x90 };
	infos.emplace_back(PatchInfo{ 0x2b ,0x3E7E2 ,patch1 });

	UINT8 patch2[] = { 0x48,0xC7,0xC2,0x7F,0x7F,0x7F,0x00,0xEB,0xC9 };
	infos.emplace_back(PatchInfo{ 0x9 ,0x33F7F ,patch2 });

	UINT8 patch3[] = { 0xBD,0x56 };
	infos.emplace_back(PatchInfo{ 0x2 ,0x33F57 ,patch3 });


	for (auto i = infos.begin(); i != infos.end(); i++)
	{
		WriteProcessMemory(GetCurrentProcess(), (LPVOID)((size_t)base + i->offset), i->patched_bytes, i->len, 0);
	}


}

extern "C"  __declspec(dllexport)  void patch()
{
	MEMORY_BASIC_INFORMATION idaModule =  getModule();

	if (IS_IDA_64 == -1)
	{
		return;
	}

	PVOID base = idaModule.BaseAddress;


//	char test[64] = { 0 };

//	sprintf_s(test, "ida base addr : %p", base);

//	MessageBoxA(NULL, test, "", MB_OK);

	size_t r = 0;

	VirtualProtect(base, 0x40000, PAGE_EXECUTE_READWRITE, (PDWORD)&r);

	if (IS_IDA_64 == 0)
	{
		ida32Patch(base);
	}

	if (IS_IDA_64 == 1)
	{
		ida64Patch(base);
	}

}