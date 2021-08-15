#include <windows.h>
#include <stdio.h>
// ShowError
int main(int argc, const char* argv[]) {
	if (argc < 2) {
		printf("Usage: ShowError <number>\n");
		return 0;
	}
	int message = atoi(argv[1]); // atoi : 把字符串转换为一个整型数
	LPWSTR text; // 只定义了一个指针（字符串）
	DWORD chars = ::FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		// function allocates
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, message, 0,
		(LPWSTR)&text, // 把str指针的地址传进去
		// ugly cast
		0, nullptr);
	if (chars > 0) {
		printf("Message %d: %ws\n", message, text);
		::LocalFree(text);  // 手动释放内存空间
	}
	else {
		printf("No such error exists\n");
	}
	return 0;
}