#include "stdafx.h"
#include <process.h>
#include "ThreadPool.h"


ThreadPoolTask::ThreadPoolTask()
{
	m_bRunning = FALSE;
}

ThreadPoolTask::~ThreadPoolTask()
{
}

bool ThreadPoolTask::IsRunning()
{
	return m_bRunning;
}

void ThreadPoolTask::SetRunning(bool bRunning)
{
	m_bRunning = bRunning;
	if (!m_bRunning)
		TaskFinish();
}

void ThreadPoolTask::Run()
{
	
}

int ThreadPoolTask::Stop()
{
	return 0;
}

void ThreadPoolTask::TaskFinish()
{
	return;
}

ThreadPool::ThreadPool()
{
	memset(&m_csThreadInfo, 0, sizeof(CRITICAL_SECTION));
	memset(&m_csTask, 0, sizeof(CRITICAL_SECTION));

	m_hSemaphore_Task = NULL;
	m_hEvent_Exit = NULL;
	m_hEvent_NomoreTask = NULL;
}

ThreadPool::~ThreadPool()
{
}

BOOL ThreadPool::Init(int nThreadNums)
{
	TP_THREAD_INFO * lpThreadInfo;
	HANDLE hThread;
	DWORD dwThreadID;

	if (nThreadNums <= 0)
		return FALSE;

	::InitializeCriticalSection(&m_csThreadInfo);
	::InitializeCriticalSection(&m_csTask);

	m_hSemaphore_Task = ::CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);	//刚开始设置任务量为0
	m_hEvent_Exit = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hEvent_NomoreTask = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hEvent_CountFinished = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	::EnterCriticalSection(&m_csThreadInfo);
	for (int i = 0; i < nThreadNums; i++)  
	{
		lpThreadInfo = new TP_THREAD_INFO;
		if (lpThreadInfo != NULL)  
		{
			hThread = (HANDLE)(ULONG_PTR)_beginthreadex(NULL, 0,
				&ThreadProc, lpThreadInfo, CREATE_SUSPENDED, (UINT*)&dwThreadID);
			if (hThread != NULL)  
			{
				lpThreadInfo->hThread = hThread;
				lpThreadInfo->lParam = (LPARAM)this;
				m_arrThreadInfo.push_back(lpThreadInfo);

				::ResumeThread(hThread);
			}
			else  
			{
				delete lpThreadInfo;
			}
		}
	}
	::LeaveCriticalSection(&m_csThreadInfo);
	return TRUE;
}

BOOL ThreadPool::AddTask(ThreadPoolTask * lpTask)
{
	if (NULL == lpTask)
		return FALSE;
	::EnterCriticalSection(&m_csTask);
	m_arrTask.push_back(lpTask);
	::LeaveCriticalSection(&m_csTask);
	::ReleaseSemaphore(m_hSemaphore_Task, 1, NULL);		// 增加任务通知信号量

	return TRUE;
}

void ThreadPool::NoMoreTaskEvent()
{
	SetEvent(m_hEvent_NomoreTask);
}

DWORD ThreadPool::WaitForCountFinish(DWORD dwMilli /* = INFINITE */)
{
	return WaitForSingleObject(m_hEvent_CountFinished, dwMilli);
}

// 线程函数
UINT ThreadPool::ThreadProc(LPVOID lpParam)
{
	TP_THREAD_INFO * lpThreadInfo;
	ThreadPool * lpThis;
	ThreadPoolTask * lpTask, *lpTempTask;

	lpThreadInfo = (TP_THREAD_INFO *)lpParam;
	if (NULL == lpThreadInfo)
		return 0;

	lpThis = (ThreadPool *)lpThreadInfo->lParam;
	if (NULL == lpThis)
		return 0;

	HANDLE hWaitEvent[3];
	hWaitEvent[0] = lpThis->m_hEvent_Exit;
	hWaitEvent[1] = lpThis->m_hSemaphore_Task;
	hWaitEvent[2] = lpThis->m_hEvent_NomoreTask;
	DWORD dwRet;
	for (;;)  
	{
		dwRet = ::WaitForMultipleObjects(3, hWaitEvent, FALSE, INFINITE);
		if (dwRet == WAIT_OBJECT_0)	 // 接到退出线程通知事件
			break;

		if (dwRet == WAIT_OBJECT_0 + 2)  
		{
			if (lpThis->m_arrTask.size() == 0)  
			{
				SetEvent(lpThis->m_hEvent_Exit);
				SetEvent(lpThis->m_hEvent_CountFinished);
				break ; 
			}
		}
		lpTask = NULL;
		::EnterCriticalSection(&lpThis->m_csTask);	// 取任务
		for (int i = 0; i < (int)lpThis->m_arrTask.size(); i++)  
		{
			lpTempTask = lpThis->m_arrTask[i];
			if (lpTempTask != NULL)  
			{
				if (!lpTempTask->IsRunning())  
				{
					lpTempTask->SetRunning(TRUE);
					lpTask = lpTempTask;
					break;
				}
			}
		}
		::LeaveCriticalSection(&lpThis->m_csTask);
		if (lpTask != NULL)		// 有任务
		{
			lpTask->Run();

			::EnterCriticalSection(&lpThis->m_csTask);	// 删除任务
			for (int i = 0; i < (int)lpThis->m_arrTask.size(); i++)  
			{
				lpTempTask = lpThis->m_arrTask[i];
				if (lpTempTask == lpTask)  
				{
					lpThis->m_arrTask.erase(lpThis->m_arrTask.begin() + i);
					break;
				}
			}
			lpTask->SetRunning(FALSE);
			delete lpTask;
			::LeaveCriticalSection(&lpThis->m_csTask);
		}
	}
	return 0;
}
