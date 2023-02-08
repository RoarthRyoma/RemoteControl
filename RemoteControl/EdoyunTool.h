#pragma once
class CEdoyunTool
{
public:
	static void Dump(BYTE* pData, size_t nSize)
	{
		std::string strOut;
		for (size_t i = 0; i < nSize; i++)
		{
			char buffer[8] = "";
			if (i > 0 && i % 16 == 0) strOut += "\n";
			snprintf(buffer, sizeof(buffer), "%02X ", pData[i] & 0xFF);
			strOut += buffer;
		}
		strOut += "\n";
		OutputDebugStringA(strOut.c_str());
	}

	static void ShowError()
	{
		LPWSTR lpMessageBuf = NULL;
		//格式化消息，分配缓存空间
		//获取当前线程最后一次出错时的编号
		//出错描述语言
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMessageBuf, 0, NULL);
		OutputDebugString(lpMessageBuf);
		MessageBox(NULL, lpMessageBuf, _T("发生错误！"), 0);
		LocalFree(lpMessageBuf);
	}

	static bool IsAdmin()
	{
		HANDLE hToken = NULL;
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
		{
			ShowError();
			return false;
		}
		TOKEN_ELEVATION eve;
		DWORD len = 0;
		if (GetTokenInformation(hToken, TokenElevation, &eve, sizeof(eve), &len) == FALSE)
		{
			ShowError();
			return false;
		}
		if (len == sizeof(eve))
		{
			CloseHandle(hToken);
			return eve.TokenIsElevated;
		}
		printf("length of tokeninforation is %d\r\n", len);
		return false;
	}

	static bool RunAsAdmin()
	{
		//本地策略组开启Administrator账户﹑禁止空密码只能登录本地控制台
		STARTUPINFO si = { 0 };
		PROCESS_INFORMATION pi = { 0 };
		TCHAR sPath[MAX_PATH] = _T("");
		GetModuleFileName(NULL, sPath, MAX_PATH);
		//_Param_(3)  => LOGON_WITH_PROFILE 值为 0x00000001
		BOOL ret = CreateProcessWithLogonW(_T("Administrator"), NULL, NULL, LOGON_WITH_PROFILE, NULL, sPath, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
		if (!ret)
		{
			ShowError();
			MessageBox(NULL, sPath, _T("创建进程失败！"), 0);
			return false;
		}
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return true;
	}

	//通过修改开机启动文件来实现开机启动
	static BOOL WriteStartupDir(const CString& strPath)
	{
		TCHAR sPath[MAX_PATH] = _T("");
		GetModuleFileName(NULL, sPath, MAX_PATH);
		return CopyFile(sPath, strPath, FALSE);
	}

	//开机启动的时候，程序的权限是跟随启动用户的
	//如果两者权限不一致，则会导致程序启动失败
	//开机启动对环境变量有影响，如果依赖dll（动态库），则可能启动失败
	//【复制这些dll到system32下面或者syswOW64下面】
	//system32下面，多是64位程序 SysWOW64下面多是32位程序
	//【使用静态库，而非动态库】
	//通过修改注册表来实现开机启动
	static bool WriteRegisterTable(const CString& strPath)
	{
		CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
		TCHAR sPath[MAX_PATH] = _T("");
		GetModuleFileName(NULL, sPath, MAX_PATH);
		BOOL ret = CopyFile(sPath, strPath, FALSE);
		if (ret == FALSE)
		{
			MessageBox(NULL, _T("复制文件失败，是否权限不足?\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
			return false;
		}
		HKEY hKey = NULL;
		ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
		if (ret != ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			MessageBox(NULL, _T("设置自动开机启动失败！是否权限不足? \r\n程序启动失败!"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
			return false;
		}
		ret = RegSetValueEx(hKey, _T("RemoteControl"), 0, REG_EXPAND_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));
		if (ret != ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			MessageBox(NULL, _T("设置自动开机启动失败！是否权限不足? \r\n程序启动失败!"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
			return false;
		}
		RegCloseKey(hKey);
		return true;
	}

	//用于带mfc命令行项目初始化(通用)
	static bool Init()
	{
		HMODULE hModule = ::GetModuleHandle(nullptr);
		if (hModule == nullptr)
		{
			wprintf(L"错误: GetModuleHandle 失败\n");
			return false;
		}
		if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
		{
			// TODO: 在此处为应用程序的行为编写代码。
			wprintf(L"错误: MFC 初始化失败\n");
			return false;
		}
		return true;
	}
};

