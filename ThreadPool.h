#pragma once
#include <vector>

class ThreadPoolTask
{
public:
	ThreadPoolTask();
	virtual ~ThreadPoolTask();

public:
	virtual bool IsRunning();
	virtual void SetRunning(bool bRunning);
	virtual void Run();
	virtual int Stop();
	virtual void TaskFinish();

private:
	bool m_bRunning;
};

struct TP_THREAD_INFO		// 线程信息结构
{
	HANDLE hThread;
	LPARAM lParam;
};

class ThreadPool
{
public:
	ThreadPool();
	~ThreadPool();

public:
	BOOL Init(int nThreadNums);
	BOOL AddTask(ThreadPoolTask * lpTask);
	void NoMoreTaskEvent();
	DWORD WaitForCountFinish(DWORD dwMilli = INFINITE);

private:
	static UINT APIENTRY ThreadProc(LPVOID lpParam);	// 线程函数

private:
	std::vector<TP_THREAD_INFO *> m_arrThreadInfo;		// 线程信息数组,用于退出时清除线程
	std::vector<ThreadPoolTask *> m_arrTask;			// 任务信息数组

	CRITICAL_SECTION m_csThreadInfo;					// 线程信息临界区
	CRITICAL_SECTION m_csTask;							// 任务信息临界区
	HANDLE m_hEvent_Exit;								// 退出线程通知事件句柄
	HANDLE m_hSemaphore_Task;							// 任务通知信号量
	HANDLE m_hEvent_NomoreTask;							// 任务添加完毕事件
	HANDLE m_hEvent_CountFinished;                      // 统计完毕事件

};

