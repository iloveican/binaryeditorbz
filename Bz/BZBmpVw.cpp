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

#include "stdafx.h"
#include "BZ.h"
#include "BZView.h"
#include "BZBmpVw.h"
#include "BZDoc.h"
#include "Splitter.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBZBmpView

IMPLEMENT_DYNCREATE(CBZBmpView, CScrollView)

CBZBmpView::CBZBmpView()
{
	m_hBmp = NULL;
	m_lpbi = NULL;
	m_tooltipLastAddress = 0xffffffff;
	m_isLButtonDown = false;
	m_isShowingCaretPos = FALSE;
}

CBZBmpView::~CBZBmpView()
{
	if(m_hBmp) ::DeleteObject(m_hBmp);
	if(m_lpbi) MemFree(m_lpbi);
}


BEGIN_MESSAGE_MAP(CBZBmpView, CScrollView)
	//{{AFX_MSG_MAP(CBZBmpView)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_VSCROLL()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(ID_BMPVIEW_WIDTH128, ID_BMPVIEW_ZOOM, OnBmpViewMode)
	ON_WM_SETCURSOR()
	ON_COMMAND_RANGE(ID_BMPVIEW_8BITCOLOR, ID_BMPVIEW_8BITCOLOR_PAT3, OnBmpViewColorWidth)
	ON_WM_SIZE()
	ON_COMMAND(ID_BMPVIEW_ADDRESSTOOLTIP, OnBmpViewAddressTooltip)
	ON_COMMAND(ID_BMPVIEW_GOTOCARET, OnBmpViewGotoCaret)
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBZBmpView drawing

void MakeBzPallet256(DWORD *pRGB)
{
	*pRGB++ = 0xFFFFFF;
	for_to(i, 31) *pRGB++ = 0x00FFFF;
	for_to_(i, 128-32) *pRGB++ = 0xFF0000;
	for_to_(i, 128) *pRGB++ = 0x000000;
}

void MakeRedPallet256(DWORD *pRGB)
{
	for(unsigned int i=0; i<=0xff; i++)
	{
		*pRGB++ = 0 | (i&0xff)<<16;
	}
}


void MakeSafetyPallet256(DWORD *pRGB)
{
	// safety pallet http://msdn.microsoft.com/en-us/library/bb250466%28VS.85%29.aspx
	DWORD* pRGBorig = pRGB;

//	*pRGB++ = 0xFFFFFF;
	for(unsigned int r=0; r<=0xff; r+=0x33)
		for(unsigned int g=0; g<=0xff; g+=0x33)
			for(unsigned int b=0; b<=0xff; b+=0x33)
				*pRGB++ = b|(g<<8)|(r<<16);
	for(unsigned int gr=0; gr<=0xffffff; gr+=0x111111)
		*pRGB++ = gr;
	*pRGB++ = 0xC0C0C0;
	*pRGB++ = 0x808080;
	*pRGB++ = 0x800000;
	*pRGB++ = 0x800080;
	*pRGB++ = 0x008000;
	*pRGB++ = 0x008080;
	pRGBorig[255] = 0xffffff;
//	TRACE("pallet[0]=0x%x, [255]=0x%x\n", ((DWORD*)(m_lpbi+1))[0], ((DWORD*)(m_lpbi+1))[255]);
}

void Make8bitBITMAPINFOHEADER(LPBITMAPINFOHEADER lpbi, LONG w, LONG h)
{
//	lpbi->biSize = sizeof(BITMAPINFOHEADER);
	lpbi->biWidth = w;
	lpbi->biHeight = h;
	lpbi->biPlanes = 1;
	lpbi->biBitCount = options.nBmpColorWidth;//8;
	lpbi->biCompression = BI_RGB;
	lpbi->biSizeImage = 0;
	lpbi->biClrUsed = 0;
	lpbi->biClrImportant = 0;
}

void CBZBmpView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CBZDoc* pDoc = (CBZDoc*)GetDocument();
	DWORD dwTotal = pDoc->GetDocSize();
	if(dwTotal < (DWORD)options.nBmpWidth) return;

	if(!m_lpbi) 
		m_lpbi = (LPBITMAPINFOHEADER)MemAlloc(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*256/*256pallet*/);

	m_lpbi->biSize = sizeof(BITMAPINFOHEADER);
	m_cBmp.cx = options.nBmpWidth;
	m_cBmp.cy = dwTotal / (options.nBmpWidth * (options.nBmpColorWidth/8));
	Make8bitBITMAPINFOHEADER(m_lpbi, m_cBmp.cx, -m_cBmp.cy/*top-down DIB*/);


	DWORD* pRGB = (DWORD*)(m_lpbi+1);
	switch(options.nBmp8bitPattern)
	{
	case 0:
		MakeBzPallet256(pRGB);
		break;
	case 1:
		MakeSafetyPallet256(pRGB);
		break;
	case 2:
		MakeRedPallet256(pRGB);
		break;
	}

	CDC* pDC = GetDC();
	HDC hDC = pDC->m_hDC;
	if(m_hBmp) {
		::DeleteObject(m_hBmp);
		m_hBmp = NULL;
	}

#ifdef FILE_MAPPING
	if(!pDoc->IsFileMapping())
#endif //FILE_MAPPING
	{
		m_hBmp = ::CreateDIBitmap(hDC, m_lpbi, CBM_INIT, pDoc->GetDocPtr(), (LPBITMAPINFO)m_lpbi, DIB_RGB_COLORS);
		if(!m_hBmp)
			ErrorMessageBox();
	}

	// TODO: calculate the total size of this view
	CSize cView = m_cBmp;
	cView.cx = cView.cx * options.nBmpZoom + BMPSPACE*2;
	cView.cy = cView.cy * options.nBmpZoom + BMPSPACE*2;
	SetScrollSizes(MM_TEXT, cView);
	
	TRACE("cView.cy=%X\n", GetTotalSize().cy);

	CSplitterWnd* pSplit = (CSplitter*)GetParent();
	ASSERT(pSplit->IsKindOf(RUNTIME_CLASS(CSplitterWnd)));
	cView.cx = options.nBmpWidth * options.nBmpZoom + BMPSPACE*2 + GetSystemMetrics(SM_CXVSCROLL)+1;
	pSplit->SetColumnInfo(0, cView.cx, 0);
	// MemFree(lpbi);

	if(m_tooltip.m_hWnd!=NULL)m_tooltip.DestroyWindow();
	m_tooltip.Create(m_hWnd, NULL, NULL, TTS_BALLOON|TTS_NOFADE|TTS_NOANIMATE/*|TTS_ALWAYSTIP*/); // TTS_ALWAYSTIP���w�肷��ƃo���[�������Ȃ��Ȃ�B
	m_tooltip.SetDelayTime(TTDT_RESHOW, 0);
	m_tooltip.SetDelayTime(TTDT_AUTOPOP, 0xffff);
	m_tooltip.SetDelayTime(TTDT_INITIAL, 0);
	m_tooltip.Activate(TRUE);
	WTL::CToolInfo toolinfo(TTF_SUBCLASS|TTF_TRANSPARENT, m_hWnd, 0, 0, _T(""));
	m_tooltip.AddTool(toolinfo);
}


BOOL CBZBmpView::OnEraseBkgnd(CDC* pDC)
{
	return false;
}

void CBZBmpView::OnDraw(CDC* pDC)
{
	pDC->SetBkColor(RGB(220,220,220));
	BZ::CMemDC pMemDC(pDC);
	if(m_hBmp) {
		HDC hDC = pMemDC->m_hDC;
		HDC hDCSrc = ::CreateCompatibleDC(hDC);
		HBITMAP hBmpOld = (HBITMAP)::SelectObject(hDCSrc, m_hBmp);
		if(options.nBmpZoom == 1)
			::BitBlt(hDC, BMPSPACE/*dstX*/, BMPSPACE/*dstY*/, m_cBmp.cx/*dstW*/, m_cBmp.cy/*dstH*/
					, hDCSrc, 0/*srcX*/, 0/*srcY*/, SRCCOPY);
		else
			::StretchBlt(hDC, BMPSPACE/*dstX*/, BMPSPACE/*dstY*/
				, m_cBmp.cx * options.nBmpZoom/*dstW*/, m_cBmp.cy * options.nBmpZoom/*dstH*/
				, hDCSrc, 0/*srcX*/, 0/*srcY*/, m_cBmp.cx/*srcW*/, m_cBmp.cy/*srcH*/, SRCCOPY);

		::SelectObject(hDCSrc, hBmpOld);
		//::DeleteDC(hDC);
	} else {
		CBZDoc* pDoc = (CBZDoc*)GetDocument();
		ASSERT(pDoc);
		if(pDoc->GetDocPtr()==NULL)return;

		CRect rClip;
		pMemDC->GetClipBox(rClip);

		rClip.top -= rClip.top % options.nBmpZoom;
		rClip.bottom += rClip.bottom % options.nBmpZoom;

		int nSpaceTop = (rClip.top < BMPSPACE) ? BMPSPACE - rClip.top : 0;
		long nBottom = m_cBmp.cy * options.nBmpZoom + BMPSPACE;
		if(rClip.bottom >= nBottom)
			rClip.bottom = nBottom;

		int nBmpHeight = (rClip.Height() - nSpaceTop) / options.nBmpZoom;
		m_lpbi->biHeight = -nBmpHeight;

		ATLTRACE("Clip: left=%ld, top=%ld(0x%08lX) %dx%d\n", rClip.left, rClip.top, rClip.top, rClip.Width(), rClip.Height());
		DWORD dwOffset = ((DWORD)rClip.top - (DWORD)(BMPSPACE - nSpaceTop)) / (DWORD)options.nBmpZoom * (DWORD)m_cBmp.cx;
		ATLTRACE("DWORD dwOffset 0x%08X = ((DWORD)rClip.top 0x%08X - (DWORD)(BMPSPACE 0x%X - nSpaceTop 0x%X)) * (DWORD)m_cBmp.cx 0x%08X / (DWORD)options.nBmpZoom 0x%08X;\n", dwOffset, (DWORD)rClip.top, BMPSPACE, nSpaceTop, m_cBmp.cx, options.nBmpZoom);
		dwOffset*=(DWORD)(options.nBmpColorWidth/8);
		ATLTRACE("dwOffset 0x%08X *=(DWORD)(options.nBmpColorWidth %ld /8);\n", dwOffset, options.nBmpColorWidth);

#ifdef FILE_MAPPING
		//pDoc->QueryMapView(pDoc->GetDocPtr(), dwOffset);
		DWORD dwIdeaSize = m_cBmp.cx * nBmpHeight * (options.nBmpColorWidth/8);
		LPBYTE lpBits = pDoc->QueryMapViewTama2(dwOffset, dwIdeaSize);
		ASSERT(pDoc->GetMapRemain(dwOffset) >= dwIdeaSize);
#elif
		LPBYTE lpBits = pDoc->GetDocPtr() + dwOffset;
#endif //FILE_MAPPING

		::StretchDIBits(pMemDC->m_hDC, BMPSPACE/*dstX*/, rClip.top + nSpaceTop/*dstY*/
				, m_cBmp.cx * options.nBmpZoom/*dstW*/, nBmpHeight * options.nBmpZoom/*dstH*/
				, 0/*srcX*/, 0/*srcY*/, m_cBmp.cx/*srcW*/, nBmpHeight/*srcH*/
				, lpBits/*srcPointer*/ , (LPBITMAPINFO)m_lpbi, DIB_RGB_COLORS, SRCCOPY);

	}
}

/////////////////////////////////////////////////////////////////////////////
// CBZBmpView diagnostics

#ifdef _DEBUG
void CBZBmpView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CBZBmpView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBZBmpView message handlers

void CBZBmpView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_isLButtonDown = true;
	m_isShowingCaretPos = FALSE;

	point += GetScrollPosition();
	point.x -= BMPSPACE;
	point.y -= BMPSPACE;
	point.x /= options.nBmpZoom;
	point.y /= options.nBmpZoom;
	if(point.x >= 0 && point.x < options.nBmpWidth && point.y >= 0) {
		DWORD dwPtr = point.y*(options.nBmpWidth * (options.nBmpColorWidth/8)) + (point.x * (options.nBmpColorWidth/8));
		CBZView* pView = (CBZView*)GetNextWindow();
		if(dwPtr < pView->m_dwTotal) {
			pView->m_dwCaret = dwPtr;
			pView->GotoCaret();
			pView->Activate();
		}
	}

//	CScrollView::OnLButtonDown(nFlags, point);
}
void CBZBmpView::OnMouseMove(UINT nFlags, CPoint point)
{
	point += GetScrollPosition();
	point.x -= BMPSPACE;
	point.y -= BMPSPACE;
	point.x /= options.nBmpZoom;
	point.y /= options.nBmpZoom;
	if(point.x >= 0 && point.x < options.nBmpWidth && point.y >= 0) {
		DWORD currentAddress = point.y*(options.nBmpWidth * (options.nBmpColorWidth/8)) + (point.x * (options.nBmpColorWidth/8));
		if(currentAddress != m_tooltipLastAddress)
		{
			CBZView* pView = (CBZView*)GetNextWindow();
			if(currentAddress < pView->m_dwTotal) {
				if(m_isLButtonDown)
				{
					pView->m_dwCaret = currentAddress;
					pView->GotoCaret();
					//pView->Activate();
				} else if(options.bAddressTooltip && !m_isShowingCaretPos) {
					TCHAR tmp[22];
					wsprintf(tmp, _T("0x%08X"), currentAddress);
					WTL::CToolInfo toolinfo(TTF_SUBCLASS|TTF_TRANSPARENT, m_hWnd, 0, 0, tmp);
					m_tooltip.SetToolInfo(toolinfo);
					ATLTRACE(_T("UpdateTooltip: %08X, %08X\n"), currentAddress, m_tooltipLastAddress);
					m_tooltipLastAddress = currentAddress;
					m_tooltip.Activate(true);
					m_tooltip.Popup();
					//CScrollView::OnMouseMove(nFlags, point);
					return;
				}
			}
		} else {
				ATLTRACE(_T("!!!UpdateTooltip: %08X, %08X\n"), currentAddress, m_tooltipLastAddress);
				return;
		}
	}

	if(!m_isShowingCaretPos)
	{
		m_tooltipLastAddress = 0xffffffff;
		m_tooltip.Activate(false);
		m_tooltip.Pop();
	}
	//CScrollView::OnMouseMove(nFlags, point);
}
void CBZBmpView::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_isLButtonDown = false;
	//CScrollView::OnLButtonUp(nFlags, point);
}

static void getBmpPointFromAddr(DWORD currentAddress, CPoint &point)
{
	// ScrollView�S�̂ɂ�����\�����������W
	point.x = (currentAddress % (options.nBmpWidth * options.nBmpColorWidth / 8)) * options.nBmpZoom + BMPSPACE;
	point.y = (currentAddress / (options.nBmpWidth * options.nBmpColorWidth / 8)) * options.nBmpZoom + BMPSPACE;
}

void CBZBmpView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// ���OnVScroll���Ă΂Ȃ��ƁAGetScrollInfo��GetScrollPosition�ŃX�N���[���O�̒l���Ԃ��Ă��Ă��܂��B
	CScrollView::OnVScroll(nSBCode, nPos, pScrollBar);

	ATLTRACE("OnVScroll\n");
	// TODO: Add your message handler code here and/or call default
	if(nSBCode == SB_THUMBTRACK) {		// ### 1.54
		SCROLLINFO si;
		GetScrollInfo(SB_VERT, &si, SIF_TRACKPOS);
		TRACE("nPos, nTrackPos=%u, %d\n", nPos, si.nTrackPos);
		nPos = si.nTrackPos;
		TRACE("nPos, nTrackPos=%u, %d\n", nPos, si.nTrackPos);

		if(m_isShowingCaretPos)
		{
			DWORD currentAddress = getCaretPos();
			CPoint point;
			getBmpPointFromAddr(currentAddress, point);
			// point���|�b�v�A�b�v���������W�ɍX�V
			point -= GetScrollPosition();

			// ScrollView�Ō����Ă���傫��
			CRect cr;
			GetClientRect(cr);

			if(cr.PtInRect(point)) // if in area
			{
				// �X�N���[�����ĉB��Ă������ɔ����ĕ\��������
				CString tmp;
				tmp.Format(_T("0x%08X"), currentAddress);
				WTL::CToolInfo toolinfo(TTF_SUBCLASS|TTF_TRANSPARENT|TTF_TRACK, m_hWnd, 0, 0, const_cast<LPTSTR>(tmp.GetString()));

				m_tooltip.Activate(TRUE);
				m_tooltip.Popup();
				m_tooltip.TrackActivate(toolinfo, TRUE);

				// �K�؂Ȉʒu�Ɉړ�
				ClientToScreen(&point);
				m_tooltip.TrackPosition(point.x, point.y);
			} else	// if not in area
			{
				m_tooltip.Activate(FALSE);
				m_tooltip.Pop();
			}
		} else if(options.bAddressTooltip)
		{
			CPoint point = GetScrollPosition();
			point.x = max(point.x-BMPSPACE, 0);
			point.y = max(point.y-BMPSPACE, 0);
			point.x /= options.nBmpZoom;
			point.y /= options.nBmpZoom;
			DWORD currentAddress = point.y*(options.nBmpWidth * (options.nBmpColorWidth/8)) + (point.x * (options.nBmpColorWidth/8));
			TCHAR tmp[22];
			wsprintf(tmp, _T("0x%08X"), currentAddress);
			WTL::CToolInfo toolinfo(TTF_SUBCLASS|TTF_TRANSPARENT, m_hWnd, 0, 0, tmp);
			m_tooltip.SetToolInfo(toolinfo);
			m_tooltip.Activate(true);
			m_tooltip.Popup();
		}
	} else if(nSBCode == SB_THUMBPOSITION) {
		if(!m_isShowingCaretPos)
		{
			m_tooltip.Activate(false);
			m_tooltip.Pop();
		}
	}
}

BOOL CBZBmpView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	//*
	// IMPLEMENT 1: Standard...
	this->SendMessage(WM_VSCROLL, zDelta > 0 ? SB_LINEUP : SB_LINEDOWN, 0);
	//*/

	/*
	// IMPLEMENT 2: Too many move
	WPARAM wParam = zDelta > 0 ? SB_LINEUP : SB_LINEDOWN;
	short delta = abs(zDelta / WHEEL_DELTA);
	for(int i = 0; i < delta; i++)
	{
		this->SendMessage(WM_VSCROLL, wParam, 0);
	}
	//*/

	/*
	// IMPLEMENT 3: Many move
	CRect rect;
	GetClientRect(rect);
	ScrollToPosition(GetScrollPosition() + CPoint(0, static_cast<int>((double)-zDelta / WHEEL_DELTA * rect.Size().cy)));
	//*/

	return CScrollView::OnMouseWheel(nFlags, zDelta, pt);
}


// ###1.54c

void CBZBmpView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CMenu menu;
	menu.LoadMenu(IDR_BMPVIEW);
	CMenu* pMenu = menu.GetSubMenu(0);
	switch(options.nBmpWidth)
	{
	case 128:
		pMenu->CheckMenuItem(ID_BMPVIEW_WIDTH128, MF_BYCOMMAND | MF_CHECKED);
		break;
	case 256:
		pMenu->CheckMenuItem(ID_BMPVIEW_WIDTH256, MF_BYCOMMAND | MF_CHECKED);
		break;
	}
	if(options.nBmpZoom > 1)
		pMenu->CheckMenuItem(ID_BMPVIEW_ZOOM, MF_BYCOMMAND | MF_CHECKED);
	switch(options.nBmpColorWidth)
	{
	case 8:
		switch(options.nBmp8bitPattern)
		{
		case 0:
			pMenu->CheckMenuItem(ID_BMPVIEW_8BITCOLOR, MF_BYCOMMAND | MF_CHECKED);
			break;
		case 1:
			pMenu->CheckMenuItem(ID_BMPVIEW_8BITCOLOR_PAT2, MF_BYCOMMAND | MF_CHECKED);
			break;
		case 2:
			pMenu->CheckMenuItem(ID_BMPVIEW_8BITCOLOR_PAT3, MF_BYCOMMAND | MF_CHECKED);
			break;
		}
		break;
	case 24:
		pMenu->CheckMenuItem(ID_BMPVIEW_24BITCOLOR, MF_BYCOMMAND | MF_CHECKED);
		break;
	case 32:
		pMenu->CheckMenuItem(ID_BMPVIEW_32BITCOLOR, MF_BYCOMMAND | MF_CHECKED);
		break;
	}
	if(options.bAddressTooltip)
		pMenu->CheckMenuItem(ID_BMPVIEW_ADDRESSTOOLTIP, MF_BYCOMMAND | MF_CHECKED);

	CPoint pt;
	GetCursorPos(&pt);
	pMenu->TrackPopupMenu(0, pt.x, pt.y, this);

	CScrollView::OnRButtonDown(nFlags, point);
}

void CBZBmpView::OnBmpViewMode(UINT nID)
{
	switch(nID) {
	case ID_BMPVIEW_WIDTH128:
		options.nBmpWidth = 128;
		break;
	case ID_BMPVIEW_WIDTH256:
		options.nBmpWidth = 256;
		break;
	case ID_BMPVIEW_ZOOM:
		options.nBmpZoom = (options.nBmpZoom == 1) ? 2 : 1;

	}
	GetMainFrame()->CreateClient();
}

BOOL CBZBmpView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: �����Ƀ��b�Z�[�W �n���h�� �R�[�h��ǉ����邩�A����̏������Ăяo���܂��B
//	::SetCursor(AfxGetApp()->LoadCursor(IDC_BMPPOINTER));
//	return true;

	return CScrollView::OnSetCursor(pWnd, nHitTest, message);
}

void CBZBmpView::OnBmpViewColorWidth(UINT nID)
{
	switch(nID)
	{
	case ID_BMPVIEW_8BITCOLOR:
		options.nBmpColorWidth = 8;
		options.nBmp8bitPattern = 0;
		break;
	case ID_BMPVIEW_8BITCOLOR_PAT2:
		options.nBmpColorWidth = 8;
		options.nBmp8bitPattern = 1;
		break;
	case ID_BMPVIEW_8BITCOLOR_PAT3:
		options.nBmpColorWidth = 8;
		options.nBmp8bitPattern = 2;
		break;
	case ID_BMPVIEW_24BITCOLOR:
		options.nBmpColorWidth = 24;
		break;
	case ID_BMPVIEW_32BITCOLOR:
		options.nBmpColorWidth = 32;
		break;
	}
	GetMainFrame()->CreateClient();
}

void CBZBmpView::OnBmpViewAddressTooltip()
{
	options.bAddressTooltip = !options.bAddressTooltip;
}

DWORD CBZBmpView::getCaretPos()
{
	CBZView* pView = (CBZView*)GetNextWindow();
	return pView->m_dwCaret;
}

void CBZBmpView::scrollToCenterOfBmpPoint(CPoint &point)
{
	// ScrollView�Ō����Ă���傫��
	CRect cr;
	GetClientRect(cr);
	CSize client = cr.Size();
	// ScrollView�Ō����Ă��镔���̂����ŁA���S�̍��W
	CPoint center = client;
	center.x /= 2;
	center.y /= 2;
	// ScrollView�S�̂̑傫��
	CSize sz = GetTotalSize();
	// scroll�I�t�Z�b�g
	CPoint scroll;

	// TODO: ���Ƃ�min/max�g�������ɂ��邩���B
	if(point.x - center.x < 0)
	{
		scroll.x = 0;
	} else if(sz.cx < point.x + center.x)
	{
		scroll.x = sz.cx - client.cx;
	} else
	{
		scroll.x = point.x - center.x;
	}

	if(point.y - center.y < 0)
	{
		scroll.y = 0;
	} else if(sz.cy < point.y + center.y)
	{
		scroll.y = sz.cy - client.cy;
	} else
	{
		scroll.y = point.y - center.y;
	}

	ScrollToPosition(scroll);
}

void CBZBmpView::OnBmpViewGotoCaret()
{
	DWORD currentAddress = getCaretPos();
	CPoint point;
	getBmpPointFromAddr(currentAddress, point);
	scrollToCenterOfBmpPoint(point);
	// point���|�b�v�A�b�v���������W�ɍX�V
	point -= GetScrollPosition();

	// TODO: CString�Ƃ�����Ȃ���TCHAR[]?
	CString tmp;
	tmp.Format(_T("0x%08X"), currentAddress);
	WTL::CToolInfo toolinfo(TTF_SUBCLASS|TTF_TRANSPARENT|TTF_TRACK, m_hWnd, 0, 0, const_cast<LPTSTR>(tmp.GetString()));
	m_tooltip.SetToolInfo(toolinfo);
	m_tooltip.Activate(TRUE);
	m_tooltip.Popup();
	m_tooltip.TrackActivate(toolinfo, TRUE);
	ClientToScreen(&point);
	m_tooltip.TrackPosition(point.x, point.y);

	m_isShowingCaretPos = TRUE;
}

void CBZBmpView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_DOWN:
		this->SendMessage(WM_VSCROLL, SB_LINEDOWN, 0);
		break;
	case VK_UP:
		this->SendMessage(WM_VSCROLL, SB_LINEUP, 0);
		break;
	case VK_NEXT://PageDown
		this->SendMessage(WM_VSCROLL, SB_PAGEDOWN, 0);
		break;
	case VK_PRIOR://PageUp
		this->SendMessage(WM_VSCROLL, SB_PAGEUP, 0);
		break;
	case VK_HOME:
		this->SendMessage(WM_VSCROLL, SB_TOP, 0);
		break;
	case VK_END:
		this->SendMessage(WM_VSCROLL, SB_BOTTOM, 0);
		break;
	}

	CScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CBZBmpView::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);

	// TODO: calculate the total size of this view
	CSize cView = m_cBmp;
	cView.cx = cView.cx * options.nBmpZoom + BMPSPACE*2;
	cView.cy = cView.cy * options.nBmpZoom + BMPSPACE*2;

	if(cx == 0 || cy == 0) { // at initialize
		return;
	}

	CSize sizePage;
	sizePage.cx = 0;
	sizePage.cy = cy - BMPSPACE*2;
	CSize sizeLine;
	sizeLine.cx = 0;
	sizeLine.cy = sizePage.cy / 10;
	SetScrollSizes(MM_TEXT, cView, sizePage, sizeLine);

	TRACE("cView.cy=%X\n", GetTotalSize().cy);
}

void CBZBmpView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	CScrollView::OnActivateView(bActivate, pActivateView, pDeactiveView);

	if(bActivate)
	{
		DWORD currentAddress = getCaretPos();
		// ScrollView�S�̂ɂ�����\�����������W
		CPoint point;
		getBmpPointFromAddr(currentAddress, point);

		// point��caret��\������ɂ������Đ��������W�ɕϊ�
		point -= GetScrollPosition();

		// caret�\��
		CreateSolidCaret(5,5);
		SetCaretPos(CPoint(point.x-2, point.y-2));
		ShowCaret();
	} else
	{
		HideCaret();
		m_isShowingCaretPos = FALSE;
		if(pActivateView != NULL) // ==NULL when closing frame, it cause assertion error in tooltip...
		{
			m_tooltip.Activate(false);
			m_tooltip.Pop();
		}
	}
}
