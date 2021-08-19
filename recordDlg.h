
// recordDlg.h : header file
//
#include <windows.h>   
#include <stdio.h>   
#include <mmsystem.h>   
#include <stdint.h>
#include "G711Codec.h"
#include "packetPS.h"
//#include "audioCapture.h"
#pragma once


// CrecordDlg dialog
class CrecordDlg : public CDialogEx
{
// Construction
public:
	CrecordDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_RECORD_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


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
	afx_msg void OnBnClickedOk();
public:
	void voiceCaptureOn();
	void voiceCaptureOff();

public:
	//HWAVEIN hWaveIn;//Éè±¸¾ä±ú

};
