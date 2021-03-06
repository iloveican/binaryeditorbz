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
#include "BZInspectView.h"
#include "MainFrm.h"
#include "zlib.h"


static WORD crc16_table[256];
static DWORD crc32_table[256];

// CBZInspectView

IMPLEMENT_DYNCREATE(CBZInspectView, CFormView)

CBZInspectView::CBZInspectView()
	: CFormView(CBZInspectView::IDD)
	, m_bSigned(true)
{

}

CBZInspectView::~CBZInspectView()
{
}

void CBZInspectView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDE_INS_HEX, m_editHex);
	DDX_Control(pDX, IDE_INS_8BITS, m_edit8bits);
	DDX_Control(pDX, IDE_INS_BINARY1, m_editBinary1);
	DDX_Control(pDX, IDE_INS_BINARY2, m_editBinary2);
	DDX_Control(pDX, IDE_INS_BINARY4, m_editBinary4);
	DDX_Control(pDX, IDE_INS_BINARY8, m_editBinary8);
	DDX_Control(pDX, IDE_INS_FLOAT, m_editFloat);
	DDX_Control(pDX, IDE_INS_DOUBLE, m_editDouble);
	DDX_Control(pDX, IDC_INS_INTEL, m_check_intel);
	DDX_Control(pDX, IDC_INS_SIGNED, m_check_signed);
	DDX_Control(pDX, IDC_INS_STATIC1, m_staticBinary1);
	DDX_Control(pDX, IDC_INS_STATIC2, m_staticBinary2);
	DDX_Control(pDX, IDC_INS_STATIC4, m_staticBinary4);
	DDX_Control(pDX, IDC_INS_STATIC8, m_staticBinary8);
	DDX_Control(pDX, IDE_INS_FILETIME, m_editFileTime);
	DDX_Control(pDX, IDC_INS_CALCSUM, m_buttonCalcsum);
	DDX_Control(pDX, IDE_INS_CRC16, m_editCRC16);
	DDX_Control(pDX, IDE_INS_CRC32, m_editCRC32);
	DDX_Control(pDX, IDE_INS_ADLER32, m_editAdler32);
	DDX_Control(pDX, IDE_INS_MD5, m_editMD5);
	DDX_Control(pDX, IDE_INS_SHA1, m_editSHA1);
	DDX_Control(pDX, IDE_INS_SHA256, m_editSHA256);
}

BEGIN_MESSAGE_MAP(CBZInspectView, CFormView)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_INS_INTEL, &CBZInspectView::OnBnClickedInsIntel)
	ON_BN_CLICKED(IDC_INS_SIGNED, &CBZInspectView::OnBnClickedInsSigned)
	ON_BN_CLICKED(IDC_INS_CALCSUM, &CBZInspectView::OnClickedInsCalcsum)
END_MESSAGE_MAP()


// CBZInspectView 診断

#ifdef _DEBUG
void CBZInspectView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CBZInspectView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CBZInspectView メッセージ ハンドラ

int CBZInspectView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	if(options.xSplitStruct == 0)
		options.xSplitStruct = lpCreateStruct->cx;

	return 0;
}

void CBZInspectView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。
	m_check_intel.SetCheck(!options.bByteOrder);
	m_check_signed.SetCheck(m_bSigned);
	_UpdateChecks();

	m_pView = (CBZView*)GetNextWindow();
}

void CBZInspectView::ClearAll(void)
{
	m_edit8bits.SetWindowText(_T(""));
	m_editHex.SetWindowText(_T(""));
	m_editBinary1.SetWindowText(_T(""));
	m_editBinary2.SetWindowText(_T(""));
	m_editBinary4.SetWindowText(_T(""));
	m_editBinary8.SetWindowText(_T(""));
	m_editFloat.SetWindowText(_T(""));
	m_editDouble.SetWindowText(_T(""));
	m_editFileTime.SetWindowText(_T(""));

	ClearSums();
}

void CBZInspectView::OnBnClickedInsIntel()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	options.bByteOrder = !options.bByteOrder;
	UpdateChecks();
	Update();
}

void CBZInspectView::OnBnClickedInsSigned()
{
	// TODO: ここにコントロール通知ハンドラ コードを追加します。
	m_bSigned = !m_bSigned;
	_UpdateChecks();

	if(m_bSigned)
	{
		m_staticBinary1.SetWindowText(_T("char"));
		m_staticBinary2.SetWindowText(_T("short"));
		m_staticBinary4.SetWindowText(_T("int"));
		m_staticBinary8.SetWindowText(_T("int64"));
	} else {
		m_staticBinary1.SetWindowText(_T("uchar"));
		m_staticBinary2.SetWindowText(_T("ushort"));
		m_staticBinary4.SetWindowText(_T("uint"));
		m_staticBinary8.SetWindowText(_T("uint64"));
	}
	Update();
}

CString BYTE2BitsCString(BYTE d)
{
	CString str;

	for(unsigned char mask=0x80;mask!=0;mask>>=1)
		str.AppendChar(d&mask ? '1' : '0');
	return str;
}

// ttp://blog.goo.ne.jp/masaki_goo_2006/e/a826604d8954db71505f3467080315f3
static void make_crc16_table(void)
{
	static int initialized = 0;
	if(initialized) return; else initialized = 1;

	for (WORD i = 0; i < 256; i++) {
		WORD c = i;
		for (int j = 0; j < 8; j++) {
			c = (c & 1) ? (0x1021 ^ (c >> 1)) : (c >> 1);
		}
		crc16_table[i] = c;
	}
}

// ttp://ja.wikipedia.org/wiki/%E5%B7%A1%E5%9B%9E%E5%86%97%E9%95%B7%E6%A4%9C%E6%9F%BB
static void make_crc32_table(void)
{
	static int initialized = 0;
	if(initialized) return; else initialized = 1;

	for (DWORD i = 0; i < 256; i++) {
		DWORD c = i;
		for (int j = 0; j < 8; j++) {
			c = (c & 1) ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
		}
		crc32_table[i] = c;
	}
}

static HCRYPTHASH crypthash_create(HCRYPTPROV cprov, ALG_ID alg)
{
	HCRYPTHASH chash;

	if(cprov == 0)
		return 0;

	if(!CryptCreateHash(cprov, alg, 0, 0, &chash))
		chash = 0;

	return chash;
}

static void crypthash_hashdata(HCRYPTHASH chash, const BYTE *p, DWORD r)
{
	BOOL ok;

	if(chash == 0)
		return;

	ok = CryptHashData(chash, p, r, 0);
	ASSERT(ok != 0);
}

static DWORD crypthash_hashsize(HCRYPTHASH chash)
{
	DWORD hs, dlen;
	BOOL ok;

	dlen = sizeof(hs);
	ok = CryptGetHashParam(chash, HP_HASHSIZE, (BYTE*)&hs, &dlen, 0);
	ASSERT(ok != 0);
	return hs;
}

// Note: Only "dlen <= 32" is supported.
static void crypthash_setstrval(CString *strval, CEdit *edit, HCRYPTHASH chash, DWORD hashlen)
{
	if(!chash)
	{
		edit->SetWindowText(_T("N/A in CSP"));
		return;
	}

	DWORD dlen = hashlen;
	BYTE hash[32]; // XXX: hashlen must be <= 32
	ASSERT(hashlen <= 32);

	if(!CryptGetHashParam(chash, HP_HASHVAL, hash, &dlen, 0))
	{
		edit->SetWindowText(_T("Error in CryptGetHashParam"));
		return;
	}

	ASSERT(dlen == hashlen);

	strval->SetString(_T(""));
	// TODO: for-AppendFormat-byteは遅い。
	for(DWORD i = 0; i < hashlen; i++) strval->AppendFormat(_T("%02X"), hash[i]);
	edit->SetWindowText(*strval);
}

void CBZInspectView::CalculateSums(void)
{
	ASSERT(m_pView != NULL);
	ASSERT(m_pView->IsBlockAvailable());

	// TODO: 4GB越え対応
	DWORD begin = m_pView->BlockBegin();
	DWORD end = m_pView->BlockEnd();
	DWORD remain = end - begin;

	// ブロックが0byteの時も表示する。(まれに初期値が知りたい場合がある…。)
	// このチェックは無効にしておく。
	//ASSERT(remain > 0);

	// context初期化
	make_crc16_table();
	WORD crc16 = 0xFFFF;
	make_crc32_table();
	DWORD crc32_1 = 0xFFFFFFFF;
	DWORD crc32_2 = crc32(0L, Z_NULL, 0);
	DWORD r_adler32 = adler32(0L, Z_NULL, 0);
	// TODO: CryptAcquireContextやCryptCreateHashのエラー時HCRYPT*を0にしているけど、0はあり得ない数値なのか?
	//       Win7SP1(x64)ではエラー時、特に値が書き換えられない…。
	//       INVALID_HANDLE_VALUEは型が合わない、HANDLEにしか使えないのかな。
	HCRYPTPROV cprov;
	if(!CryptAcquireContext(&cprov, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
	{
		if(!CryptAcquireContext(&cprov, NULL, MS_ENH_RSA_AES_PROV_XP, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
		{
			cprov = 0;
		}
	}
	HCRYPTHASH ch_md5 = crypthash_create(cprov, CALG_MD5);
	HCRYPTHASH ch_sha1 = crypthash_create(cprov, CALG_SHA1);
	// WinXP SP2まではSHA256をサポートしていない。
	HCRYPTHASH ch_sha256 = crypthash_create(cprov, CALG_SHA_256);

	// sum計算
	CBZDoc *doc = m_pView->GetDocument();

	while(remain > 0)
	{
		LPBYTE p = doc->QueryMapViewTama2(begin, remain);
		// TODO: 4GB越え対応
		DWORD mr = doc->GetMapRemain(begin);
		DWORD r = min(mr, remain);

		// 余り分をFinalする時、ハッシュ関数によってずれがある…どうする?
		// →CryptServiceProviderではAPIでカバーしてくれるので考慮の必要なし。
		for(DWORD i = 0; i < r; )
		{
			// TODO: p+iからrまでcontextをupdate
			crc16 = crc16_table[(crc16 ^ p[i]) & 0xFF] ^ (crc16 >> 8);
			crc32_1 = crc32_table[(crc32_1 ^ p[i]) & 0xFF] ^ (crc32_1 >> 8);
			i += 1;
		}

		crc32_2 = crc32(crc32_2, p, r);
		r_adler32 = adler32(r_adler32, p, r);

		crypthash_hashdata(ch_md5, p, r);
		crypthash_hashdata(ch_sha1, p, r);
		crypthash_hashdata(ch_sha256, p, r);

		remain -= r;
		begin += r;
	}

	// 結果表示
	CString strval;

	strval.Format(_T("%04X (%04X)"), crc16^0xFFFF, crc16);
	m_editCRC16.SetWindowText(strval);

	// TODO: zlibの結果を採用すべきかな
	ASSERT((crc32_1^0xFFFFFFFF) == crc32_2);
	strval.Format(_T("%08X (%08X)"), crc32_1^0xFFFFFFFF, crc32_1);
	m_editCRC32.SetWindowText(strval);

	strval.Format(_T("%08X"), r_adler32);
	m_editAdler32.SetWindowText(strval);

	if(cprov)
	{
		// paranoid
		if(ch_md5) ASSERT(crypthash_hashsize(ch_md5) == 16);
		if(ch_sha1) ASSERT(crypthash_hashsize(ch_sha1) == 20);
		if(ch_sha256) ASSERT(crypthash_hashsize(ch_sha256) == 32);

		crypthash_setstrval(&strval, &m_editMD5, ch_md5, 16);
		crypthash_setstrval(&strval, &m_editSHA1, ch_sha1, 20);
		crypthash_setstrval(&strval, &m_editSHA256, ch_sha256, 32);
	} else
	{
		m_editMD5.SetWindowText(_T("CSP error"));
		m_editSHA1.SetWindowText(_T("CSP error"));
		m_editSHA256.SetWindowText(_T("CSP error"));
	}

	// 片付け
	if(ch_md5) CryptDestroyHash(ch_md5);
	if(ch_sha1) CryptDestroyHash(ch_sha1);
	if(ch_sha256) CryptDestroyHash(ch_sha256);
	if(cprov) CryptReleaseContext(cprov, 0);
}

void CBZInspectView::ClearSums(void)
{
	m_editCRC16.SetWindowText(_T(""));
	m_editCRC32.SetWindowText(_T(""));
	m_editAdler32.SetWindowText(_T(""));
	m_editMD5.SetWindowText(_T(""));
	m_editSHA1.SetWindowText(_T(""));
	m_editSHA256.SetWindowText(_T(""));
}

void CBZInspectView::Update(void)
{
	int val;
	CString strVal;
	
//	val = m_pView->GetValue(m_pView->m_dwCaret, 4);
//	void *pVal = &val;
	
	ULONGLONG qval = m_pView->GetValue64(m_pView->m_dwCaret);
	void *pVal = &qval;

	val = m_pView->GetValue(m_pView->m_dwCaret, 1);
	strVal = SeparateByComma(m_bSigned ? (int)(char)val : val, m_bSigned);
	m_editBinary1.SetWindowText(strVal);

	val = m_pView->GetValue(m_pView->m_dwCaret, 2);
	strVal = SeparateByComma(m_bSigned ? (int)(short)val : val, m_bSigned);
	m_editBinary2.SetWindowText(strVal);

	val = m_pView->GetValue(m_pView->m_dwCaret, 4);
	strVal = SeparateByComma(val, m_bSigned);
	m_editBinary4.SetWindowText(strVal);

	strVal = SeparateByComma64(qval, m_bSigned);
	m_editBinary8.SetWindowText(strVal);

	val = m_pView->GetValue(m_pView->m_dwCaret, 4);
	float ft = *((float*)&val);
	strVal.Format(_T("%f"), ft);
	m_editFloat.SetWindowText(strVal);

	strVal.Format(_T("%f"), *((double*)&qval));
	m_editDouble.SetWindowText(strVal);

	strVal.Format(_T("0x%08X %08X"), *( ((int*)pVal)+1 ), *( ((int*)pVal)+0 ));
//	strVal.Format("0x%016I64X", qval);
	m_editHex.SetWindowText(strVal);

	CString str8bits = BYTE2BitsCString(*( ((BYTE*)pVal)+7 ));
	strVal = str8bits + _T(" ");
	str8bits = BYTE2BitsCString(*( ((BYTE*)pVal)+6 ));
	strVal += str8bits + _T(" ");
	str8bits = BYTE2BitsCString(*( ((BYTE*)pVal)+5 ));
	strVal += str8bits + _T(" ");
	str8bits = BYTE2BitsCString(*( ((BYTE*)pVal)+4 ));
	strVal += str8bits + _T(" - ");
	str8bits = BYTE2BitsCString(*( ((BYTE*)pVal)+3 ));
	strVal += str8bits + _T(" ");
	str8bits = BYTE2BitsCString(*( ((BYTE*)pVal)+2 ));
	strVal += str8bits + _T(" ");
	str8bits = BYTE2BitsCString(*( ((BYTE*)pVal)+1 ));
	strVal += str8bits + _T(" ");
	str8bits = BYTE2BitsCString(*( ((BYTE*)pVal)+0 ));
	strVal += str8bits;
	m_edit8bits.SetWindowText(strVal);

	CString strTime;
	SYSTEMTIME st;
	FileTimeToSystemTime((FILETIME*)&qval, &st);
	strTime.Format(_T("%04d/%02d/%02d %02d:%02d:%02d.%07d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, (qval % 10000000));
	m_editFileTime.SetWindowText(strTime);

	// ブロックが0byteの時も表示する。(まれに初期値が知りたい場合がある…。)
	if(m_pView->IsBlockAvailable() /*&& (m_pView->BlockEnd() - m_pView->BlockBegin()) > 0*/)
	{
		// 小さいサイズならこの場で計算する。
		// TODO: 4096は適当、設定可能にする。
		if((m_pView->BlockEnd() - m_pView->BlockBegin()) > 4096)
		{
			m_buttonCalcsum.EnableWindow(TRUE);
			ClearSums();
		} else
		{
			m_buttonCalcsum.EnableWindow(FALSE);
			CalculateSums();
		}
	} else
	{
		m_buttonCalcsum.EnableWindow(FALSE);
		ClearSums();
	}
}

void CBZInspectView::UpdateChecks(void)
{
	GetMainFrame()->UpdateInspectViewChecks();
}


void CBZInspectView::_UpdateChecks(void)
{
	m_check_intel.SetCheck(!options.bByteOrder);
	m_check_signed.SetCheck(m_bSigned);
}

void CBZInspectView::OnClickedInsCalcsum()
{
	CalculateSums();
	m_buttonCalcsum.EnableWindow(FALSE);
}
