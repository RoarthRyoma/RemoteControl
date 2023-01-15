#pragma once
#include <map>
//#include <stdio.h>
#include <list>
#include <atlimage.h>
#include <direct.h>
#include <io.h>
#include "Resource.h"
#include "Packet.h"
#include "EdoyunTool.h"
#include "LockDialog.h"

//#pragma warning(disable:4996) //��������C4996����

class CCommand
{
public:
	CCommand();
	~CCommand();
	int ExecuteCommand(int nCmd, std::list<CPacket>& listPacket, CPacket& inPacket)
	{
		std::map<int, CMDFUNC>::iterator it = m_mapFunction.find(nCmd);
		if (it == m_mapFunction.end())
		{
			return -1;
		}

		return (this->*it->second) (listPacket, inPacket);
	}
	static void RunCommand(void* arg, int status, std::list<CPacket>& list, CPacket& inPacket)
	{
		CCommand* that = (CCommand*)arg;
		if (status > 0)
		{
			int ret = that->ExecuteCommand(status, list, inPacket);
			if (ret != 0)
			{
				TRACE("ִ������ʧ��:%d ret=%d\r\n", status, ret);
			}
		}
		else
		{
			MessageBox(NULL, _T("�޷����������û�,�Զ�������! "), _T("�����û�ʧ��"), MB_OK | MB_ICONERROR);
		}
	}
protected:
	typedef int(CCommand::* CMDFUNC)(std::list<CPacket>& listPacket, CPacket& inPacket);//��Ա����ָ��
	std::map<int, CMDFUNC> m_mapFunction;//������ŵ����ܵ�ӳ��
	CLockDialog dlg;
	unsigned threadid;
protected:
	static unsigned _stdcall ThreadLockDlg(void* arg)
	{
		CCommand* that = (CCommand*)arg;
		that->threadLockDlgMain();
		_endthreadex(0);
		return 0;
	}

	void threadLockDlgMain()
	{
		TRACE("%s(%d)%d\r\n", __FUNCTION__, __LINE__, GetCurrentThreadId());

		dlg.Create(IDD_DIALOG_INFO, NULL);
		dlg.ShowWindow(SW_SHOW);
		//�ڱκ�̨����
		CRect rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = GetSystemMetrics(SM_CXFULLSCREEN);//w1
		rect.bottom = (LONG)(GetSystemMetrics(SM_CYFULLSCREEN) * 1.1);//h1
		TRACE("right = %d bottom = %d\r\n", rect.right, rect.bottom);
		dlg.MoveWindow(rect);
		CWnd* pText = dlg.GetDlgItem(IDC_STATIC);
		if (pText)
		{
			CRect rtText;
			pText->GetWindowRect(rtText);
			int nWidth = rtText.Width();//w0;
			int nHeight = rtText.Height();//h0
			int x = (rect.right - nWidth) / 2;
			int y = (rect.bottom - nHeight) / 2;
			pText->MoveWindow(x, y, rtText.Width(), rtText.Height());
		}
		//�����ö�
		dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		//�������λ��
		ShowCursor(false);
		//����������
		::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);
		//���������Χ
		dlg.GetWindowRect(rect);
		ClipCursor(rect);
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_KEYDOWN)
			{
				TRACE("msg:%08X lparam:%08X wparam:%08X\r\n", msg.message, msg.wParam, msg.lParam);
				if (msg.wParam == 0x1B)//��ESC�˳�
				{
					break;
				}
			}
		}
		ClipCursor(NULL);
		::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);
		ShowCursor(true);
		dlg.DestroyWindow();
	}

	//��ȡ������Ϣ
	int MakeDriveInfo(std::list<CPacket>& listPacket, CPacket& inPacket)//1->A  2->B  3->C ... 26->Z
	{
		std::string result;
		for (int i = 1; i <= 26; i++)
		{
			if (_chdrive(i) == 0)
			{
				if (result.size() > 0) result += ',';
				result += 'A' + i - 1;
			}
		}
		listPacket.push_back(CPacket(1, (BYTE*)result.c_str(), result.size()));
		//CPacket pack(1, (BYTE*)result.c_str(), result.size());//�������
		//CEdoyunTool::Dump((BYTE*)pack.Data(), pack.Size());
		//CServerSocket::getInstance()->Send(CPacket(1, (BYTE*)result.c_str(), result.size()));
		//CServerSocket::getInstance()->Send(pack);
		return 0;
	}

	//��ȡ���̷���Ŀ¼
	int MakeDirectoryInfo(std::list<CPacket>& listPacket, CPacket& inPacket)
	{
		std::string strPath = inPacket.strData;
		//std::list<FILEINFO> lstFileInfo;
		//bool flag = CServerSocket::getInstance()->GetFilePath(strPath);
		//if (!flag)
		//{
		//	OutputDebugString(_T("����������󣡵�ǰ����ǻ�ȡ�ļ��б�"));
		//	return -1;
		//}
		if (_chdir(strPath.c_str()) != 0)
		{
			FILEINFO finfo;
			//finfo.IsInvalid = TRUE;
			//finfo.IsDirectory = TRUE;
			finfo.HasNext = FALSE;
			//memcpy(finfo.szFileName, strPath.c_str(), strPath.size());
			//lstFileInfo.push_back(finfo);
			listPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
			/*CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
			CServerSocket::getInstance()->Send(pack);*/
			OutputDebugString(_T("û��Ȩ�޷��ʵ�ǰĿ¼��"));
			return -2;
		}
		_finddata_t fdata;
		int hfind = 0;
		if ((hfind = _findfirst("*", &fdata)) == -1)// * ƥ�������ļ�
		{
			OutputDebugString(_T("û���ҵ��κ��ļ���"));
			FILEINFO finfo;
			finfo.HasNext = FALSE;
			listPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
			/*CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
			CServerSocket::getInstance()->Send(pack);*/
			return -3;
		}
		int COUNT = 0;
		do
		{
			FILEINFO finfo;
			finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
			memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
			//lstFileInfo.push_back(finfo);
			TRACE("file info: %s \r\n", finfo.szFileName);
			listPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
			/*CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
			CServerSocket::getInstance()->Send(pack);*/
			COUNT++;
		} while (!_findnext(hfind, &fdata));
		//������Ϣ�����ƶ�
		FILEINFO finfo;
		finfo.HasNext = FALSE;
		listPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
		/*CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
		CServerSocket::getInstance()->Send(pack);*/
		TRACE("server COUNT: %d\r\n", COUNT);
		return 0;
	}

	int RunFile(std::list<CPacket>& listPacket, CPacket& inPacket)
	{
		std::string strPath = inPacket.strData;
		//CServerSocket::getInstance()->GetFilePath(strPath);
		ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);

		//�������
		/*CPacket pack(3, NULL, 0);
		CServerSocket::getInstance()->Send(pack);*/
		listPacket.push_back(CPacket(3, NULL, 0));
		return 0;
	}

	int DownloadFile(std::list<CPacket>& listPacket, CPacket& inPacket)
	{
		std::string strPath = inPacket.strData;
		//CServerSocket::getInstance()->GetFilePath(strPath);
		long long data = 0;
		//FILE* pFile = fopen(strPath.c_str(), "rb");
		FILE* pFile = NULL;
		errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
		//if (pFile == NULL)
		//{
		//	CPacket pack(4, (BYTE*)&data, 8);
		//	CServerSocket::getInstance()->Send(pack);
		//	return -1;
		//}
		if (err != 0)
		{
			/*CPacket pack(4, (BYTE*)&data, 8);
			CServerSocket::getInstance()->Send(pack);*/
			listPacket.push_back(CPacket(4, (BYTE*)&data, 8));
			return -1;
		}
		if (pFile != NULL)
		{
			fseek(pFile, 0, SEEK_END);
			data = _ftelli64(pFile);
			/*CPacket head(4, (BYTE*)&data, 8);
			CServerSocket::getInstance()->Send(head);*/
			listPacket.push_back(CPacket(4, (BYTE*)&data, 8));
			fseek(pFile, 0, SEEK_SET);

			char buffer[1024]{};
			size_t rlen = 0;
			do
			{
				rlen = fread(buffer, 1, 1024, pFile);
				/*CPacket pack(4, (BYTE*)buffer, rlen);
				CServerSocket::getInstance()->Send(pack);*/
				listPacket.push_back(CPacket(4, (BYTE*)buffer, rlen));
			} while (rlen >= 1024);
			fclose(pFile);
		}
		/*CPacket pack(4, NULL, 0);
		CServerSocket::getInstance()->Send(pack);*/
		listPacket.push_back(CPacket(4, NULL, 0));
		return 0;
	}

	int MouseEvent(std::list<CPacket>& listPacket, CPacket& inPacket)
	{
		MOUSEEV mouse;
		memcpy(&mouse, inPacket.strData.c_str(), sizeof(MOUSEEV));
		//if (CServerSocket::getInstance()->GetMouseEvent(mouse))
		//{
			DWORD nFlags = 0;
			switch (mouse.nButton)
			{
			case 0://���
				nFlags = 1;
				break;
			case 1://�Ҽ�
				nFlags = 2;
				break;
			case 2://�м�
				nFlags = 4;
				break;
			case 4://û�а����κμ�
				nFlags = 8;
				break;
			default:
				break;
			}

			if (nFlags != 8)
			{
				SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
			}

			switch (mouse.nAction)
			{
			case 0://����
				nFlags |= 0x10;
				break;
			case 1://˫��
				nFlags |= 0x20;
				break;
			case 2://����
				nFlags |= 0x40;
				break;
			case 3://�ſ�
				nFlags |= 0x80;
				break;
			default:
				break;
			}
			TRACE("mouse event : %08X, x=%d y=%d\r\n", nFlags, mouse.ptXY.x, mouse.ptXY.y);
			switch (nFlags)
			{
			case 0x21://���˫��
				//GetMessageExtraInfo => ��ȡ��ǰ�̵߳Ķ�����Ϣ,ϵͳAPI,���簴��Ctrl����������Ĳ���
				mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
				mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			case 0x11://�������
				mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
				mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
				break;
			case 0x41://�������
				mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
				break;
			case 0x81://����ſ�
				mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
				break;
			case 0x22://�Ҽ�˫��
				mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
				mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			case 0x12://�Ҽ�����
				mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
				mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
				break;
			case 0x42://�Ҽ�����
				mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
				break;
			case 0x82://�Ҽ��ſ�
				mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
				break;
			case 0x24://�м�˫��
				mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
				mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			case 0x14://�м�����
				mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
				mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
				break;
			case 0x44://�м�����
				mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
				break;
			case 0x84://�м��ſ�
				mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
				break;
			case 0x08://����ƶ�
				mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
				break;
			default:
				break;
			}

			//�������
			/*CPacket pack(5, NULL, 0);
			CServerSocket::getInstance()->Send(pack);*/
			listPacket.push_back(CPacket(5, NULL, 0));
		//}
		//else
		//{
		//	OutputDebugString(_T("��ȡ����������ʧ��!"));
		//	return -1;
		//}

		return 0;
	}

	int SendScreen(std::list<CPacket>& listPacket, CPacket& inPacket)
	{
		CImage screen;//GDI  ͼ���豸�ӿ�(Graphics Device Interface)
		HDC hScreen = ::GetDC(NULL);
		int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);
		int nWidth = GetDeviceCaps(hScreen, HORZRES);
		int nHeight = GetDeviceCaps(hScreen, VERTRES);
		screen.Create(nWidth, nHeight, nBitPerPixel);
		BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);
		ReleaseDC(NULL, hScreen);

		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
		if (hMem == NULL) return -1;
		IStream* pStream = NULL;
		HRESULT hRet = CreateStreamOnHGlobal(hMem, TRUE/*�Լ������ڴ�ʱʹ��FALSE*/, &pStream);
		if (hRet == S_OK)
		{
			screen.Save(pStream, Gdiplus::ImageFormatJPEG);
			LARGE_INTEGER bg = { 0 };
			pStream->Seek(bg, STREAM_SEEK_SET, NULL);
			PBYTE pData = (PBYTE)GlobalLock(hMem);
			SIZE_T nSize = GlobalSize(hMem);
			/*CPacket pack(6, pData, nSize);
			CServerSocket::getInstance()->Send(pack);*/
			listPacket.push_back(CPacket(6, pData, nSize));
			GlobalUnlock(hMem);
		}
		pStream->Release();
		GlobalFree(hMem);
		//screen.Save(_T("test2022.jpg"), Gdiplus::ImageFormatJPEG);

		//ULONGLONG tick = GetTickCount64();
		//screen.Save(_T("test2022.png"), Gdiplus::ImageFormatPNG);
		//TRACE("png: %d\r\n", GetTickCount64() - tick);
		//tick = GetTickCount64();
		//screen.Save(_T("test2022.jpg"), Gdiplus::ImageFormatJPEG);
		//TRACE("jpeg: %d\r\n", GetTickCount64() - tick);
		screen.ReleaseDC();
		return 0;
	}

	int LockMachine(std::list<CPacket>& listPacket, CPacket& inPacket)
	{
		if ((dlg.m_hWnd == NULL) || (dlg.m_hWnd == INVALID_HANDLE_VALUE))
		{
			//_beginthread(ThreadLockDlg, 0, NULL);
			_beginthreadex(NULL, 0, &CCommand::ThreadLockDlg, this, 0, &threadid);
			TRACE("threadid - %d\r\n", threadid);
		}
		/*CPacket pack(7, NULL, 0);
		CServerSocket::getInstance()->Send(pack);*/
		listPacket.push_back(CPacket(7, NULL, 0));
		return 0;
	}

	int UnlockMachine(std::list<CPacket>& listPacket, CPacket& inPacket)
	{
		//dlg.SendMessage(WM_KEYDOWN, 0x1B, 0x00010001);
		//::SendMessage(dlg.m_hWnd, WM_KEYDOWN, 0x1B, 0x00010001);
		PostThreadMessage(threadid, WM_KEYDOWN, 0x1B, 0);
		/*CPacket pack(8, NULL, 0);
		CServerSocket::getInstance()->Send(pack);*/
		listPacket.push_back(CPacket(8, NULL, 0));
		return 0;
	}

	int TestConnect(std::list<CPacket>& listPacket, CPacket& inPacket)
	{
		/*CPacket pack(1981, NULL, 0);
		bool ret = CServerSocket::getInstance()->Send(pack);
		TRACE(" test Send ret: %d\r\n", ret);*/
		listPacket.push_back(CPacket(1981, NULL, 0));
		return 0;
	}

	int DeleteLocalFile(std::list<CPacket>& listPacket, CPacket& inPacket)
	{
		std::string strPath = inPacket.strData;
		//CServerSocket::getInstance()->GetFilePath(strPath);
		TCHAR sPath[MAX_PATH] = _T("");

		/*//������������
		mbstowcs(sPath, strPath.c_str(), strPath.size());*/
		//WindowsAPI������ֽںͶ��ֽڵ�ת��
		MultiByteToWideChar(CP_ACP, 0, strPath.c_str(), strPath.size(), sPath, sizeof(sPath) / sizeof(TCHAR));
		DeleteFile(sPath);
		/*CPacket pack(9, NULL, 0);
		bool ret = CServerSocket::getInstance()->Send(pack);
		TRACE(" delete file Send ret: %d\r\n", ret);*/
		listPacket.push_back(CPacket(9, NULL, 0));
		return 0;
		/*//����C4996��ȫ�����д��
		size_t converted = strPath.size() + 1;
		errno_t err = mbstowcs_s(&converted, sPath, converted, strPath.c_str(), _TRUNCATE);
		if (err == 0)
		{
			DeleteFile(sPath);
			CPacket pack(9, NULL, 0);
			bool ret = CServerSocket::getInstance()->Send(pack);
			TRACE(" delete file Send ret: %d\r\n", ret);
			return 0;
		}
		else
		{
			TRACE("delete file failed\r\n");
			return -1;
		}*/
	}
};

