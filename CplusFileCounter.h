#pragma once
#include "ThreadPool.h"

#include <string>
using std::string;
using std::wstring;

#ifdef _UNICODE
typedef wstring tstring;
#else
typedef string tstring;
#endif // _UNICODE

class IFileCounter
{
public:
	IFileCounter(TCHAR* pPrjDir, TCHAR* pFileName);
	virtual ~IFileCounter();
	bool LoadFile();
	virtual void CountFile() = 0 ;

protected:
	tstring m_strFileName;
	static tstring s_strPrjDir;
	PVOID m_pvFile;
	DWORD m_dwFileSize;
private:
	HANDLE m_hFile;
	HANDLE m_hFileMap;
};

class CplusFileCounter : public IFileCounter, public ThreadPoolTask
{
public:
	CplusFileCounter(TCHAR* pPrjDir, TCHAR* pFileName);
	~CplusFileCounter();
	virtual void CountFile();
	virtual void Run();

};

