#include "DispatchMsgService.h"
#include <algorithm>
#include "bike.pb.h"
#include "events_def.h"



DispatchMsgService* DispatchMsgService::DMS_ = nullptr;
int DispatchMsgService::clientNumber = 0;
std::queue<iEvent *> DispatchMsgService::response_events;
pthread_mutex_t DispatchMsgService::queue_mutext;
NetworkInterface* DispatchMsgService::NTIF_ = nullptr;

DispatchMsgService::DispatchMsgService():tp(nullptr)
{
	
	
}

DispatchMsgService::~DispatchMsgService()
{

}

BOOL DispatchMsgService::open()
{
	svr_exit_ = false;

	thread_mutex_create(&queue_mutext);
	LOG_DEBUG("�̳߳س�ʼ��......");
	//printf("�̳߳س�ʼ��......\n");
	tp = thread_pool_init();
	LOG_DEBUG("��ǰ�̳߳��е��߳���Ϊ��tp->threads=%d", tp->threads);
	LOG_DEBUG("�̳߳��ܴ�������������Ϊ��tp->max_queue=%d", tp->max_queue);
	LOG_DEBUG("�̳߳س�ʼ���ɹ���");
	//printf("�̳߳س�ʼ���ɹ���\n");
	return tp ? TURE : FALSE;
}

void DispatchMsgService::close()
{
	svr_exit_ = true;

	thread_pool_destroy(tp);
	thread_mutex_destroy(&queue_mutext);
	subscribers_.clear();

	tp = NULL;
}

void DispatchMsgService::subscribe(u32 eid, iEventHandler * handler)
{
	//LOG_DEBUG("DispatchMsgService::subscribe eid:%d\n", eid);
	T_EventHandlersMap::iterator iter = subscribers_.find(eid);
	/*map<string, int> m_stlmap;
	m_stlmap[��xiaomi��] = 88;]

	auto mpit = m_stlmap.begin();
	first��õ�Map��key����Чֵ��
	second��õ�Map��value����Чֵ��

	����
	mpit ->first; // �õ��� string ֵ�� ��xiaomi��
	mpit ->second; //�õ��� int ֵ�� 88*/
	/*find �㷨�᷵��һ��ָ���ҵ�����ĵ����������û���ҵ����󣬻᷵��������еĽ���������*/
	if (iter != subscribers_.end())
	{
		
		T_EventHandlers::iterator hdl_iter = std::find(iter->second.begin(), iter->second.end(), handler);
		if (hdl_iter == iter->second.end())
		{
			iter->second.push_back(handler);

		}
	}
	else
	{
		subscribers_[eid].push_back(handler);
	}
}

void DispatchMsgService::unsubscribe(u32 eid, iEventHandler * handler)
{
	T_EventHandlersMap::iterator iter = subscribers_.find(eid);
	if (iter != subscribers_.end())
	{
		T_EventHandlers::iterator hdl_iter = std::find(iter->second.begin(), iter->second.end(), handler);
		if (hdl_iter != iter->second.end())
		{
			iter->second.erase(hdl_iter);//�൱�ڲ�����ֵ//
		}
	}
}

i32 DispatchMsgService::enqueue(iEvent * ev)
{
	if (NULL == ev)
	{
		return -1;
	}
	ConnectSession* cs = (ConnectSession*)ev->get_args();
	LOG_DEBUG("���ͻ���[%s][%p]����Ͷ�ݵ����������.......", cs->remote_ip, cs->bev);
	thread_task_t* task = thread_task_alloc(0);
	task->handler = DispatchMsgService::svc;
	task->ctx = ev;

	return thread_task_post(tp, task);
}

void DispatchMsgService::svc(void * argv)
{
	DispatchMsgService* dms = DispatchMsgService::getInstance();
	
	iEvent* ev = (iEvent*)argv;
	if (!dms->svr_exit_)
	{
		LOG_DEBUG("DispatchMsgService::svc....\n");
		printf("DispatchMsgService::svc....\n");
		printf("!ev->eid=%d\n", ev->get_eid());

		iEvent* rsp = dms->process(ev);
		if (rsp)
		{
			rsp->dump(std::cout);
			rsp->set_args(ev->get_args());
			
		}
		else
		{
			//������ֹ��Ӧ�¼�
			rsp = new ExitRspEv();
			rsp->set_args(ev->get_args());
		}
		//
		ConnectSession* cs = (ConnectSession*)rsp->get_args();
		LOG_DEBUG("���ڽ��¼�[%s][%p]������Ӧ�¼����е���.......", cs->remote_ip, cs->bev);
		thread_mutex_lock(&queue_mutext);
		int size = rsp->Bytesize();
		response_events.push(rsp); //����ʲô�¼�����ͳһ������Ӧ�¼����е���
		thread_mutex_unlock(&queue_mutext);
	}
}

iEvent* DispatchMsgService::process(const iEvent * ev)
{
	LOG_DEBUG("DispatchMsgService::process - ev: %p\n", ev);

	if (ev == NULL)
	{
		return nullptr;
	}
	u32 eid = ev->get_eid();
	LOG_DEBUG("DispatchMsgService::process-eid: %u\n", eid);
	printf("DispatchMsgService::process-eid: %u\n", eid);
	if (eid == EEVNETID_UNKOWN)
	{
		LOG_WARN("DispatchMsgService: unknow event id %d\n", eid);
		printf("DispatchMsgService: unknow event id %d\n", eid);
		return nullptr;
	}
	T_EventHandlersMap::iterator handlers = subscribers_.find(eid);
	if (handlers == subscribers_.end())
	{
		LOG_WARN("DispatchMsgService��no any event handler subscribed: %d\n", eid);
		printf("DispatchMsgService��no any event handler subscribed: %d\n", eid);
		return nullptr;
	}
	iEvent* rsp = NULL;
	for (auto iter = handlers->second.begin(); iter != handlers->second.end(); iter++)
	{
		iEventHandler *handler = *iter;
		LOG_DEBUG("DispatchMsgService��get handler: %s\n", handler->get_name().c_str());
		printf("DispatchMsgService: get handler: %s\n", handler->get_name().c_str());
		printf("!ev: eid=%d\n", ev->get_eid());
		
		rsp = handler->handle(ev);//����ʹ��vector��list���ض�� rsp		
	}	
	//����
	return rsp;
}

DispatchMsgService * DispatchMsgService::getInstance()
{
	if (DMS_ == nullptr)
	{
		DMS_ = new DispatchMsgService();
		
	}
	return DMS_;
}

void DispatchMsgService::setAddClientNumber()
{
	++clientNumber;
}

void DispatchMsgService::setDecClientNumber()
{
	--clientNumber;
}



int DispatchMsgService::getClientNumber()
{
	return clientNumber;
}

iEvent * DispatchMsgService::parseEvent(const char * message, u32 len, u16 eid)
{
	if (!message)
	{
		LOG_ERROR("DispatchMsgService::parseEvent - message is null[eid=%d]\n", eid);
		return nullptr;
	}
	switch (eid)
	{
		//��ȡ�ֻ���֤��
		case EEVENTID_GET_MOBILE_CODE_REQ:
		{
			tutorial::mobile_request mr;
			mr.set_mobile(message);
			LOG_DEBUG("�ͻ������ڳ��Ի�ȡ�ֻ���֤��......");
			LOG_DEBUG("���ڴӿͻ������л���massage�н�������.......");
			if (mr.ParseFromArray(message, len))//��������
			{
				LOG_DEBUG("�����������.�ͻ����ֻ���Ϊ��%s", mr.mobile().c_str());
				MobileCodeReqEv* ev = new MobileCodeReqEv(mr.mobile());
				return ev;
			}
			break;
		}
		//��½
		case EEVENTID_LOGIN_REQ:
		{
			LOG_DEBUG("�ͻ������ڳ��Ե�½......");
			LOG_DEBUG("���ڴӿͻ������л���massage�н�������.......");
			tutorial::login_request lr;
			//�������������
			lr.set_icode(eid);
			if (lr.ParseFromArray(message, len))
			{
				LOG_DEBUG("�����������.�ͻ����ֻ���Ϊ��%s�� ��֤��Ϊ��%d", lr.mobile().c_str(), lr.icode());
				LoginReqEv* ev = new LoginReqEv(lr.mobile(), lr.icode());
				return ev;
			}
			break;
		}
		//��ȡ�˻���Ϣ����
		case EEVENTID_LIST_ACCOUNT_RECORDS_REQ:
		{
			LOG_DEBUG("�ͻ��˽������У����ڻ�ȡ�˻���Ϣ......");
			LOG_DEBUG("���ڴӿͻ������л���massage�н�������.......");
			tutorial::list_account_records_request recordRequest;
			if (recordRequest.ParseFromArray(message, len))
			{
				
				RecordsRequestEv* ev = new RecordsRequestEv(recordRequest.mobile());
				return ev;
			}
			break;
		}
		//��ȡ�˻���Ϣ��Ӧ
		case EEVENTID_ACCOUNT_RECORDS_RSP:
		{
			LOG_DEBUG("�ͻ��˽������У��˻���Ϣ�ѷ���......");
			LOG_DEBUG("���ڴӿͻ������л���massage�н�������.......");
			tutorial::list_account_records_response larr;
			if (larr.ParseFromArray(message, len))
			{
				const tutorial::list_account_records_response_account_record& ar = larr.records(0);
				
				RecordsResponseEv* ev = new RecordsResponseEv(ar.);
				return ev;
			}
			break;
		}

	default:
		break;
	}

	return nullptr;
}

void DispatchMsgService::handleAllResponseEvent(NetworkInterface* interface)
{
	//printf("into DispatchMsgService::handleAllResponseEvent -- \n");
	//LOG_DEBUG("into DispatchMsgService::handleAllResponseEvent -- \n");
	bool done = false;
	while (!done)
	{
		iEvent* ev = nullptr;
		thread_mutex_lock(&queue_mutext);
		if (!response_events.empty())
		{
			printf("!response_events.empty()\n");
			ev = response_events.front();//�ó���Ӧ�ĵ�һ���¼�
			
			int i = ev->Bytesize();
			response_events.pop();
		}
		else
		{
			done = true;
			//printf("done=true!\n");
		}
		thread_mutex_unlock(&queue_mutext);

		if (!done)
		{
			u32 EventId = ev->get_eid();
			printf("get_eid()......\nEventId=%d\n", EventId);
			if (EventId == EEVENTID_GET_MOBILE_CODE_RSP)//��ȡ�ֻ���֤����Ӧ
			{
				LOG_DEBUG("���ڻ�ȡ�ֻ���֤����Ӧ--DispatchMsgService::handleAllResponseEvent - id:EEVENTID_GET_MOBILE_CODE_RSP\n");
				printf("DispatchMsgService::handleAllResponseEvent - id:EEVENTID_GET_MOBILE_CODE_RSP\n");

				//������Ӧ��Ϣ
				sendPesponseMessage(ev, EEVENTID_GET_MOBILE_CODE_RSP, interface);

			}
			
			else if (EventId == EEVENTID_LOGIN_RSP) //��½��Ӧ
			{
				LOG_DEBUG("���ڻ�ȡ�ֻ���½��Ӧ--DispatchMsgService::handleAllResponseEvent - id:EEVENTID_LOGIN_RSP\n");
				sendPesponseMessage(ev, EEVENTID_LOGIN_RSP, interface);
			}
			else if (EventId == EEVNETID_EXIT_RSP) //�˳���Ӧ
			{
				LOG_DEBUG("�˳���Ӧ--ev->get_eid() == EEVNETID_EXIT_RSP\n");
				ConnectSession* cs = (ConnectSession*)ev->get_args();
				cs->response = ev;
				interface->send_response_message(cs);
			}
		}
	}
}

void DispatchMsgService::sendPesponseMessage(iEvent* ev, EventID Eid, NetworkInterface* interface)
{
	ConnectSession* cs = (ConnectSession*)ev->get_args();
	cs->response = ev;
	
	//ϵ�л���������
	cs->message_len = ev->Bytesize();
	cs->write_buf = new char[cs->message_len + MESSAGE_HEADER_LEN];

	//��װͷ��
	memcpy(cs->write_buf, MESSAGE_HEADER_ID, 4);
	*(u16*)(cs->write_buf + 4) = Eid;
	*(i32*)(cs->write_buf + 6) = cs->message_len;
	LOG_DEBUG("�������л�����......");
	ev->SerializeToArray(cs->write_buf + MESSAGE_HEADER_LEN, cs->message_len);
	
	//printf("!!!!!!!!!!!!sendPesponseMessage--cs->write_buf:%s\n", cs->write_buf);
	interface->send_response_message(cs);
}


