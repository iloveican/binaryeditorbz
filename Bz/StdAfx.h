// stdafx.h : include file for standard system include files,
//	or project specific include files that are used frequently, but
//		are changed infrequently
//


#define VC_EXTRALEAN		// Windows �w�b�_�[����g�p����Ă��Ȃ����������O���܂��B
#define WINVER 0x0501 //XP
#define _WIN32_WINNT 0x0501 //XP
#define _WIN32_WINDOWS 0x0410 // ����� Windows Me �܂��͂���ȍ~�̃o�[�W���������ɓK�؂Ȓl�ɕύX���Ă��������B
#define _WIN32_IE 0x0600	// ����� IE �̑��̃o�[�W���������ɓK�؂Ȓl�ɕύX���Ă��������B

#define ISOLATION_AWARE_ENABLED 1
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

#include <afxwin.h>         // MFC �̃R�A����ѕW���R���|�[�l���g
#include <afxext.h>         // MFC �̊g������


//#include <afxdisp.h>        // MFC �I�[�g���[�V���� �N���X


#ifndef _AFX_NO_OLE_SUPPORT
//#include <afxdtctl.h>		// MFC �� Internet Explorer 4 �R���� �R���g���[�� �T�|�[�g
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC �� Windows �R���� �R���g���[�� �T�|�[�g
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxpriv.h>

#define _WTL_FORWARD_DECLARE_CSTRING 
#include <atlcoll.h>
#include <atlstr.h>
#include <atlbase.h>
#include <atlwin.h>

#define _WTL_NO_AUTOMATIC_NAMESPACE
#include <atlapp.h>
#include <atldlgs.h>
#include <atlgdi.h>//CDCHandle
#include <atlctrls.h>//CComboBox
#include <atlframe.h>//COwnerDraw
#include <atlcrack.h>
#include <atlmisc.h>
#include <atlddx.h>
#include <atlsplit.h>

#include "..\cmos.h"

#include <imagehlp.h>

namespace BZ { // TODO:HACK!! avoid conflict with atl
#include "MemDC.h"
}

/*
#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif
*/
