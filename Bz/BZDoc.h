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

#define FILE_MAPPING	// ###1.60

enum UndoMode {	UNDO_INS, UNDO_OVR, UNDO_DEL };

class BZ_SCRIPT_DLLEXTERN CBZDoc : public CDocument
{
public: // create from serialization only
	CBZDoc();
	DECLARE_DYNCREATE(CBZDoc)

// Attributes
private:
	LPBYTE	m_pData; //���������̃A�h���X�B�t�@�C�����A�h���X��0�ɓ�����A�h���X�������B�����������I�ȃt�@�C���}�b�s���O�̏ꍇ������B���̏ꍇm_pData����}�b�s���O�̈悪�n�܂��Ă���Ƃ͌���Ȃ��Am_pData�͋[���I�ȃA�h���X�ɂȂ��Ă���
	DWORD	m_dwTotal;
	LPBYTE	m_pUndo;
	DWORD	m_dwUndo;
	DWORD	m_dwUndoSaved;		// ###1.54
	CDWordArray m_arrMarks;

	DWORD	m_dwAllocationGranularity;
public:
	BOOL	m_bReadOnly;

	LPBYTE	GetDocPtr() ; // { return m_pData; }
	DWORD	GetDocSize() { return m_dwTotal; }
#ifdef FILE_MAPPING
	HANDLE	m_hMapping;
	CFile*	m_pFileMapping;
	CBZDoc* m_pDupDoc;
	LPBYTE	m_pMapStart;	// ###1.61�@�}�b�s���O�̈��[�i���������̃A�h���X�j
	DWORD   m_dwFileOffset; //�f�[�^�t�@�C���̃}�b�s���O�J�n�A�h���X�i�t�@�C�����̃A�h���X�j
	DWORD	m_dwMapSize; //�f�[�^�t�@�C���̃}�b�s���O�T�C�Y
#endif //FILE_MAPPING
	DWORD	m_dwBase;		// ###1.63

	DWORD	m_restoreCaret;
	POINT	m_restoreScroll;
	DWORD	m_restoredwBlock;
	BOOL	m_restorebBlock;

// Operations
public:
	BOOL	CopyToClipboard(DWORD dwPtr, DWORD dwSize, BOOL bHexString);
	DWORD	PasteFromClipboard(DWORD dwPtr, BOOL bIns, int format=0);
	DWORD	PasteHexstringFromClipboard(DWORD dwPtr, BOOL bIns);
	BOOL	isDocumentEditedSelfOnly();
	void	InsertData(DWORD dwPtr, DWORD dwSize, BOOL bIns);
	void	DeleteData(DWORD dwPtr, DWORD dwSize);
	BOOL	StoreUndo(DWORD dwPtr, DWORD dwSize, UndoMode mode);
	DWORD	DoUndo();
	void	TouchDoc();		// ###1.54
	void	DuplicateDoc(CBZDoc* pDstDoc);
	void	SetMark(DWORD dwPtr);
	BOOL	CheckMark(DWORD dwPtr);
	DWORD	JumpToMark(DWORD dwPtr);
	BOOL	CanUndo() { return !!m_pUndo; };
	BOOL	DoCopyToClipboard(LPBYTE lpStart, DWORD dwSize, BOOL bHexString);
	DWORD	ClipboardReadOpen(HGLOBAL &hMem, LPBYTE &pMem, LPBYTE &pWorkMem);
	void	ClipboardReadClose(HGLOBAL hMem, LPBYTE pMem, LPBYTE pWorkMem);
#ifdef FILE_MAPPING
	_inline DWORD	GetFileOffsetFromFileMappingPointer(LPBYTE pConv) { return pConv-m_pData; }
	_inline LPBYTE	GetFileMappingPointerFromFileOffset(DWORD dwFileOffset) { return dwFileOffset+m_pData; }

	BOOL	IsFileMapping() { return m_hMapping != NULL; }
	LPBYTE  QueryMapView(LPBYTE pBegin, DWORD dwOffset)
	{
		ATLTRACE("QueryMapView pBegin=0x%08X, dwOffset=0x%08X\n", pBegin, dwOffset);
		//return m_pMapStart ? QueryMapView1(pBegin, dwOffset) : pBegin;
		LPBYTE p = pBegin + dwOffset;
		if(m_pMapStart && IsOutOfMap1(p))return QueryMapView1(pBegin, dwOffset);
		else return pBegin;
	}
	LPBYTE  QueryMapViewTama2(DWORD dwStartOffset, DWORD dwIdealMapSize)
	{
		ASSERT(dwIdealMapSize > 0);
		if(m_pMapStart)
		{
			DWORD dwLastOffset = dwStartOffset + dwIdealMapSize-1;
			if(dwLastOffset > m_dwTotal-1) dwLastOffset = m_dwTotal-1;
			if(IsOutOfMapTama(dwStartOffset) || IsOutOfMapTama(dwLastOffset))return _QueryMapViewTama2(dwStartOffset, dwIdealMapSize);
		}
		return GetFileMappingPointerFromFileOffset(dwStartOffset);
	}
	BOOL    IsOutOfMap(LPBYTE p) { return m_pMapStart ? IsOutOfMap1(p) : FALSE; }
	DWORD	GetMapSize() { return m_pMapStart ? m_dwFileOffset + m_dwMapSize : m_dwTotal; }
	DWORD	GetMapRemain(DWORD dwStart)
	{
		if(m_dwTotal==0)return 0;
		if(m_pMapStart)
		{
			DWORD dwMapSize = m_dwFileOffset + m_dwMapSize;
			if(m_dwFileOffset <= dwStart && dwStart < dwMapSize)return dwMapSize - dwStart;
		} else {
			if(m_dwTotal-1 >= dwStart)return m_dwTotal - dwStart;
		}
		return 0;
	}
private:
	BOOL     MapView();
	LPBYTE  QueryMapView1(LPBYTE pBegin, DWORD dwOffset);
	void	AlignMapSize(DWORD dwStartOffset, DWORD dwIdealMapSize);
	LPBYTE  _QueryMapViewTama2 (DWORD dwStartOffset, DWORD dwIdealMapSize);

	BOOL    IsOutOfMap1(LPBYTE p);
	BOOL IsOutOfMapTama(DWORD dwOffset)
	{
		if(m_pMapStart)
		{
			return (dwOffset < m_dwFileOffset || dwOffset >= m_dwFileOffset + m_dwMapSize);
		} else return true;
	}
#endif //FILE_MAPPING

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBZDoc)
	public:
	virtual void Serialize(CArchive& ar);
	void CBZDoc::SavePartial(CFile& file, DWORD offset, DWORD size);
	void CBZDoc::SavePartialInflated(CFile& file, DWORD offset, DWORD size, CBZView& view);
	virtual void DeleteContents();
	protected:
	//}}AFX_VIRTUAL

#ifdef FILE_MAPPING
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void ReleaseFile(CFile* pFile, BOOL bAbort);
	virtual BOOL SaveModified();
#endif //FILE_MAPPING

// Implementation
public:
	virtual ~CBZDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DWORD GetFileLength(CFile* pFile, BOOL bErrorMsg = FALSE);	// ###1.60

// Generated message map functions
protected:
	//{{AFX_MSG(CBZDoc)
	afx_msg void OnEditReadOnly();
	afx_msg void OnUpdateEditReadOnly(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnEditReadOnlyOpen();
	afx_msg void OnUpdateEditReadOnlyOpen(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
