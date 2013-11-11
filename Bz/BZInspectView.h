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


//#pragma once
#include "afxwin.h"



// CBZInspectView �t�H�[�� �r���[

class CBZInspectView : public CFormView
{
	DECLARE_DYNCREATE(CBZInspectView)

protected:
	CBZInspectView();           // ���I�����Ŏg�p����� protected �R���X�g���N�^
	virtual ~CBZInspectView();

public:
	enum { IDD = IDD_BZINSPECTVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g

	DECLARE_MESSAGE_MAP()

private:
	bool m_bSigned;
	CBZView* m_pView;
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnInitialUpdate();
	void ClearAll(void);
	CEdit m_editHex;
	CEdit m_edit8bits;
	CEdit m_editBinary1;
	CEdit m_editBinary2;
	CEdit m_editBinary4;
	CEdit m_editBinary8;
	CEdit m_editFloat;
	CEdit m_editDouble;
	afx_msg void OnBnClickedInsIntel();
	afx_msg void OnBnClickedInsSigned();
	CButton m_check_intel;
	CButton m_check_signed;
	CStatic m_staticBinary1;
	CStatic m_staticBinary2;
	CStatic m_staticBinary4;
	CStatic m_staticBinary8;
	void Update(void);
	void UpdateChecks(void);
	void _UpdateChecks(void);
	CEdit m_editFileTime;
	CButton m_buttonCalcsum;
	CEdit m_editCRC16;
	CEdit m_editCRC32;
	CEdit m_editAdler32;
	CEdit m_editMD5;
	CEdit m_editSHA1;
	CEdit m_editSHA256;
	afx_msg void OnClickedInsCalcsum();
	void CBZInspectView::CalculateSums(void);
	void CBZInspectView::ClearSums(void);
};


