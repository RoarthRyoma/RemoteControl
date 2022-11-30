
// RemoteClientDlg.h : header file
//

#pragma once
#include "ClientSocket.h"

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
	/// <summary>
	/// 重新加载当前文件夹内容
	/// </summary>
	void LoadFileCurrent();
	/// <summary>
	/// 加载文件信息
	/// </summary>
	void LoadFileInfo();
	/// <summary>
	/// 获取路径
	/// </summary>
	/// <param name="hTree"></param>
	/// <returns></returns>
	CString GetPath(HTREEITEM hTree);
	/// <summary>
	/// 删除树的子节点
	/// </summary>
	/// <param name="hTree"></param>
	void DeleteTreeChildrenItem(HTREEITEM hTree);
	/// <summary>
	/// 1.查看磁盘分区信息
	///	2.查看分区目录信息
	///	3.打开文件
	///	4.下载文件
	///	5.鼠标事件
	///	6.发送屏幕内容 = > 发送屏幕截图
	///	7.锁机
	///	8.解锁
	/// </summary>
	/// <param name="nCmd">命令</param>
	/// <param name="pData">数据</param>
	/// <param name="nLength">数据长度</param>
	/// <returns>返回命令值</returns>
	int SendCommandPacket(int nCmd, BYTE* pData = NULL, size_t nLength = 0, bool bAutoClose = true);

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnTest();
	DWORD m_server_address;
	CString m_port;
	afx_msg void OnBnClickedBtnFileinfo();
	CTreeCtrl m_Tree;
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
//	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMRClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	// 显示文件
	CListCtrl m_List;
	afx_msg void OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDownloadFile();
	afx_msg void OnDeleteFile();
	afx_msg void OnRunFile();
};
