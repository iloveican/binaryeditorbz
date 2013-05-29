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


#pragma once
#include "afxcmn.h"
#include "afxwin.h"



// CBZAnalyzerView �t�H�[�� �r���[

class CBZAnalyzerView : public CFormView
{
	DECLARE_DYNCREATE(CBZAnalyzerView)

protected:
	CBZAnalyzerView();           // ���I�����Ŏg�p����� protected �R���X�g���N�^
	virtual ~CBZAnalyzerView();

public:
	enum { IDD = IDD_BZANALYZERVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g

	DECLARE_MESSAGE_MAP()
public:
	CProgressCtrl m_progress;
	CListCtrl m_resultList;
	CComboBox m_combo_analyzetype;
	virtual void OnInitialUpdate();
	afx_msg void OnBnClickedAnalyzeStart();
	afx_msg void OnBnClickedAnalyzerSave();
	afx_msg void OnBnClickedAnalyzerSaveall();

	void Clear();

	unsigned long GetAddress(int nItem);
	BOOL MakeExportDir(LPTSTR pathOutputDir, LPCTSTR pathDstFolder);
	int MakeExportPath(LPTSTR pathOutput, LPCTSTR pathDir, unsigned long ulStartAddr);
	HRESULT SaveFile(LPCTSTR pathOutputDir, unsigned long ulStartAddr, LPBYTE outbuf, unsigned int outbufsize);
};


