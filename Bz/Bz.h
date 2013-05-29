/*
licenced by New BSD License

Copyright (c) 1996-2013, c.mos(original author) & devil.tamachan@gmail.com(Modder)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "options.h"

#include <Shlwapi.h>

/////////////////////////////////////////////////////////////////////////////
// CBZApp:
// See BZ.cpp for the implementation of this class
//

class CBZDocTemplate : public CSingleDocTemplate
{
//	DECLARE_DYNAMIC(CBZDocTemplate)

// Constructors
public:
	CBZDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,
		CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);
public:
	void SetDocument(CDocument* pDoc);
	virtual void RemoveDocument(CDocument* pDoc);
};

class CBZDocManager : public CDocManager
{
public:
	virtual BOOL DoPromptFileName(CString& fileName, UINT nIDSTitle,
			DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate);
};

class CBZApp : public CWinApp
{
public:
	CBZApp();

// Attributes
public:
	BOOL m_bFirstInstance;
	HINSTANCE m_hInstDll; 

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBZApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
private:
	void LoadProfile();
	void WriteProfile();
	void ShortNameToLongName(CString& sShortPath);

	//{{AFX_MSG(CBZApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFilePageSetup();
	afx_msg void OnToolsEditBZDef();
	afx_msg void OnFileSaveDumpList();
	//}}AFX_MSG
	afx_msg void OnLanguage(UINT nID);
	afx_msg void OnUpdateLanguage(CCmdUI* pCmdUI);
	afx_msg void OnFileSaveSelected();
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CBZOptions

enum CharSet { CTYPE_ASCII, CTYPE_SJIS, CTYPE_UNICODE, CTYPE_JIS, CTYPE_EUC, CTYPE_UTF8, CTYPE_EBCDIC, CTYPE_EPWING, CTYPE_COUNT, CTYPE_BINARY = -1 };
enum TextColor{ TCOLOR_ADDRESS, TCOLOR_ADDRESS2, TCOLOR_TEXT, TCOLOR_SELECT, TCOLOR_MARK, TCOLOR_MISMATCH, TCOLOR_STRUCT, TCOLOR_MEMBER, TCOLOR_OVERBROTHER, TCOLOR_HORIZON, TCOLOR_COUNT };
enum MemberColumn { MBRCOL_OFFSET, MBRCOL_LABEL, MBRCOL_VALUE, MBRCOL_MAX };

#define SYSCOLOR 0x80000000
inline BOOL IsSystemColor(COLORREF rgb) { return (rgb & SYSCOLOR) != 0; }
inline COLORREF GetSystemColor(COLORREF rgb) { return (IsSystemColor(rgb)) ? (COLORREF)::GetSysColor(rgb & ~SYSCOLOR) : rgb; }

#define BARSTATE_TOOL 1
#define BARSTATE_STATUS 2
#define BARSTATE_FULLPATH 4
#define BARSTATE_NOFLAT 8

class CBZOptions : public COptions
{
public:
	void Load();
	void Save();

	CharSet charset;
	BOOL bAutoDetect;
	CString sFontName;
	int nFontSize;
	int fFontStyle;
	BOOL bByteOrder;
	CPoint ptFrame;
	int nCmdShow;
	int cyFrame;
	int cyFrame2;
	int cxFrame2;
	int xSplit;
	int ySplit;
	int xSplitStruct;
	BOOL bStructView;
	UINT nSplitView;
	int nComboHeight;
	COLORREF colors[TCOLOR_COUNT][2];
	int  colWidth[MBRCOL_MAX];
	BOOL bLanguage;
	CRect rMargin;
	DWORD dwDetectMax;
	DWORD barState;
	BOOL bReadOnlyOpen;
	int  nBmpWidth;
	int  nBmpZoom;
	DWORD dwMaxOnMemory;
	DWORD dwMaxMapSize;
	BOOL  bTagAll;
	BOOL  bSubCursor;

	CString sDumpHeader;
	int nDumpPage;
	BOOL  bDWordAddr;

	BOOL  bSyncScroll;
	int  iGrid;
	int  nBmpColorWidth;
	int  nBmp8bitPattern;

	BOOL bInspectView;
	BOOL bAnalyzerView;

};

extern CBZOptions options;
CString SeparateByComma(int num, BOOL bSigned = FALSE);
CString SeparateByComma64(ULONGLONG num, BOOL bSigned = FALSE);
CString GetModulePath(LPCSTR pFileName);
CString GetStructFilePath(UINT uID);
LPVOID ReadFile(LPCTSTR pPath);
void ErrorMessageBox();	// ###1.61


class CAboutDlg : public CDialogImpl<CAboutDlg>
{
public:
	enum { IDD = IDD_ABOUTBOX };

	BEGIN_MSG_MAP(CAboutDlg)
		COMMAND_ID_HANDLER_EX(IDOK, OnOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
	END_MSG_MAP()

	void OnOK(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		EndDialog(nID);
	}

	void OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		EndDialog(nID);
	}
};

/////////////////////////////////////////////////////////////////////////////
