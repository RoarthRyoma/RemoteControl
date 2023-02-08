#pragma once

template<class T>
class CQueue
{//线程安全的队列,利用IOCP来实现
public:
	CQueue();
	~CQueue();
	bool PushBack(const T& data);
	virtual bool PopFront(T& data);
	size_t Size();
	bool Clear();

	enum
	{
		//EQNone,
		EQPush,
		EQPop,
		EQSize,
		EQClear
	};

	typedef struct IocpParam
	{
		size_t nOperator;		//操作
		T Data;	//数据
		_beginthread_proc_type cbFunc;//回调
		HANDLE hEvent;	//pop操作需要的
		IocpParam(int op, const T& data, _beginthread_proc_type cb = NULL, HANDLE hEve = NULL)
		{
			nOperator = op;
			Data = data;
			cbFunc = cb;
			hEvent = hEve;
		}
		IocpParam()
		{
			nOperator = EQNone;
		}
	}PPARAM;	//Post Parameter 用于投递信息的结构体
private:
	static void threadEntry(void* arg);
	virtual void threadMain();
private:
	std::list<T>m_lstData;
	HANDLE m_hCompeletionPort;
	HANDLE m_hThread;
};

