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
#include "BZScriptInterface.h"
#include "BZScriptWrapper.h"


BZScriptWrapper::BZScriptWrapper(void)
{
}


BZScriptWrapper::~BZScriptWrapper(void)
{
}


BOOL BZScriptWrapper::getNextHistory(CString &str)
{
	if(histidx < history.GetCount())
	{
		histidx++;
		if(histidx < history.GetCount())
		{
			str = history.GetAt(histidx);
		} else
		{
			str = CString(_T(""));
		}
		return TRUE;
	}
	return FALSE;
}


BOOL BZScriptWrapper::getPrevHistory(CString &str)
{
	if(0 < histidx)
	{
		histidx--;
		str = history.GetAt(histidx);
		return TRUE;
	}
	return FALSE;
}


BOOL BZScriptWrapper::init(BZScriptInterface *sif, CBZScriptView *view)
{
	if(!sif->init(view))
		return FALSE;

	this->sif = sif;
	histidx = 0;

	return TRUE;
}


CString BZScriptWrapper::run(CBZScriptView *view, CString str)
{
	// �����̍Ō�Ɠ����Ȃ痚���ɒǉ����Ȃ�
	if(history.GetCount() == 0 || history.GetAt(history.GetCount() - 1) != str)
	{
		histidx = history.Add(str) + 1;
	} else
	{
		histidx = history.GetCount();
	}

	return sif->run(view, CStringA(str));
}
