#pragma once
#include "utils.h"
class task_thread;
class task_manager;
class task_worker;

//task definitions
class task
{
public:
	bool				b_async;
	task_worker			*p_worker;
	std::vector<void*>	*per_thread_global_var;

	task() : b_async(true), p_worker(NULL), per_thread_global_var(NULL) {};
	virtual ~task() {};
};

class subtask
{
public:
	task *p_task;
	virtual ~subtask() {};
};

//thread definitions
class task_thread_param
{
public:
	int				thread_id;
	task_thread		*p_self;
	task_manager	*p_manager;
	void*			p_per_thread_global_var;
};

class task_thread
{
protected:
	HANDLE h_thread;

public:
	bool	b_running;
	int		sleep_time;

	task_thread();
	~task_thread();

	static DWORD WINAPI thread_func(LPVOID pParam);
	void start_thread(void *param, LPTHREAD_START_ROUTINE rt = thread_func, int t_sleep = 1);
	void stop_thread();
};

//the worker
class task_worker
{
public:
	virtual void work(shared_ptr<subtask> &t, void*	p_per_thread_global_var) = 0;

	//BE CAREFUL:
	//It is the manager's responsibility to make sure the following procedures will not be re-entered!!!.
	virtual void init_task(shared_ptr<task> &t) = 0;
	virtual void finish_task(shared_ptr<task> &t) = 0;

	virtual int get_subtask(shared_ptr<subtask> &st, shared_ptr<task> &t) = 0;
	virtual bool complete_subtask(shared_ptr<subtask> &t) = 0;
};

//the manager
class task_manager
{
private:
	int		num_threads_busy, n_threads;
	shared_ptr<task>
		t_working, t_done;
	bool	b_working, b_done, b_terminating;

	task_thread			*threads;
	task_thread_param	*tps;
	mutable CRITICAL_SECTION
		cs_alloc, cs_complete;

	int n_threads_busy() const;

public:
	task_manager(const int n_threads, const int t_sleep);
	~task_manager();

	int num_threads() const;

	int get_subtask(shared_ptr<subtask> &t);
	void complete_subtask(shared_ptr<subtask> &t);

	bool start(const shared_ptr<task> &t);
	bool is_terminating() const;
	bool is_working() const;
	bool is_done() const;

	bool is_free() const;
	bool get_result(shared_ptr<task> &t);

	void terminate_all();
};