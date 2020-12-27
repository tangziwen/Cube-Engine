#ifndef WORKER_THREAD_SYSTEM_H
#define WORKER_THREAD_SYSTEM_H
#include "EngineDef.h"
#include <functional>
#include <vector>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
namespace tzw{
	typedef std::function<void ()> VoidJob;
	class WorkerJob
	{
	public:
		WorkerJob(VoidJob work, VoidJob finish);
		WorkerJob(VoidJob work);
		WorkerJob();
		VoidJob m_work;
		VoidJob m_onFinished;
	};
	//typedef std::function<void ()> WorkerJob;
	class WorkerThreadSystem: public Singleton<WorkerThreadSystem>
	{
	public:
		WorkerThreadSystem();
		void pushOrder(WorkerJob order);
		void pushMainThreadOrder(WorkerJob order);
		void pushMainThreadOrderWithLoading(std::string tipsInfo, WorkerJob order);
		void init();
		void workderUpdate();
		void mainThreadUpdate();
	private:
		std::list<WorkerJob> m_JobRecieverList;
		std::list<WorkerJob> m_jobProcessList;
		std::list<WorkerJob> m_mainThreadFunctionList;
		std::list<WorkerJob> m_mainThreadCB1;
		std::list<WorkerJob> m_mainThreadCB2;
		std::thread * m_thread;
		std::mutex m_mutex;
		std::condition_variable m_cond_var;
	};
}

#endif
