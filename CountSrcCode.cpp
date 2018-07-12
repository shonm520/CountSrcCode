// CountSrcCode.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Shlwapi.h>
#include <stdlib.h>
#include "ThreadPool.h"
#include "CplusFileCounter.h"

#pragma comment(lib, "Shlwapi.lib")


CountInfo g_CountInfo;
int g_TotalFile;

class CppFileFilter
{
public:
	bool ValidFile(TCHAR* szFileName)  
	{
		TCHAR* pExt = PathFindExtension(szFileName);
		if ((_tcsicmp(pExt, _T(".cpp"))) == 0 ||
			(_tcsicmp(pExt, _T(".c"))) == 0 ||
			(_tcsicmp(pExt, _T(".h"))) == 0)
			return true;

		return false;
	}
};

template<class TFileFilter = CppFileFilter>
class CoutSrcCode
{
public:
	void Init(int thrdnums);  //开启线程个数
	void SearchAndCountFile(TCHAR* szFilePath);
	void CountFile(TCHAR* szFilePath);
	bool ValidFile(TCHAR* szFileName);

private:
	ThreadPool m_ThreadPool;
	TFileFilter m_FileFilter;
};

template<class TFileFilter>
void CoutSrcCode<TFileFilter>::Init(int thrdnums)
{
	m_ThreadPool.Init(thrdnums);
}

template<class TFileFilter>
bool CoutSrcCode<TFileFilter>::ValidFile(TCHAR* szFileName)
{
	return m_FileFilter.ValidFile(szFileName);
}

template<class TFileFilter>
void CoutSrcCode<TFileFilter>::CountFile(TCHAR* szFilePath)
{
	SearchAndCountFile(szFilePath);
	m_ThreadPool.NoMoreTaskEvent();

	DWORD dwRet = m_ThreadPool.WaitForCountFinish();
	if (dwRet == WAIT_OBJECT_0)  
	{
		if (g_TotalFile == 0)
		{
			_tprintf(_T("no relative file found\n"));
			return;
		}
		_tprintf(_T("total file num:%d\n"), g_TotalFile);
		_tprintf(_T("\n\nthe total code line:%d\n"), g_CountInfo.nTotal);
		_tprintf(_T("total empty line:%d\n"), g_CountInfo.nEmpty);
		_tprintf(_T("total effective line:%d\n"), g_CountInfo.nEffective);
		_tprintf(_T("total comment line:%d\n"), g_CountInfo.nComment);
	}
}

template<class TFileFilter>
void CoutSrcCode<TFileFilter>::SearchAndCountFile(TCHAR* szFilePath)
{
	TCHAR szFind[MAX_PATH] = {0};
	WIN32_FIND_DATA FindFileData;
	_tcscpy_s(szFind, szFilePath);
	PathAddBackslash(szFind);
	_tcscat_s(szFind, _T("*.*"));
	HANDLE hFind = ::FindFirstFile(szFind, &FindFileData);
	if (INVALID_HANDLE_VALUE == hFind)
		return;
	TCHAR szFile[MAX_PATH] = { 0 };
	static TCHAR* pPrjDir = NULL ;
	if (pPrjDir == NULL)  
	{
		pPrjDir = szFilePath;
	}
	do  
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)  
			continue;
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)  
		{
			if (FindFileData.cFileName[0] != '.')               //去掉.  和.. 的目录
			{
				_tcscpy_s(szFile, szFilePath);
				PathAddBackslash(szFile);
				_tcscat_s(szFile, FindFileData.cFileName);
				SearchAndCountFile(szFile);
			}
		}
		else  
		{
			if (ValidFile(FindFileData.cFileName))  
			{
				_tcscpy_s(szFile, szFilePath);
				PathAddBackslash(szFile);
				_tcscat_s(szFile, FindFileData.cFileName);
				CplusFileCounter* counter = new CplusFileCounter(pPrjDir, szFile);
				g_TotalFile++;
				m_ThreadPool.AddTask(counter);
			}
		}
	} while (FindNextFile(hFind, &FindFileData));
	FindClose(hFind);
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2)  
	{
		_tprintf(_T("there needs a folder to count\nplease input Enter to quit\n"));
		getchar();
		return -1;
	}
	CoutSrcCode<CppFileFilter> cc;
	cc.Init(10);
	TCHAR* pFile = argv[1];
	cc.CountFile(pFile);

	_tprintf(_T("\nplease input Enter to quit\n"));
	getchar();

	return 0;
}

