#pragma once
#include "afxwin.h"
#include "afxtempl.h"


// CPasteType �_�C�A���O

class CPasteType : public CDialogEx
{
	DECLARE_DYNAMIC(CPasteType)

public:
	CPasteType(CWnd* pParent = NULL);   // �W���R���X�g���N�^�[
	virtual ~CPasteType();

// �_�C�A���O �f�[�^
	enum { IDD = IDD_PASTETYPE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLbnSelchangeCliptypelist();
	CListBox m_FormatList;
	CStatic m_ItemSize;
protected:
	CArray<int> m_formats;
public:
	afx_msg void OnClickedOk();
	afx_msg void OnClickedCancel();
	CButton m_OKButton;
};
