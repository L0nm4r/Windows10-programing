#include <windows.h>
#include <stdio.h>

int main()
{
	SYSTEM_INFO si;
	::GetNativeSystemInfo(&si); // 双冒号意味着这个函数是Windowsapi

	printf("Number of Logical Processors: %d\n",si.dwNumberOfProcessors); // 处理器 
	printf("Page size: %d Bytes\n",si.dwPageSize); // 
	printf("Processor Mask: 0x%p\n", (PVOID)si.dwActiveProcessorMask); // 0x000000000000000F
	printf("Minimum process address : 0x%p\n", si.lpMinimumApplicationAddress); // 0x00010000
	printf("Maxmum process address : 0x%p\n", si.lpMaximumApplicationAddress);
	/*	x86 0xFFFEFFFF
		x64 0x00007FFFFFFEFFFF*/
	return 0;
}
	