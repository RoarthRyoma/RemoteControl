
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
	/// ���¼��ص�ǰ�ļ�������
	/// </summary>
	void LoadFileCurrent();
	/// <summary>
	/// �����ļ���Ϣ
	/// </summary>
	void LoadFileInfo();
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
	/// <summary>
	/// 1.�鿴���̷�����Ϣ
	///	2.�鿴����Ŀ¼��Ϣ
	///	3.���ļ�
	///	4.�����ļ�
	///	5.����¼�
	///	6.������Ļ���� = > ������Ļ��ͼ
	///	7.����
	///	8.����
	/// </summary>
	/// <param name="nCmd">����</param>
	/// <param name="pData">����</param>
	/// <param name="nLength">���ݳ���</param>
	/// <returns>��������ֵ</returns>
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
	// ��ʾ�ļ�
	CListCtrl m_List;
	afx_msg void OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDownloadFile();
	afx_msg void OnDeleteFile();
	afx_msg void OnRunFile();
};
