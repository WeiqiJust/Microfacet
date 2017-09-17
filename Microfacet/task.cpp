#include "task.h"

task_thread::task_thread()
	: b_running(false)
{
}

task_thread::~task_thread()
{
	stop_thread();
}

void task_thread::start_thread(void *param, LPTHREAD_START_ROUTINE rt, int t_sleep)
{
	if (!b_running)
	{
		sleep_time = t_sleep;
		b_running = true;
		h_thread = CreateThread(NULL, 0, rt, param, 0, NULL);
	}
}

void task_thread::stop_thread()
{
	if (b_running)
	{
		b_running = false;
		WaitForSingleObject(h_thread, INFINITE);
	}
}

DWORD WINAPI task_thread::thread_func(LPVOID pParam)
{
	task_thread_param *tp = (task_thread_param*)pParam;

	try
	{
		shared_ptr<subtask> t;
		while (tp->p_self->b_running)
		{
			int result = tp->p_manager->get_subtask(t);

			if (result <= 0)
				Sleep(tp->p_self->sleep_time);
			else {
				t->p_task->p_worker->work(t, tp->p_per_thread_global_var);
				tp->p_manager->complete_subtask(t);
			}
		}
	}
	catch (exception)
	{
		printf("Error on task thread function !\n");
		//e.report();
		return -1;
	}

	return 0;
}

task_manager::task_manager(const int n_threads, const int t_sleep)
	:threads(NULL), num_threads_busy(0), n_threads(n_threads)
{
	b_working = false;
	b_done = false;
	b_terminating = false;

	threads = new task_thread[n_threads];
	tps = new task_thread_param[n_threads];

	InitializeCriticalSection(&cs_alloc);
	InitializeCriticalSection(&cs_complete);

	for (int i = 0; i < n_threads; i++)
	{
		tps[i].thread_id = i;
		tps[i].p_manager = this;
		tps[i].p_self = &threads[i];
		tps[i].p_per_thread_global_var
			= NULL;

		threads[i].start_thread(&tps[i], task_thread::thread_func, t_sleep);
	}
}

task_manager::~task_manager()
{
	SAFE_DELETE_ARR(threads);
	SAFE_DELETE_ARR(tps);

	DeleteCriticalSection(&cs_alloc);
	DeleteCriticalSection(&cs_complete);
}

int task_manager::num_threads() const
{
	return n_threads;
}

int task_manager::n_threads_busy() const
{
	int n;
	EnterCriticalSection(&cs_alloc);
	n = num_threads_busy;
	LeaveCriticalSection(&cs_alloc);
	return n;
}

bool task_manager::is_terminating() const
{
	bool b_result;
	EnterCriticalSection(&cs_alloc);
	b_result = b_terminating;
	LeaveCriticalSection(&cs_alloc);
	return b_result;
}

bool task_manager::is_working() const
{
	bool b_result;
	EnterCriticalSection(&cs_alloc);
	b_result = b_working;
	LeaveCriticalSection(&cs_alloc);
	return b_result;
}

bool task_manager::is_done() const
{
	bool b_result;
	EnterCriticalSection(&cs_alloc);
	b_result = b_done;
	LeaveCriticalSection(&cs_alloc);
	return b_result;
}

bool task_manager::is_free() const
{
	return (!is_working() && !is_terminating() && !is_done());
}

bool task_manager::get_result(shared_ptr<task> &t)
{
	bool b_result;
	EnterCriticalSection(&cs_alloc);
	b_result = b_done;
	if (b_done)
	{
		b_done = false;
		t = t_done;
	}
	LeaveCriticalSection(&cs_alloc);
	return b_result;
}

void task_manager::terminate_all()
{
	if (is_terminating())
	{
		while (is_terminating())
		{
			Sleep(1);
		}
	}
	else {
		EnterCriticalSection(&cs_alloc);
		if (b_working)
			b_working = false;
		b_terminating = true;
		LeaveCriticalSection(&cs_alloc);

		while (n_threads_busy() > 0)
			Sleep(1);

		EnterCriticalSection(&cs_alloc);
		b_terminating = false;
		b_done = false;
		LeaveCriticalSection(&cs_alloc);
	}
}

int task_manager::get_subtask(shared_ptr<subtask> &t)
{
	EnterCriticalSection(&cs_alloc);
	if (!b_working)
	{
		LeaveCriticalSection(&cs_alloc);
		return -1;
	}

	int ret = t_working->p_worker->get_subtask(t, t_working);
	if (ret > 0)
	{
		t->p_task = t_working.get();
		LeaveCriticalSection(&cs_alloc);
		EnterCriticalSection(&cs_complete);
		num_threads_busy++;
		LeaveCriticalSection(&cs_complete);
		return ret;
	}

	LeaveCriticalSection(&cs_alloc);
	return 0;
}


void task_manager::complete_subtask(shared_ptr<subtask> &t)
{
	bool b_finish;

	EnterCriticalSection(&cs_complete);
	b_finish = t_working->p_worker->complete_subtask(t);
	num_threads_busy--;
	LeaveCriticalSection(&cs_complete);

	if (b_finish)
	{
		EnterCriticalSection(&cs_alloc);
		t_working->p_worker->finish_task(t_working);
		b_done = true;
		t_done = t_working;
		b_working = false;
		LeaveCriticalSection(&cs_alloc);
	}
}

bool task_manager::start(const shared_ptr<task> &t)
{
	if (!is_free())
		return false;

	EnterCriticalSection(&cs_alloc);
	t_working = t;
	for (int i = 0; i < n_threads; i++)
		tps[i].p_per_thread_global_var = t_working->per_thread_global_var ?
		(*t_working->per_thread_global_var)[i] : NULL;
	t->p_worker->init_task(t_working);

	num_threads_busy = 0;
	b_working = true;
	b_done = false;
	LeaveCriticalSection(&cs_alloc);

	if (!t->b_async)
	{
		while (is_working())
			Sleep(0);
	}
	return true;
}

