# Processes

## Process basics

在基本的process的基础之上又发展出很多process类型

- Protected processes : 创建之初是为了保护具有数字产权的进程免受别的进程的访问， 管理员进程也不能访问被保护进程的内存空间。
- UWP processes : host the Windows Runtime, and typically are published to the Microsoft Store
  - A UWP process executes inside an AppContainer - a sandbox of sorts that limits the operations this
    process can carry out.

- Protected Processes Light ：PPL，admin-level processes都不能访问其内存。可以使一个第三方服务以PPL进程运行
- Minimal Processes : no executable file mapped into the process address space, and no DLLs , The process address space is truly empty

- Pico Processes: 比minimal process多了一个pico provider, （kernel driver) 可以拦截Linux系统调用转化为等价的Windows系统调用，wsl实现的基础。

> minimal and Pico processes can only be created by the kernel.

PID : 值都是4的倍数，最小的PID是4

>  handles also start with 4 and are multiples of 4

Status : Running, Suspended（暂停） and Not Responding.

GUI process Not Responding ：一个GUI Process至少有一个线程来处理它的与用户的交互，这个thread会有一个消息队列message queue(只要调用任何UI、GDI函数都会有消息队列)，thread通过GetMessage和PeekMessage获取和送出消息队列（也可能是别的API），超过5秒没有任何类似函数调用，系统就会把这个线程判定为Not Responding状态

UWP processes 在move into the background会被暂停

Non-UWP processes that have no GUI are always shown with a Running status，除非这个进程所有的线程都被暂停

可以使用NtSuspendProcess和NtResumeProcessg暂停/重新运行一个thread

进程的session id ：  Session 0 is used for system ， session 1 and higher are used for interactive logins

## Process Creation

### 大致flow

![image-20210818232503530](README/image-20210818232503530.png)

- 通知CSRSS.exe 有新的进程、线程的创建，Csrss  is a helper to the kernel for managing some aspects of Windows subsystem processes

新进程创建完毕

---

- 新进程初始化：

  - NtDll 为新进程创建  Process Environment Block （PEB） （user-mode management object for the process）
  - 和TEB Thread Environment Block ， user mode management object for the thread
    - PEB TEB都在winternl.h

  > 使用NtCurrentTeb() 访问当前线程TEB，通过NtCurrentTeb()->ProcessEnvironmentBlock访问当前进程PEB

  - process heap创建

  - 创建线程池

  - 加载DLLs (DLL hijacking) ,  kernel32.dll, user32.dll, gdi32.dll , advapi32.dll
    - 使用dumpbin可查看加载的dll和函数
    - C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30037\bin\Hostx64\x86\dumpbin.exe
    - 使用PE Explorer也可以。
    - API Sets的概念：只提供接口，具体的实现可以在别的地方。通过PEB结构体可以查看到和APISets对象的实现。
    - dll的加载顺序
      1. KnownDLLs
         - HKEY_LOCAL_MA-CHINE\SYSTEM\CurrentControlSet\Control\Session Manager\KnownDLLs
      2. The directory of the executable
      3. The current directory of the process
      4. The System directory returned by GetSystemDirectory ， c:\windows\system32
      5. The Windows directory returned by GetWindowsDirectory ， c:\Windows
      6. The directories listed in the PATH environment variable

    - 调用dll的dllmain函数进行初始化

  - 执行exe的入口函数 （entry point）

    - 初始化C/C++ runtime函数，malloc,new,fopen
    - ...
    - 当所有这些初始化操作完成后， C/C + +启动函数就调用应用程序的进入点函数
    - 运行main函数main/wmain/Winmain/wWinmain

    ![image-20210819000647796](README/image-20210819000647796.png)

    这个可以在项目属性设置

    ![image-20210819001047730](README/image-20210819001047730.png)

    GUI/CUI Application的区别在点函数的不同，启动函数的不同。

    

### main Functions

```c++
int main(int argc, const char* argv[]); // const is optional
int wmain(int argc, const wchar_t* argv[]); // const is optional
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR commandLine, int showCmd);
int wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR commandLine, int showCmd);
```

main和wmain是CUI程序的入口点函数。

winMain和wWinMain是GUI程序的入口点函数。

参数说明：

- argc : 参数个数 >= 1
- argv : 参数数组， argv[0] 为 可执行文件的full path
- hInstance :  it’s the address to which the executable is mapped , 这个值由编译器决定
  - HINSTANCE type == HMODULE type(模块句柄)

![image-20210819223633157](README/image-20210819223633157.png)

- hPrevInstance :  represent the HINSTANCE of a previous instance of the same executable ，通常值为NULL

- commandLine ; cmdline ，但是不包括可执行文件路径，string类型，可以使用下面的api转换为数组。

  ```c++
  #include <ShellApi.h>
  LPWSTR* CommandLineToArgvW(_In_ LPCWSTR lpCmdLine, _Out_ int* pNumArgs);
  // 返回一个指针，指向一个p-string数组
  // pNumArgs : 返回的参数个数
  
  // demo	int count;
  PWSTR* args = CommandLineToArgvW(lpCmdLine, &count);
  WCHAR text[1024] = { 0 };
  for (int i = 0; i < count; i++) {
      ::wcscat_s(text, 1024, args[i]);
      ::wcscat_s(text, 1024, L"\n");
  }
  ::LocalFree(args); // 使用LocalFree来释放内存
  ::MessageBox(nullptr, text, L"Command Line Arguments", MB_OK);
  ```

  - 这个api的特性
    - 如果传的参数是一个empty string，返回的就是full executable path
    - 否则返回的就是一个数组，不包括full executable path
  - GetCommandLine 获得cmdline (string) , 可以传递给CommandLineToArgvw

- showCmd ：如何展示这个application的Window

### Process Environment Variables

The names and values are stored in the Registry (like most system data in Windows) --> 环境变量的值是储存在注册表里的，知识盲区

```
user的环境变量：HKEY_CURRENT_USER\Environment
system的环境变量 ：HKEY_LOCAL_MACHINE\System\Current ControlSet\Control\SessionManager\Environment
```

A process receives environment variables from its parent process （copy )

```c
int main(int argc, char* argv[], const char* env[]); // const is optional
int wmain(int argc, wchar_t* argv[], const wchar_t* env[]); // const is optional
```

获得环境变量的key和value

```c++
int main(int argc, const char* argv[], char* env[]) {
	for (int i = 0; ; i++) {
		if (env[i] == nullptr)
			break;
		// printf("[~]%s", env[i]);
		auto equals = strchr(env[i], '='); // 返回匹配到的地址
		// equals是一个地址
		*equals = '\0'; // 把 = 换成 \0
		printf("%s: %s\n", env[i], equals + 1);
		// printf("[!]%s", env[i]);
		*equals = '=';
	}
	return 0;
}
```

- 上面env是一个string数组，每一个元素的格式是xxx=xxx

```c++
PWSTR env = ::GetEnvironmentStrings(); // GetEnvironmentStrings
WCHAR text[8192] = { 0 };
auto p = env;
while (*p) {
	auto equals = wcschr(p, L'=');
	if (equals != p) {
		// eliminate empty names/values
		wcsncat_s(text, p, equals - p);
		wcscat_s(text, L": ");
		wcscat_s(text, equals + 1);
		wcscat_s(text, L"\n");
	}
	p += wcslen(p) + 1;
}
::FreeEnvironmentStrings(env); // 释放内存
```

获得、改变单个环境变量值

```c++
BOOL SetEnvironmentVariable(
	_In_ LPCTSTR lpName,
	_In_opt_ LPCTSTR lpValue);
DWORD GetEnvironmentVariable(
	_In_opt_ LPCTSTR lpName,
	_Out_ LPTSTR lpBuffer,
	_In_ DWORD nSize);
```

GetEnvironmentVariable经典用法

```c++
std::wstring ReadEnvironmentVariable(PCWSTR name) {
	DWORD count = ::GetEnvironmentVariable(name, nullptr, 0); // 先获取字符串的大小
	if (count > 0) {
		std::wstring value; // 申请一个buffer
		value.resize(count);
		::GetEnvironmentVariable(name, const_cast<PWSTR>(value.data()), count);
        // 获得value
		return value;
	}
	return L"";
}
```

把%windir%\\explorer.exe转换为 c:\windows\explorer.exe

```c++
DWORD ExpandEnvironmentStrings(
	_In_ LPCTSTR lpSrc,
	_Out_opt_ LPTSTR lpDst,
	_In_ DWORD nSize);

WCHAR path[MAX_PATH];
::ExpandEnvironmentStrings(L"%windir%\\explorer.exe", path, MAX_PATH); // MAX_PATH是file PATH的值
printf("%ws\n", path); // c:\windows\explorer.exe
```

