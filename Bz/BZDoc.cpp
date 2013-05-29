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
#include "BZDoc.h"
#include "Mainfrm.h"
#include "zlib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef FILE_MAPPING
  //#define FILEOFFSET_MASK 0xFFF00000	// by 1MB
  #define MAX_FILELENGTH  0xFFFFFFF0
#endif //FILE_MAPPING

/////////////////////////////////////////////////////////////////////////////
// CBZDoc

IMPLEMENT_DYNCREATE(CBZDoc, CDocument)

BEGIN_MESSAGE_MAP(CBZDoc, CDocument)
	//{{AFX_MSG_MAP(CBZDoc)
	ON_COMMAND(ID_EDIT_READONLY, OnEditReadOnly)
	ON_UPDATE_COMMAND_UI(ID_EDIT_READONLY, OnUpdateEditReadOnly)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_READONLYOPEN, OnEditReadOnlyOpen)
	ON_UPDATE_COMMAND_UI(ID_EDIT_READONLYOPEN, OnUpdateEditReadOnlyOpen)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_INDICATOR_INS, OnEditReadOnly)
	ON_UPDATE_COMMAND_UI_RANGE(ID_FILE_SAVE, ID_FILE_SAVE_AS, OnUpdateFileSave)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBZDoc construction/destruction

CBZDoc::CBZDoc()
{
	// TODO: add one-time construction code here
	m_pData = NULL;
	m_dwTotal = 0;
	m_bReadOnly = TRUE;
#ifdef FILE_MAPPING
	m_hMapping = NULL;
	m_pFileMapping = NULL;
	m_pDupDoc = NULL;
	m_pMapStart = NULL;
	m_dwFileOffset = 0;
	m_dwMapSize = 0;
#endif //FILE_MAPPING
	m_dwBase = 0;

	//Undo
	m_pUndo = NULL;
	m_dwUndo = 0;
	m_dwUndoSaved = 0;

	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	m_dwAllocationGranularity = sysinfo.dwAllocationGranularity;
}

CBZDoc::~CBZDoc()
{
//	TRACE("DestructDoc:%X\n",this);
}

LPBYTE	CBZDoc::GetDocPtr()
{
	return m_pData;
}

void CBZDoc::DeleteContents() 
{
	// TODO: Add your specialized code here and/or call the base class

	if(m_pData) {
#ifdef FILE_MAPPING
		if(IsFileMapping()) {
			VERIFY(::UnmapViewOfFile(m_pMapStart ? m_pMapStart : m_pData));
			m_pMapStart = NULL;
			m_dwFileOffset = 0;
			m_dwMapSize = 0;
		}
		else
#endif //FILE_MAPPING
			MemFree(m_pData);
		m_pData = NULL;
		m_dwTotal = 0;
		m_dwBase = 0;
		UpdateAllViews(NULL);
	}
#ifdef FILE_MAPPING
	if(IsFileMapping()) {
		if(m_pDupDoc) {
			m_pDupDoc->m_pDupDoc = NULL;
			m_pDupDoc = NULL;
			m_hMapping = NULL;
			m_pFileMapping = NULL;
		} else {
			VERIFY(::CloseHandle(m_hMapping));
			m_hMapping = NULL;
			if(m_pFileMapping) {
				ReleaseFile(m_pFileMapping, FALSE);
				m_pFileMapping = NULL;
			}
		}
	}
#endif //FILE_MAPPING

	if(m_pUndo) {
		MemFree(m_pUndo);
		m_pUndo = NULL;
	}	
	m_bReadOnly = FALSE;
	m_arrMarks.RemoveAll();
	CDocument::DeleteContents();
}

/////////////////////////////////////////////////////////////////////////////
// CBZDoc serialization

void CBZDoc::Serialize(CArchive& ar)
{
	MEMORYSTATUS ms;
	GlobalMemoryStatus(&ms);

	CFile *pFile = ar.GetFile();
	ar.Flush();

	if (ar.IsLoading())	{
		// TODO: add loading code here
		m_dwTotal = GetFileLength(pFile);
#ifdef FILE_MAPPING
		if(IsFileMapping()) {
			if(!MapView()) return;
		} else
#endif //FILE_MAPPING
		{
			if(!(m_pData = (LPBYTE)MemAlloc(m_dwTotal))) {
				AfxMessageBox(IDS_ERR_MALLOC);
				return;
			}
			DWORD len = pFile->Read(m_pData, m_dwTotal);
			if(len < m_dwTotal) {
				AfxMessageBox(IDS_ERR_READFILE);
				MemFree(m_pData);	// ###1.61
				m_pData = NULL;
				return;
			}
			m_bReadOnly = options.bReadOnlyOpen;
		}
	} else {
		// TODO: add storing code here
#ifdef FILE_MAPPING
		if(IsFileMapping()) {
			BOOL bResult = (m_pMapStart) ? ::FlushViewOfFile(m_pMapStart, m_dwMapSize) : ::FlushViewOfFile(m_pData, m_dwTotal);
			if(!bResult) {
				ErrorMessageBox();
			}
		} else
#endif //FILE_MAPPING
			pFile->Write(m_pData, m_dwTotal);
		m_dwUndoSaved = m_dwUndo;		// ###1.54
		TouchDoc();
/*		if(m_pUndo) {
			MemFree(m_pUndo);
			m_pUndo = NULL;
		}	
*/	}
}

void CBZDoc::SavePartial(CFile& file, DWORD offset, DWORD size)
{
#ifdef FILE_MAPPING
	if(IsFileMapping()) {
		// QueryMapView�͎g�����ςȂ��Ŗ��Ȃ��B(draw���Ȃǂ�QueryMapView���Ȃ����Ă����B)
		while(size > 0) {
			LPBYTE pData = QueryMapViewTama2(offset, size); // �������݂����f�[�^��������悤�ɂ���B
			DWORD writesize = min(GetMapRemain(offset), size); // �������߂邾���̃f�[�^�ʂ����ς���B
			file.Write(pData, writesize); // �������ށB

			// �������߂����Aoffset�i�߂�size���炷�B
			offset += writesize;
			size -= writesize;
		}
	} else
#endif
		file.Write(m_pData + offset, size);
}

static int inflateBlock(CFile& file, LPBYTE buf, const DWORD bufsize, z_stream& z, LPBYTE pData, DWORD dataSize) {
	int ret;

	z.next_in = pData;
	z.avail_in = dataSize;

	do {
		z.next_out = buf;
		z.avail_out = bufsize;
		ret = inflate(&z, Z_NO_FLUSH);

		// �W�J��̃f�[�^���������ށB
		// �G���[�ł��ł������͏����B
		file.Write(buf, bufsize - z.avail_out);

		if(ret != Z_OK && ret != Z_STREAM_END) {
			// Error! �����^�[���B
			return ret;
		}
	} while(z.avail_out == 0);

	return ret;
}

void CBZDoc::SavePartialInflated(CFile& file, DWORD offset, DWORD size, CBZView& view)
{
	z_stream z = {0};
	const DWORD bufsize = 32768; // TODO ���̃T�C�Y�ő��v��?
	BYTE *buf = new BYTE[bufsize];
	int ret;

	if(inflateInit(&z) != Z_OK) {
		AfxMessageBox(IDS_ERR_INFLATEINIT, MB_OK | MB_ICONERROR);
		return;
	}

#ifdef FILE_MAPPING
	if(IsFileMapping()) {
		// QueryMapView�͎g�����ςȂ��Ŗ��Ȃ��B(draw���Ȃǂ�QueryMapView���Ȃ����Ă����B)
		while(size > 0) {
			LPBYTE pData = QueryMapViewTama2(offset, size); // �W�J�������f�[�^��������悤�ɂ���B
			DWORD dataSize = min(GetMapRemain(offset), size); // �W�J�ł��邾���̃f�[�^�ʂ����ς���B
			ret = inflateBlock(file, buf, bufsize, z, pData, dataSize);

			if(ret != Z_OK) { // Z_STREAM_END �������̓G���[���̓��[�v�𔲂���B
				// offset/size��z.avail_in����Čv�Z����B�ǂꂭ�炢�]���ł����������킩��B
				offset += dataSize - z.avail_in; // �Ӗ��Ȃ����ǈꉞoffset���������Ă����c
				size -= dataSize - z.avail_in;
				break;
			}

			// �W�J�ł������Aoffset�i�߂�size���炷�B
			offset += dataSize;
			size -= dataSize;
		}
	} else
#endif
	{
		ret = inflateBlock(file, buf, bufsize, z, m_pData + offset, size);
		// offset/size��z.avail_in����Čv�Z����B�ǂꂭ�炢�]���ł����������킩��B
		offset += size - z.avail_in; // �Ӗ��Ȃ����ǈꉞoffset���������Ă����c
		size = z.avail_in;
	}

	if(ret == Z_STREAM_END) {
		if(size > 0) {
			// �]�����o�C�g����\���B
			CString sMsg;
			sMsg.Format(IDS_INFLATE_REMAIN_BYTES, size);
			if(AfxMessageBox(sMsg, MB_YESNO | MB_ICONQUESTION) == IDYES) {
				// ���ɑI�����Ă���͈͂��������Ƃ�������B(���ې������Ȃ��Ƃ���̓o�O�B)
				view.setBlock(view.BlockBegin(), view.BlockEnd() - size);
			}
		} else {
			/* do nothing */
		}
	} else if(ret == Z_OK) {
		AfxMessageBox(IDS_ERR_INFLATE_RESULT_OK, MB_OK | MB_ICONERROR);
	} else {
		AfxMessageBox(IDS_ERR_INFLATE_RESULT_ERROR, MB_OK | MB_ICONERROR);
	}

	inflateEnd(&z); // �O�̂��߁Az.avail_in���Q�ƂȂǂ������inflateEnd����B
	delete buf;
}

#ifdef FILE_MAPPING

BOOL CBZDoc::MapView()
{
	m_dwMapSize = m_dwTotal;
	m_pData = (LPBYTE)::MapViewOfFile(m_hMapping, m_bReadOnly ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, m_dwMapSize);
	if(!m_pData) {
		m_dwMapSize = options.dwMaxMapSize;
		m_pData = (LPBYTE)::MapViewOfFile(m_hMapping, m_bReadOnly ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, m_dwMapSize);
		if(m_pData) {
			m_pMapStart = m_pData;
			m_dwFileOffset = 0;
		}
		else {
			ErrorMessageBox();
			AfxThrowMemoryException();	// ###1.61
			return FALSE;
		}
	}
	return TRUE;
}

LPBYTE CBZDoc::QueryMapView1(LPBYTE pBegin, DWORD dwOffset)
{
	TRACE("QueryMapView1 m_pData:%X, pBegin:%X, dwOffset:%X\n", m_pData, pBegin, dwOffset);
	LPBYTE p = pBegin + dwOffset;
	//if(IsOutOfMap1(p)) {QueryMapView()���Ɉړ�
		if(p == m_pData + m_dwTotal && p == m_pMapStart + m_dwMapSize) return pBegin;	// ###1.61a
		DWORD dwBegin = GetFileOffsetFromFileMappingPointer(pBegin);//DWORD dwBegin = pBegin - m_pData;
		VERIFY(::UnmapViewOfFile(m_pMapStart));//�����ŏ������܂ꂿ�Ⴄ����OK?
		//m_dwFileOffset = GetFileOffsetFromFileMappingPointer(p)/*(p - m_pDate)*/ & FILEOFFSET_MASK;
		DWORD dwTmp1 = GetFileOffsetFromFileMappingPointer(p);
		m_dwFileOffset = dwTmp1 - (dwTmp1 % m_dwAllocationGranularity);
		m_dwMapSize = min(options.dwMaxMapSize, m_dwTotal - m_dwFileOffset); //�ǂ�������������������B�S�̈���}�b�s���O�ł���HCBZDoc::MapView()�̃u���b�N�Q���R�����g�A�E�g�����ق������������B����32�r�b�g��4GB�I�[�o�[�Ή��̍ۂɖ��ɂȂ肻�� by tamachan(20120907)
		if(m_dwMapSize == 0) {	// ###1.61a
			//m_dwFileOffset = (m_dwTotal - (~FILEOFFSET_MASK + 1)) & FILEOFFSET_MASK;
			dwTmp1 = (m_dwTotal - m_dwAllocationGranularity);
			m_dwFileOffset = dwTmp1 - (dwTmp1 % m_dwAllocationGranularity);
			m_dwMapSize = m_dwTotal - m_dwFileOffset;
		}
		int retry = 3;
		m_pMapStart = NULL;
		do
		{
			m_pMapStart = (LPBYTE)::MapViewOfFile(m_hMapping, m_bReadOnly ? FILE_MAP_READ : FILE_MAP_WRITE, 0, m_dwFileOffset, m_dwMapSize);
		} while(m_pMapStart==NULL && --retry > 0);
		TRACE("MapViewOfFile Doc=%X, %X, Offset:%X, Size:%X\n", this, m_pMapStart, m_dwFileOffset, m_dwMapSize);
		if(!m_pMapStart) {
			ErrorMessageBox();
			AfxPostQuitMessage(0);
			return NULL;
		}
		m_pData = m_pMapStart - m_dwFileOffset; //�o�O?�F���z�I�ȃA�h���X�i�t�@�C���̃I�t�Z�b�g0�ɂ����郁�����A�h���X�j�����o���Ă���Bm_pMapStart<m_dwFileOffset�������ꍇ�A0�����邱�Ƃ�����̂ł͂Ȃ����낤���H�����������ꍇ�܂����H�HIsOutOfMap�͐���ɓ��������H by tamachan(20121004)
//		ASSERT(m_pMapStart > m_pData);
		pBegin = GetFileMappingPointerFromFileOffset(dwBegin);//pBegin = m_pData + dwBegin;
	//}
	return pBegin;
}

void CBZDoc::AlignMapSize(DWORD dwStartOffset, DWORD dwIdealMapSize)
{
	m_dwFileOffset = dwStartOffset - (dwStartOffset % m_dwAllocationGranularity);
	m_dwMapSize = min(max(options.dwMaxMapSize, dwIdealMapSize),  m_dwTotal - m_dwFileOffset);
	if(m_dwMapSize == 0)
	{
		DWORD dwTmp1 = (m_dwTotal - m_dwAllocationGranularity);
		m_dwFileOffset = dwTmp1 - (dwTmp1 % m_dwAllocationGranularity);
		m_dwMapSize = m_dwTotal - m_dwFileOffset;
	}//�o�O�F�t�@�C���T�C�Y���ɒ[�ɏ������ꍇ�o�O��
}
LPBYTE CBZDoc::_QueryMapViewTama2(DWORD dwStartOffset, DWORD dwIdealMapSize)
{
	if(dwStartOffset == m_dwTotal && dwStartOffset == m_dwFileOffset + m_dwMapSize) return GetFileMappingPointerFromFileOffset(dwStartOffset);//�o�O�H�������������ɗ���̂͂��������̂ł͂Ȃ����H
	VERIFY(::UnmapViewOfFile(m_pMapStart));//�����Ń}�b�s���O��Ԃւ̕ύX�����t�@�C���֏������܂��B��ɕۑ���������ꍇ�̓A���h�D�Ŗ߂��B

	AlignMapSize(dwStartOffset, dwIdealMapSize);

	m_pMapStart = (LPBYTE)::MapViewOfFile(m_hMapping, m_bReadOnly ? FILE_MAP_READ : FILE_MAP_WRITE, 0, m_dwFileOffset, m_dwMapSize);
	TRACE("MapViewOfFile Doc=%X, %X, Offset:%X, Size:%X\n", this, m_pMapStart, m_dwFileOffset, m_dwMapSize);
	if(!m_pMapStart) {
		ErrorMessageBox();
		AfxPostQuitMessage(0);
		return NULL;
	}
	m_pData = m_pMapStart - m_dwFileOffset;
	return GetFileMappingPointerFromFileOffset(dwStartOffset);
}

BOOL CBZDoc::IsOutOfMap1(LPBYTE p)
{
	return ((int)p < (int)m_pMapStart || (int)p >= (int)(m_pMapStart + m_dwMapSize));
}

#endif //FILE_MAPPING

/////////////////////////////////////////////////////////////////////////////
// CBZDoc diagnostics

#ifdef _DEBUG
void CBZDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CBZDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBZDoc commands


void CBZDoc::OnEditReadOnly() 
{
#ifdef FILE_MAPPING
	if(IsFileMapping()) {
		AfxMessageBox(IDS_ERR_MAP_RO);
		return;
	}
#endif //FILE_MAPPING
	m_bReadOnly = !m_bReadOnly;	
}

void CBZDoc::OnUpdateEditReadOnly(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bReadOnly);
}

void CBZDoc::OnEditReadOnlyOpen() 
{
	// TODO: Add your command handler code here
	options.bReadOnlyOpen = !options.bReadOnlyOpen;
}

void CBZDoc::OnUpdateEditReadOnlyOpen(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(options.bReadOnlyOpen);	
}

BOOL CBZDoc::CopyToClipboard(DWORD dwPtr, DWORD dwSize, BOOL bHexString)	// ###1.5
{
#ifdef FILE_MAPPING
	QueryMapView(m_pData, dwPtr);
	if(dwSize >= options.dwMaxMapSize || IsOutOfMap(m_pData + dwPtr + dwSize)) {
		AfxMessageBox(IDS_ERR_COPY);
		return FALSE;
	}
#endif //FILE_MAPPING
	HGLOBAL hMemTxt, hMemBin;
	LPBYTE pMemTxt, pMemBin;
	if(!bHexString) {
		hMemTxt = ::GlobalAlloc(GMEM_MOVEABLE, dwSize + 1);
	} else {
		// hexstring
		hMemTxt = ::GlobalAlloc(GMEM_MOVEABLE, dwSize * 2 + 1);
	}
	pMemTxt  = (LPBYTE)::GlobalLock(hMemTxt);
	if(!bHexString) {
		hMemBin = ::GlobalAlloc(GMEM_MOVEABLE, dwSize + sizeof(dwSize));
		pMemBin  = (LPBYTE)::GlobalLock(hMemBin);
	}
	if(!bHexString) {
		memcpy(pMemTxt, m_pData + dwPtr, dwSize);
		*(pMemTxt + dwSize) = '\0';
	} else {
		// hexstring
		LPBYTE s = m_pData + dwPtr;
		LPBYTE e = s + dwSize;
		LPBYTE d = pMemTxt;
		for(; s < e; s++, d += 2) {
			wsprintfA((LPSTR)d, "%02X", *s);
		}
		*d = '\0';
	}
	::GlobalUnlock(hMemTxt);
	if(!bHexString) {
		*((DWORD*)(pMemBin)) = dwSize;
		memcpy(pMemBin + sizeof(dwSize), m_pData + dwPtr, dwSize);
		::GlobalUnlock(hMemBin);
	}
	AfxGetMainWnd()->OpenClipboard();
	::EmptyClipboard();
	::SetClipboardData(CF_TEXT, hMemTxt);
	if(!bHexString) {
		::SetClipboardData(RegisterClipboardFormat(_T("BinaryData2")), hMemBin);
	}
	::CloseClipboard();
	return TRUE;
}

DWORD CBZDoc::PasteFromClipboard(DWORD dwPtr, BOOL bIns)
{
	AfxGetMainWnd()->OpenClipboard();
	HGLOBAL hMem;
	DWORD dwSize;
	LPBYTE pMem;
	LPBYTE pWorkMem = NULL;
	if(hMem = ::GetClipboardData(RegisterClipboardFormat(_T("BinaryData2")))) {
		pMem = (LPBYTE)::GlobalLock(hMem);
		dwSize = *((DWORD*)(pMem));
		pMem += sizeof(DWORD);
	} else if((options.charset == CTYPE_UTF8 || options.charset == CTYPE_UNICODE) && (hMem = GetClipboardData(CF_UNICODETEXT))) {
		int nchars;

		pMem = (LPBYTE)::GlobalLock(hMem);
		nchars = lstrlenW((LPWSTR)pMem);
		dwSize = nchars * 2;

		if(options.charset == CTYPE_UTF8) {
			// UTF-8�Ȃ�AUTF-16����UTF-8�ɕϊ�����B
			dwSize = ConvertUTF16toUTF8(pWorkMem, (LPCWSTR)pMem); // pWorkMem��MemAlloc���ԋp����|�C���^�ŏ㏑�������B

			pMem = pWorkMem;
		} else {
			// UTF-16�Ȃ�A�o�C�g�I�[�_�[�𒲐�����B
			pWorkMem = (LPBYTE)MemAlloc(dwSize);
			for(int i = 0; i < nchars; i++) {
				((LPWORD)pWorkMem)[i] = SwapWord(((LPWORD)pMem)[i]);
			}

			pMem = pWorkMem;
		}
	} else if(hMem = GetClipboardData(CF_TEXT)) {
		pMem = (LPBYTE)::GlobalLock(hMem);
		dwSize = lstrlenA((LPCSTR)pMem);
	} else {
/*		UINT uFmt = 0;
		while(uFmt = ::EnumClipboardFormats(uFmt)) {
			CString sName;
			::GetClipboardFormatName(uFmt, sName.GetBuffer(MAX_PATH), MAX_PATH);
			sName.ReleaseBuffer();
			TRACE("clip 0x%X:%s\n", uFmt, sName);
		}

		return 0;
*/		if(!(hMem = ::GetClipboardData(::EnumClipboardFormats(0))))
			return 0;
		pMem = (LPBYTE)::GlobalLock(hMem);
		dwSize = ::GlobalSize(hMem);
	}
	if(!dwSize) return 0;
#ifdef FILE_MAPPING
	if(IsFileMapping()) {
		// FileMapping���A�\��t����ƃt�@�C���T�C�Y���傫���Ȃ��Ă��܂��悤�ł���΁A���O�ɒ���t���T�C�Y��؂�l�߂�B
		//int nGlow = dwSize - (m_dwTotal - dwPtr);
		DWORD nGlow = dwSize - (m_dwTotal - dwPtr);
		if(nGlow <= dwSize/*overflow check*/ && nGlow > 0)
			dwSize -= nGlow;
	}
#endif //FILE_MAPPING
	if(bIns || dwPtr == m_dwTotal)
		StoreUndo(dwPtr, dwSize, UNDO_DEL);
	else
		StoreUndo(dwPtr, dwSize, UNDO_OVR);
	InsertData(dwPtr, dwSize, bIns);
	memcpy(m_pData+dwPtr, pMem, dwSize);
	::GlobalUnlock(hMem);
	::CloseClipboard();
	if(pWorkMem) {
		MemFree(pWorkMem);
	}
	return dwPtr+dwSize;
}

void CBZDoc::InsertData(DWORD dwPtr, DWORD dwSize, BOOL bIns)
{
	BOOL bGlow = false;
	DWORD nGlow = dwSize - (m_dwTotal - dwPtr);
	if(nGlow <= dwSize/*overflow check*/ && nGlow > 0)bGlow=true;
//	int nGlow = dwSize - (m_dwTotal - dwPtr);
	if(!m_pData) {
		m_pData = (LPBYTE)MemAlloc(dwSize);
		m_dwTotal = dwSize;
	} else if(bIns || dwPtr == m_dwTotal) {
			m_pData = (LPBYTE)MemReAlloc(m_pData, m_dwTotal+dwSize);
			memmove(m_pData+dwPtr+dwSize, m_pData+dwPtr, m_dwTotal - dwPtr);
			m_dwTotal += dwSize;
	} else if(bGlow) {
			m_pData = (LPBYTE)MemReAlloc(m_pData, m_dwTotal+nGlow);
			m_dwTotal += nGlow;
	}
	ASSERT(m_pData != NULL);
}

void CBZDoc::DeleteData(DWORD dwPtr, DWORD dwSize)
{
	if(dwPtr == m_dwTotal) return;
	memmove(m_pData+dwPtr, m_pData+dwPtr+dwSize, m_dwTotal-dwPtr-dwSize);
	m_dwTotal -= dwSize;
#ifdef FILE_MAPPING
	if(!IsFileMapping())
#endif //FILE_MAPPING
		m_pData = (LPBYTE)MemReAlloc(m_pData, m_dwTotal);
	TouchDoc();
}

BOOL CBZDoc::isDocumentEditedSelfOnly()
{
	return m_dwUndo != m_dwUndoSaved;
}

void CBZDoc::TouchDoc()
{
	SetModifiedFlag(isDocumentEditedSelfOnly());
	GetMainFrame()->OnUpdateFrameTitle();
}

void CBZDoc::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!!m_pUndo);	
}


//OVR
//dwPtr-4byte(file-offset), mode(byte), data(? byte), dwBlock-4byte(������܂߂��S���̃o�C�g)
//dwSize��dwBlock-9�B�܂�data�̃T�C�Y

void CBZDoc::StoreUndo(DWORD dwPtr, DWORD dwSize, UndoMode mode)
{
	if(mode == UNDO_OVR && dwPtr+dwSize >= m_dwTotal)
		dwSize = m_dwTotal - dwPtr;
	if(dwSize == 0) return;
#ifdef FILE_MAPPING
	QueryMapView(m_pData, dwPtr);
#endif //FILE_MAPPING
	DWORD dwBlock = dwSize + 9;
	if(mode == UNDO_DEL)
		dwBlock = 4 + 9;
	if(!m_pUndo) {
		m_pUndo = (LPBYTE)MemAlloc(dwBlock);
		m_dwUndo = m_dwUndoSaved = 0;
	} else
		m_pUndo = (LPBYTE)MemReAlloc(m_pUndo, m_dwUndo+dwBlock);
	ASSERT(m_pUndo != NULL);
	LPBYTE p = m_pUndo + m_dwUndo;
	*((DWORD*&)p)++ = dwPtr;
	*p++ = mode;
	if(mode == UNDO_DEL) {
		*((DWORD*&)p)++ = dwSize;
	} else {
		memcpy(p, m_pData+dwPtr, dwSize);
		p+=dwSize;
	}
	*((DWORD*&)p)++ = dwBlock;
	m_dwUndo += dwBlock;
	ASSERT(m_dwUndo >= dwBlock && m_dwUndo != 0xFFffFFff);//Overflow check
	ASSERT(p == m_pUndo+m_dwUndo);
	TouchDoc();
}

DWORD CBZDoc::DoUndo()
{
	DWORD dwSize = *((DWORD*)(m_pUndo+m_dwUndo-4));
	m_dwUndo -= dwSize;
	dwSize -= 9;
	LPBYTE p = m_pUndo + m_dwUndo;
	DWORD dwPtr = *((DWORD*&)p)++;
	UndoMode mode = (UndoMode)*p++;
#ifdef FILE_MAPPING
	QueryMapView(m_pData, dwPtr);
#endif //FILE_MAPPING
	if(mode == UNDO_DEL) {
		DeleteData(dwPtr, *((DWORD*)p));
	} else {
		InsertData(dwPtr, dwSize, mode == UNDO_INS);
		memcpy(m_pData+dwPtr, p, dwSize);
	}
	if(m_dwUndo)
		m_pUndo = (LPBYTE)MemReAlloc(m_pUndo, m_dwUndo);
	else {				// ### 1.54
		MemFree(m_pUndo);
		m_pUndo = NULL;
		//if(m_dwUndoSaved)m_dwUndoSaved = UINT_MAX;
	}
	if(m_dwUndo < m_dwUndoSaved)m_dwUndoSaved = 0xFFffFFff;
	// if(!m_pUndo)
		TouchDoc();
	return dwPtr;
}

void CBZDoc::DuplicateDoc(CBZDoc* pDstDoc)
{
#ifdef FILE_MAPPING
	if(IsFileMapping()) {
		m_pDupDoc = pDstDoc;
		pDstDoc->m_pDupDoc = this;
		pDstDoc->m_hMapping = m_hMapping;
		pDstDoc->m_pFileMapping = m_pFileMapping;
		pDstDoc->m_dwTotal = m_dwTotal;
		pDstDoc->MapView();
	} else
#endif //FILE_MAPPING
	{
		pDstDoc->m_pData = (LPBYTE)MemAlloc(m_dwTotal);
		memcpy(pDstDoc->m_pData, m_pData, m_dwTotal);
		pDstDoc->m_dwTotal = m_dwTotal;
	}
	pDstDoc->m_dwBase = m_dwBase;
	pDstDoc->SetTitle(GetTitle());
	CString s = GetPathName();
	if(!s.IsEmpty())
		pDstDoc->SetPathName(s);
//	pDstDoc->UpdateAllViews(NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CBZDoc Mark

void CBZDoc::SetMark(DWORD dwPtr)
{
	for_to(i, m_arrMarks.GetSize()) {
		if(m_arrMarks[i] == dwPtr) {
			m_arrMarks.RemoveAt(i);
			return;
		} else if(m_arrMarks[i] >= m_dwTotal) {
			m_arrMarks.RemoveAt(i);
		}
	}
	m_arrMarks.Add(dwPtr);
}

BOOL CBZDoc::CheckMark(DWORD dwPtr)
{
	for_to(i,  m_arrMarks.GetSize()) {
		if(m_arrMarks[i] == dwPtr)
			return TRUE;
	}
	return FALSE;
}

DWORD CBZDoc::JumpToMark(DWORD dwStart)
{
	DWORD dwNext = m_dwTotal;
Retry:
	for_to(i, m_arrMarks.GetSize()) {
		if(m_arrMarks[i] > dwStart && m_arrMarks[i] < dwNext)
			dwNext = m_arrMarks[i];
	}
	if(dwNext == m_dwTotal && dwStart) {
		dwStart = 0;
		goto Retry;
	}
	if(dwNext < m_dwTotal) 
		return dwNext;
	return INVALID;
}

void CBZDoc::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_bReadOnly);
}

/////////////////////////////////////////////////////////////////////////////
// ###1.60 File Mapping

DWORD CBZDoc::GetFileLength(CFile* pFile, BOOL bErrorMsg)
{
	DWORD dwSizeHigh = 0;
	DWORD dwSize = ::GetFileSize((HANDLE)pFile->m_hFile, &dwSizeHigh);
	if(dwSizeHigh) {
		if(bErrorMsg)
			AfxMessageBox(IDS_ERR_OVER4G);
		dwSize = MAX_FILELENGTH;
	}
	return dwSize;
}

#ifdef FILE_MAPPING

#ifndef _AFX_OLD_EXCEPTIONS
    #define DELETE_EXCEPTION(e) do { e->Delete(); } while (0)
#else   //!_AFX_OLD_EXCEPTIONS
    #define DELETE_EXCEPTION(e)
#endif  //_AFX_OLD_EXCEPTIONS


void CBZDoc::ReleaseFile(CFile* pFile, BOOL bAbort)
{
	// ------ File Mapping ----->
	if(IsFileMapping()) return;
	// <----- File Mapping ------
	ASSERT_KINDOF(CFile, pFile);
	if (bAbort)
		pFile->Abort(); // will not throw an exception
	else {
		pFile->Close();
	}
	delete pFile;
}

BOOL CBZDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (IsModified())
		TRACE0("Warning: OnOpenDocument replaces an unsaved document.\n");

	CFileException fe;
	CFile* pFile = GetFile(lpszPathName,
		CFile::modeRead|CFile::shareDenyWrite, &fe);
	if (pFile == NULL)
	{
		ReportSaveLoadException(lpszPathName, &fe,
			FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		return FALSE;
	}

	DeleteContents();
	SetModifiedFlag();  // dirty during de-serialize

	// ------ File Mapping ----->
	DWORD dwSize = GetFileLength(pFile, TRUE);
	if(dwSize >= options.dwMaxOnMemory) {
		m_bReadOnly = options.bReadOnlyOpen || ::GetFileAttributes(lpszPathName) & FILE_ATTRIBUTE_READONLY;
		if(!m_bReadOnly) {  // Reopen for ReadWrite
			ReleaseFile(pFile, FALSE);
			pFile = GetFile(lpszPathName, CFile::modeReadWrite | CFile::shareExclusive, &fe);
			if(pFile == NULL) {
				ReportSaveLoadException(lpszPathName, &fe, FALSE, AFX_IDP_INVALID_FILENAME);
				return FALSE;
			}
		}
		m_pFileMapping = pFile;
		m_hMapping = ::CreateFileMapping((HANDLE)pFile->m_hFile, NULL, m_bReadOnly ? PAGE_READONLY : PAGE_READWRITE
			, 0, 0, /*options.bReadOnlyOpen ? 0 : dwSize + options.dwMaxOnMemory,*/ NULL);
		if(!m_hMapping) {
			ErrorMessageBox();
			ReleaseFile(pFile, FALSE);
			return FALSE;
		}
	}
	// <----- File Mapping ------

	CArchive loadArchive(pFile, CArchive::load | CArchive::bNoFlushOnDelete);
	loadArchive.m_pDocument = this;
	loadArchive.m_bForceFlat = FALSE;
	TRY
	{
		CWaitCursor wait;
		if (GetFileLength(pFile) != 0)
			Serialize(loadArchive);     // load me
		loadArchive.Close();
		ReleaseFile(pFile, FALSE);
	}
	CATCH_ALL(e)
	{
		ReleaseFile(pFile, TRUE);
		DeleteContents();   // remove failed contents

		TRY
		{
			ReportSaveLoadException(lpszPathName, e,
				FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		}
		END_TRY
		DELETE_EXCEPTION(e);
		return FALSE;
	}
	END_CATCH_ALL

	SetModifiedFlag(FALSE);     // start off with unmodified

	return TRUE;
}

BOOL CBZDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	CFileException fe;
	CFile* pFile = NULL;

	// ------ File Mapping ----->
	if(IsFileMapping())
		pFile = m_pFileMapping;
	else 
	// <----- File Mapping ------
		pFile = GetFile(lpszPathName, CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive, &fe);

	if (pFile == NULL)
	{
		ReportSaveLoadException(lpszPathName, &fe,
			TRUE, AFX_IDP_INVALID_FILENAME);
		return FALSE;
	}

	CArchive saveArchive(pFile, CArchive::store | CArchive::bNoFlushOnDelete);
	saveArchive.m_pDocument = this;
	saveArchive.m_bForceFlat = FALSE;
	TRY
	{
		CWaitCursor wait;
		Serialize(saveArchive);     // save me
		saveArchive.Close();
		ReleaseFile(pFile, FALSE);
	}
	CATCH_ALL(e)
	{
		ReleaseFile(pFile, TRUE);

		TRY
		{
			ReportSaveLoadException(lpszPathName, e,
				TRUE, AFX_IDP_FAILED_TO_SAVE_DOC);
		}
		END_TRY
		DELETE_EXCEPTION(e);
		return FALSE;
	}
	END_CATCH_ALL

	SetModifiedFlag(FALSE);     // back to unmodified

	return TRUE;        // success
}

BOOL CBZDoc::SaveModified() 
{
	if (!IsModified())
		return TRUE;        // ok to continue

	// get name/title of document
	CString name;
	if (m_strPathName.IsEmpty())
	{
		// get name based on caption
		name = m_strTitle;
		if (name.IsEmpty())
			VERIFY(name.LoadString(AFX_IDS_UNTITLED));
	}
	else
	{
		// get name based on file title of path name
		name = m_strPathName;
		/*if (afxData.bMarked4)
		{
			AfxGetFileTitle(m_strPathName, name.GetBuffer(_MAX_PATH), _MAX_PATH);
			name.ReleaseBuffer();
		}*/
	}

	CString prompt;
	AfxFormatString1(prompt, AFX_IDP_ASK_TO_SAVE, name);
	switch (AfxMessageBox(prompt, MB_YESNOCANCEL, AFX_IDP_ASK_TO_SAVE))
	{
	case IDCANCEL:
		return FALSE;       // don't continue

	case IDYES:
		// If so, either Save or Update, as appropriate
		if (!DoFileSave())
			return FALSE;       // don't continue
		break;

	case IDNO:
		// If not saving changes, revert the document
		if(IsFileMapping() && m_dwUndoSaved != 0xFFffFFff) {
			while(isDocumentEditedSelfOnly())//m_pUndo)
				DoUndo();
		}
		break;

	default:
		ASSERT(FALSE);
		break;
	}
	return TRUE;    // keep going
}

#endif //FILE_MAPPING
