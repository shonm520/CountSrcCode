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

struct TP_THREAD_INFO		// �߳���Ϣ�ṹ
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
	static UINT APIENTRY ThreadProc(LPVOID lpParam);	// �̺߳���

private:
	std::vector<TP_THREAD_INFO *> m_arrThreadInfo;		// �߳���Ϣ����,�����˳�ʱ����߳�
	std::vector<ThreadPoolTask *> m_arrTask;			// ������Ϣ����

	CRITICAL_SECTION m_csThreadInfo;					// �߳���Ϣ�ٽ���
	CRITICAL_SECTION m_csTask;							// ������Ϣ�ٽ���
	HANDLE m_hEvent_Exit;								// �˳��߳�֪ͨ�¼����
	HANDLE m_hSemaphore_Task;							// ����֪ͨ�ź���
	HANDLE m_hEvent_NomoreTask;							// �����������¼�
	HANDLE m_hEvent_CountFinished;                      // ͳ������¼�

};

