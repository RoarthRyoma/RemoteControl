#pragma once

template<class T>
class CQueue
{//�̰߳�ȫ�Ķ���,����IOCP��ʵ��
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
		size_t nOperator;		//����
		T Data;	//����
		_beginthread_proc_type cbFunc;//�ص�
		HANDLE hEvent;	//pop������Ҫ��
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
	}PPARAM;	//Post Parameter ����Ͷ����Ϣ�Ľṹ��
private:
	static void threadEntry(void* arg);
	virtual void threadMain();
private:
	std::list<T>m_lstData;
	HANDLE m_hCompeletionPort;
	HANDLE m_hThread;
};

