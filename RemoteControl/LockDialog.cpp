﻿// LockDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteControl.h"
#include "LockDialog.h"
#include "afxdialogex.h"


// CLockDialog 对话框

IMPLEMENT_DYNAMIC(CLockDialog, CDialog)

CLockDialog::CLockDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_INFO, pParent)
{

}

CLockDialog::~CLockDialog()
{
}

void CLockDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLockDialog, CDialog)
END_MESSAGE_MAP()


// CLockDialog 消息处理程序
