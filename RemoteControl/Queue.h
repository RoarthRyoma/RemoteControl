#pragma once
#include "pch.h"
#include <list>
#include "Thread.h"

template<class T>
class CQueue
{//线程安全的队列,利用IOCP来实现
public:
	enum
	{
		EQNone,
		EQPush,
		EQPop,
		EQSize,
		EQClear
	};

	typedef struct IocpParam
	{
		size_t nOperator;		//操作
		T Data;	//数据
		HANDLE hEvent;	//pop操作需要的
		IocpParam(int op, const T& data, HANDLE hEve = NULL)
		{
			nOperator = op;
			Data = data;
			hEvent = hEve;
		}
		IocpParam()
		{
			nOperator = EQNone;
		}
	}PPARAM;	//Post Parameter 用于投递信息的结构体

	CQueue()
	{
		m_lock = false;
		m_hCompeletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
		m_hThread = INVALID_HANDLE_VALUE;
		if (m_hCompeletionPort != NULL)
		{
			m_hThread = (HANDLE)_beginthread(&CQueue<T>::threadEntry, 0, this);
		}
	}
	virtual ~CQueue()
	{
		if (m_lock) return;
		m_lock = true;
		PostQueuedCompletionStatus(m_hCompeletionPort, 0, NULL, NULL);
		WaitForSingleObject(m_hThread, INFINITE);
		if (m_hCompeletionPort != NULL)
		{
			HANDLE hTemp = m_hCompeletionPort;
			m_hCompeletionPort = NULL;
			CloseHandle(hTemp);
		}
	}
	bool PushBack(const T& data)
	{
		IocpParam* pParam = new IocpParam(EQPush, data);
		if (m_lock)
		{
			delete pParam;
			return false;
		}
		bool ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM), (ULONG_PTR)pParam, NULL);
		if (ret == false)
		{
			delete pParam;
		}
		//printf_s("push back done %d %08p\r\n", ret, (void*)pParam);
		return ret;
	}
	virtual bool PopFront(T& data)
	{
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		IocpParam Param(EQPop, data, hEvent);
		if (m_lock)
		{
			if (hEvent) CloseHandle(hEvent);
			return false;
		}
		bool ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM), (ULONG_PTR)&Param, NULL);
		if (ret == false)
		{
			CloseHandle(hEvent);
			return false;
		}
		ret = WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0;
		if (ret)
		{
			data = Param.Data;
		}
		return ret;
	}
	size_t Size()
	{
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		IocpParam Param(EQSize, T(), hEvent);
		if (m_lock)
		{
			if (hEvent) CloseHandle(hEvent);
			return -1;
		}
		bool ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM), (ULONG_PTR)&Param, NULL);
		if (ret == false)
		{
			CloseHandle(hEvent);
			return -1;
		}
		ret = WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0;
		if (ret)
		{
			return Param.nOperator;
		}
		return -1;
	}
	bool Clear()
	{
		if (m_lock)
		{
			return false;
		}
		IocpParam* pParam = new IocpParam(EQClear, T());
		bool ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM), (ULONG_PTR)pParam, NULL);
		if (ret == false)
		{
			delete pParam;
		}
		//printf_s("push back done %d %08p\r\n", ret, (void*)pParam);
		return ret;
	}

	
protected:
	static void threadEntry(void* arg)
	{
		CQueue<T>* that = (CQueue<T>*)arg;
		that->threadMain();
		_endthread();
	}

	virtual void DealParam(PPARAM* pParam)
	{
		switch (pParam->nOperator)
		{
		case EQPush:
			m_lstData.push_back(pParam->Data);
			delete pParam;
			//printf_s("delete %08p!\r\n", (void*)pParam);
			break;
		case EQPop:
			if (m_lstData.size() > 0)
			{
				pParam->Data = m_lstData.front();
				m_lstData.pop_front();
			}
			if (pParam->hEvent != NULL)
			{
				SetEvent(pParam->hEvent);
			}
			break;
		case EQSize:
			pParam->nOperator = m_lstData.size();
			if (pParam->hEvent != NULL)
			{
				SetEvent(pParam->hEvent);
			}
			break;
		case EQClear:
			m_lstData.clear();
			delete pParam;
			//printf_s("clear %08p!\r\n", (void*)pParam);
			break;
		default:
			OutputDebugStringA("unknown operator!\r\n");
			break;
		}
	}

	virtual void threadMain()
	{
		PPARAM* pParam = nullptr;
		ULONG_PTR CompletionKey = 0;
		OVERLAPPED* pOverlapped = nullptr;
		DWORD dwTransferred = 0;
		while (GetQueuedCompletionStatus(m_hCompeletionPort, &dwTransferred, &CompletionKey, &pOverlapped, INFINITE))
		{
			if (dwTransferred == 0 && CompletionKey == NULL)
			{
				//printf_s("thread is prepare to exit!\r\n");
				break;
			}
			pParam = (PPARAM*)CompletionKey;
			DealParam(pParam);
		}
		while (GetQueuedCompletionStatus(m_hCompeletionPort, &dwTransferred, &CompletionKey, &pOverlapped, 0))
		{
			if ((dwTransferred == 0) && (CompletionKey == NULL))
			{
				//printf_s("thread is prepare to exit!\r\n");
				continue;
			}
			pParam = (PPARAM*)CompletionKey;

			DealParam(pParam);
		}
		HANDLE hTemp = m_hCompeletionPort;
		m_hCompeletionPort = NULL;
		CloseHandle(hTemp);
	}
protected:
	std::list<T>m_lstData;
	HANDLE m_hCompeletionPort;
	HANDLE m_hThread;
	std::atomic<bool> m_lock;	//队列正在析构
};



template<class T>
class CSendQueue :public CQueue<T>, public ThreadFuncBase
{
public:
	typedef int (ThreadFuncBase::* EDYCALLBACK)(T& data);

	CSendQueue(ThreadFuncBase* obj, EDYCALLBACK callback): CQueue<T>(), m_base(obj), m_callback(callback)
	{
		m_thread.Start();
		m_thread.UpdateWorker(::ThreadWorker(this, (FUNCTYPE)&CSendQueue<T>::threadTick));
	}

	virtual ~CSendQueue()
	{
		m_base = NULL;
		m_callback = NULL;
		m_thread.Stop();
	}

protected:
	virtual bool PopFront(T& data)
	{
		return false;
	}

	bool PopFront()
	{
		typename CQueue<T>::IocpParam* Param = new typename CQueue<T>::IocpParam(CQueue<T>::EQPop, T());
		if (CQueue<T>::m_lock)
		{
			delete Param;
			return false;
		}
		bool ret = PostQueuedCompletionStatus(CQueue<T>::m_hCompeletionPort, sizeof(*Param), (ULONG_PTR)&Param, NULL);
		if (ret == false)
		{
			delete Param;
			return false;
		}
		return ret;
	}
	int threadTick()
	{
		if (WaitForSingleObject(CQueue<T>::m_hThread, 0) != WAIT_TIMEOUT)
		{
			return 0;
		}
		if (CQueue<T>::m_lstData.size() > 0)
		{
			PopFront();
		}
		Sleep(1);
		return 0;
	}
	virtual void DealParam(typename CQueue<T>::PPARAM* pParam)
	{
		switch (pParam->nOperator)
		{
		case CQueue<T>::EQPush:
			CQueue<T>::m_lstData.push_back(pParam->Data);
			delete pParam;
			//printf_s("delete %08p!\r\n", (void*)pParam);
			break;
		case CQueue<T>::EQPop:
			if (CQueue<T>::m_lstData.size() > 0)
			{
				pParam->Data = CQueue<T>::m_lstData.front();
				if ((m_base->*m_callback)(pParam->Data) == 0)
				{
					CQueue<T>::m_lstData.pop_front();
				}
			}
			delete pParam;
			break;
		case CQueue<T>::EQSize:
			pParam->nOperator = CQueue<T>::m_lstData.size();
			if (pParam->hEvent != NULL)
			{
				SetEvent(pParam->hEvent);
			}
			break;
		case CQueue<T>::EQClear:
			CQueue<T>::m_lstData.clear();
			delete pParam;
			//printf_s("clear %08p!\r\n", (void*)pParam);
			break;
		default:
			OutputDebugStringA("unknown operator!\r\n");
			break;
		}
	}
private:
	ThreadFuncBase* m_base;
	EDYCALLBACK m_callback;
	CThread m_thread;
};

typedef CSendQueue<std::vector<char>>::EDYCALLBACK SENDCALLBACK;