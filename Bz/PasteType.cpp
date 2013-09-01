// PasteType.cpp : �����t�@�C��
//

#include "stdafx.h"
#include "Bz.h"
#include "PasteType.h"
#include "afxdialogex.h"


// CPasteType �_�C�A���O

IMPLEMENT_DYNAMIC(CPasteType, CDialogEx)

CPasteType::CPasteType(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPasteType::IDD, pParent)
{
	m_ItemSize.SetWindowText(_T(""));
	if(!OpenClipboard())
	{
		AfxMessageBox(_T("�N���b�v�{�[�h���J���܂���B"), MB_ICONERROR);
		return;
	}
	int i = 0;
	while((i = EnumClipboardFormats(i)) != 0)
	{
		TCHAR fmtname[256];
		int len = GetClipboardFormatName(i, fmtname, sizeof(fmtname) / sizeof(fmtname[0]) - 1);
		fmtname[len] = '\0';
		m_FormatList.AddString(fmtname);
		m_formats.Add(i);
	}
	CloseClipboard();
}

CPasteType::~CPasteType()
{
}

void CPasteType::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CLIPTYPELIST, m_FormatList);
	DDX_Control(pDX, IDC_CLIPITEMSIZE, m_ItemSize);
	DDX_Control(pDX, IDOK, m_OKButton);
}


BEGIN_MESSAGE_MAP(CPasteType, CDialogEx)
	ON_LBN_SELCHANGE(IDC_CLIPTYPELIST, &CPasteType::OnLbnSelchangeCliptypelist)
	ON_BN_CLICKED(IDOK, &CPasteType::OnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CPasteType::OnClickedCancel)
END_MESSAGE_MAP()


// CPasteType ���b�Z�[�W �n���h���[


void CPasteType::OnLbnSelchangeCliptypelist()
{
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
}


void CPasteType::OnClickedOk()
{
	// �N���b�v�{�[�h���ύX����Ă��Ȃ����m�F�B
	// �{����ClipboardViewer�Ȃǂɓo�^���āA�ύX���ꂽ�Ƃ��ɒʒm���ׂ�����
	// Windows2000 or WindowsXP�`��p�Ȃ̂Ō�����B
	// �t�H�[�}�b�g�ꗗ�������Ă����OK�Ƃ���(���Q�Ȃ��̂�)�B
	if(!OpenClipboard())
	{
		AfxMessageBox(_T("�N���b�v�{�[�h���J���܂���B"), MB_ICONERROR);
		return;
	}
	int i = 0, j = 0;
	while((i = EnumClipboardFormats(i)) != 0)
	{
		if(i != m_formats.GetAt(j))
		{
			break;
		}
		j++;
	}
	CloseClipboard();

	if(i != 0)
	{
		AfxMessageBox(_T("�N���b�v�{�[�h���ύX����Ă��邽�ߓ\��t�����܂���B"), MB_ICONERROR);
		m_OKButton.EnableWindow(FALSE);
		return;
	}

	CDialogEx::OnOK();

	EndDialog(i);
}


void CPasteType::OnClickedCancel()
{
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	CDialogEx::OnCancel();
	EndDialog(0);
}
