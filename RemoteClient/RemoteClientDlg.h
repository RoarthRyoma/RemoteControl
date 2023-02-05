
// RemoteClientDlg.h : header file
//

#pragma once
#include "ClientSocket.h"
#include "StatusDlg.h"

#ifndef WM_SEND_PACK_ACK
#define WM_SEND_PACK_ACK (WM_USER+2)		//���Ͱ�����Ӧ��
#endif

// CRemoteClientDlg dialog
class CRemoteClientDlg : public CDialogEx
{
// Construction
public:
	CRemoteClientDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTECLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
private:
	//CImage m_image;//�����ͼƬ
	//bool m_isFull;//�����Ƿ�������, true-��
	bool m_isClosed;//�����߳��Ƿ�ر�
private:
	/// <summary>
	/// ������ݵ��̺߳���
	/// </summary>
	/// <param name="arg"></param>
	//static void threadEntryForWatchData(void* arg);//����ʹ��this
	/// <summary>
	/// ʵ�ʼ�����ݵķ���
	/// </summary>
	//void threadWatchData();//����ʹ��this,�����������ʵ�������
	/// <summary>
	/// �������߳������ļ�
	/// </summary>
	/// <param name="arg"></param>
	//static void threadEntryForDownFile(void* arg);
	/// <summary>
	/// ���߳���ʵ�������ļ�����
	/// </summary>
	//void threadDownFile();
	/// <summary>
	/// ���¼��ص�ǰ�ļ�������
	/// </summary>
	void LoadFileCurrent();
	/// <summary>
	/// �����ļ���Ϣ
	/// </summary>
	//void LoadFileInfo();
	/// <summary>
	/// ��ȡ·��
	/// </summary>
	/// <param name="hTree"></param>
	/// <returns></returns>
	CString GetPath(HTREEITEM hTree);
	/// <summary>
	/// ɾ�������ӽڵ�
	/// </summary>
	/// <param name="hTree"></param>
	void DeleteTreeChildrenItem(HTREEITEM hTree);
	
	//int SendCommandPacket(int nCmd, BYTE* pData = NULL, size_t nLength = 0, bool bAutoClose = true);

	void DealCommand(WORD nCmd, const std::string& strData, LPARAM lParam);
	void InitUIData();
	void Str2Tree(const std::string& drivers, CTreeCtrl& tree);
	void UpdateFileInfo(const FILEINFO& finfo, HTREEITEM hParent);
	void UpdateDownloadFile(const std::string& strData, FILE* pFile);

protected:
	HICON m_hIcon;
	CStatusDlg m_dlgStatus;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	void LoadFileInfo();

	afx_msg void OnBnClickedBtnTest();
	DWORD m_server_address;
	CString m_port;
	afx_msg void OnBnClickedBtnFileinfo();
	CTreeCtrl m_Tree;
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
//	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMRClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	// ��ʾ�ļ�
	CListCtrl m_List;
	afx_msg void OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDownloadFile();
	afx_msg void OnDeleteFile();
	afx_msg void OnRunFile();
	//afx_msg LRESULT OnSendPacket(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedBtnStartWatch();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//bool isFull() const
	//{
	//	return m_isFull;
	//}

	//void SetImageStatus(bool isFull = false)
	//{
	//	m_isFull = isFull;
	//}
	afx_msg void OnIpnFieldchangedIpaddressServ(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeEditPort();
	afx_msg LRESULT OnSendPackAck(WPARAM wParam, LPARAM lParam);
};
