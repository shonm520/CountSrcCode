#include "stdafx.h"
#include "CplusFileCounter.h"

extern CountInfo g_CountInfo;

tstring IFileCounter::s_strPrjDir;

IFileCounter::IFileCounter(TCHAR* pPrjDir, TCHAR* pFileName) :
m_strFileName(pFileName),
m_pvFile(NULL),
m_hFile(INVALID_HANDLE_VALUE),
m_hFileMap(NULL),
m_dwFileSize(0)
{
	if (s_strPrjDir.empty())
	{
		s_strPrjDir = pPrjDir;
	}
}

IFileCounter::~IFileCounter()
{
	if (m_hFile != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFile);
	if (m_pvFile)
		UnmapViewOfFile(m_pvFile);
	if (m_hFileMap) 
		CloseHandle(m_hFileMap);
}

bool IFileCounter::LoadFile()
{
	m_hFile = CreateFile(m_strFileName.c_str(), GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if (m_hFile == INVALID_HANDLE_VALUE)
		return false;

	m_dwFileSize = GetFileSize(m_hFile, NULL);
	m_hFileMap = CreateFileMapping(m_hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (m_hFileMap == NULL)
		return false;

	m_pvFile = MapViewOfFile(m_hFileMap, FILE_MAP_READ, 0, 0, 0);
	if (m_pvFile == NULL)
		return false;

	return true;
}

CplusFileCounter::CplusFileCounter(TCHAR* pPrjDir, TCHAR* pFileName) :
IFileCounter(pPrjDir, pFileName)
{
	
}

CplusFileCounter::~CplusFileCounter()
{
}

void CplusFileCounter::CountFile()
{
	int index = 0;
	int nTotal = 0;
	int nEmptyLine = 0;
	int nComment = 0;
	PSTR pStr = (PSTR)m_pvFile;
	bool bEmptyLine = true;
	int nSlash = 0;  //ע��//�ĸ���
	bool bCommentStart = false;
	int nLineInComment = 0;
	int nEffective = 0;  //��Ч��
	bool bEffective = false;
	do
	{
		char p = pStr[index++];
		char pNext = pStr[index];
		if (p != 0x20 && p != '\n' && p != '\r')  //�������в��ǿ���
		{
			bEmptyLine = false;
			if (p != '/' && p != '*' && !bCommentStart && nSlash == 0)
				bEffective = true;
		}
		if (p == '\n' || index >= (int)m_dwFileSize)   //������һ�л��ߵ����һ���ַ�ʱ����
		{
			if (bEmptyLine) nEmptyLine++;
			if (bEffective) nEffective++;
			if (nSlash >= 1 && !bCommentStart)  //�������ϵ�/Ϊһ��ע����,���Ҳ���/**/Ƕ��ע����
				nComment++;
			if (bCommentStart && !bEmptyLine)   //����ע���еĿ��в���ע����			
				nLineInComment++;

			nTotal++;
			bEmptyLine = true;                   //������һ��Ϊ����
			nSlash = 0;                          //������һ��û��/
			bEffective = false;
			if (index >= (int)m_dwFileSize)
			{
				if (p == '\n')  //���һ���ַ��պ��ǻ���ʱ
				{
					nTotal++;  
					nEmptyLine++;
				}
				break;
			}
		}
		if (p == '/' && pNext == '/')
			nSlash++;
		if (p == '/' && pNext == '*')
		{
			bCommentStart = true;
			if (nSlash == 0)
			{   //����ע�Ϳ�ʼ�н���
				nComment += 1;
			}
		}
		if (p == '*' && pNext == '/')
		{
			nComment += nLineInComment;
			bCommentStart = false;
			nLineInComment = 0;
		}
	} while (index < (int)m_dwFileSize);

	InterlockedExchangeAdd(&g_CountInfo.nEmpty, nEmptyLine);  //ԭ�Ӳ��������߳�ͬʱ�޸ĵ���Ҫ
	InterlockedExchangeAdd(&g_CountInfo.nTotal, nTotal);
	InterlockedExchangeAdd(&g_CountInfo.nEffective, nEffective);
	InterlockedExchangeAdd(&g_CountInfo.nComment, nComment);

	_tprintf(_T("%s\t\t total:%4d empty:%3d effective:%4d comment:%3d\n"),
		m_strFileName.substr(s_strPrjDir.length() + 1).c_str(), nTotal, nEmptyLine, nEffective, nComment);
}

void CplusFileCounter::Run()
{
	if (!LoadFile())
	{
		_tprintf(_T("cannot load file : %s\n"), m_strFileName.substr(s_strPrjDir.length() + 1).c_str());
	}
	CountFile();
}
