#include "NetworkInterface.h"
#include "DispatchMsgService.h"
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



//�мǣ�ConnectSession ������C���͵ĳ�Ա����
static ConnectSession* session_init(int fd, struct bufferevent* bev)
{
	ConnectSession* temp = nullptr; //bufferevent
	temp = new ConnectSession();

	if (!temp)
	{
		fprintf(stderr, "malloc failed. reason: %m\n");
		return nullptr;
	}
	memset(temp, '\0', sizeof(ConnectSession));
	temp->bev = bev;//bufferevent
	temp->fd = fd;
	return temp;
}

void session_free(ConnectSession *cs)
{
	if (cs)
	{		
		if (cs->read_buf)
		{
			delete[] cs->read_buf;
			cs->read_buf = nullptr;
		}
	
		if (cs->write_buf)
		{
			delete[] cs->write_buf;
			cs->write_buf = nullptr;
		}
		delete cs;
		cs = nullptr;
	}
}

NetworkInterface::NetworkInterface()
{
	base_ = nullptr;
	listener_ = nullptr;
}

NetworkInterface::~NetworkInterface()
{
	close();
}

bool NetworkInterface::start(int port)
{
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);

	base_ = event_base_new();

	listener_ = evconnlistener_new_bind(base_, NetworkInterface::listener_cb, base_,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
		512, (struct sockaddr*)&sin,
		sizeof(struct sockaddr_in));
}

void NetworkInterface::close()
{
	if (base_)
	{
		event_base_free(base_);
		base_ = nullptr;
	}
	if (listener_)
	{
		evconnlistener_free(listener_);
		listener_ = nullptr;
	}
}

void NetworkInterface::session_reset(ConnectSession * cs)
{
	if (cs)
	{
		LOG_DEBUG("�����ͷ���Դ......");
		if (cs->read_buf)
		{
			delete cs->read_buf;
			cs->read_buf = nullptr;
		}
		if (cs->write_buf)
		{
			delete cs->write_buf;
			cs->write_buf = nullptr;
		}

		cs->session_stat = SESSION_STATUS::SS_REQUEST;
		cs->req_stat = MESSAGE_STATUS::MS_READ_HEADER;

		cs->message_len = 0;
		cs->read_message_len = 0;
		cs->read_header_len = 0;
		LOG_DEBUG("�ͷ���Դ��ɣ�");
	}
}

void NetworkInterface::listener_cb(evconnlistener * listener, evutil_socket_t fd, sockaddr * sock, int socklen, void * arg)
{
	struct event_base* base = (struct event_base*)arg;
	
	//Ϊ����ͻ��˷���һ��bufferevent
	struct bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	
	ConnectSession* cs = session_init(fd, bev);
	
	cs->session_stat = SESSION_STATUS::SS_REQUEST;//
	cs->req_stat = MESSAGE_STATUS::MS_READ_HEADER;
	//printf("!!listener_cb cs->bev=%p\n\n", &cs->bev);
	strcpy(cs->remote_ip, inet_ntoa(((sockaddr_in*)sock)->sin_addr));
	LOG_DEBUG("accept new client remote ip :%s[%p]\n", cs->remote_ip, cs->bev);
	DispatchMsgService::setAddClientNumber();
	LOG_DEBUG("current client number:%d\n", DispatchMsgService::getClientNumber());
	
	bufferevent_setcb(bev, handle_request, handle_response, handle_error, cs);
	bufferevent_enable(bev, EV_READ | EV_PERSIST);
	//���ó�ʱֵ��10��û��д���߶�ʱ������ʱʱ�����ó����� handle_error
	bufferevent_settimeout(bev, 1000, 1000);//��ʱֵӦ�����������ļ���
}

void NetworkInterface::handle_request(bufferevent * bev, void * arg)
{
	ConnectSession *cs = (ConnectSession*)(arg);
	if (cs->session_stat != SESSION_STATUS::SS_REQUEST)
	{
		LOG_WARN("NetworkInterface::handle_request wrong session stat[%d]\n", cs->session_stat);
		return;
	}
	/*cs->read_buf = new char[100];
	cs->read_header_len = bufferevent_read(bev, cs->read_buf, 100);
	cs->read_buf[cs->read_header_len] = '\0';
	//LOG_DEBUG("recv from client cs->header[%s], lenth[%d]", cs->header, cs->read_header_len);
	bufferevent_enable(bev, EV_WRITE);*/
	
	if (cs->req_stat == MESSAGE_STATUS::MS_READ_HEADER)//��ȡͷ��
	{
		//�ҵ����ʣ�3.13    read_header_len�ĳ�ʼֵ�Ƕ��٣�����
		i32 len = bufferevent_read(bev, cs->header + cs->read_header_len, MESSAGE_HEADER_LEN - cs->read_header_len);
		cs->read_header_len += len;
		cs->header[cs->read_header_len] = '\0';
		LOG_DEBUG("recv from client cs->header[%s], lenth[%d]", cs->header, cs->read_header_len);
		printf("recv from client cs->header<<<< %s \n", cs->header);
		//printf("read_header_len:%d\n", cs->read_header_len);

		if (cs->read_header_len == MESSAGE_HEADER_LEN)//��ȡͷ������
		{
			if (strncmp(cs->header, MESSAGE_HEADER_ID, strlen(MESSAGE_HEADER_ID)) == 0)
			{
				//FBEB	   �¼�ID	   ���ݳ���N	 ��������
				//4�ֽ�    2���ֽ�      4���ֽ�
				cs->eid = *((u16 *)(cs->header + 4));		 //��ȡ�¼�ID
				cs->message_len = *((u32 *)(cs->header + 6));//��ȡ���ݳ���
				//LOG_DEBUG("NetworkInterface::handle_request erron: message_len:%d\n", cs->message_len);
				if (cs->message_len < 1 || cs->message_len > MAX_MESSAGE_LEN)
				{
					LOG_ERROR("NetworkInterface::handle_request erron: message_len:%d\n", cs->message_len);
					printf("NetworkInterface::handle_request erron: message_len:%d\n", cs->message_len);
					bufferevent_free(bev);
					session_free(cs);
					return;
				}
				cs->req_stat = MESSAGE_STATUS::MS_READ_MESSAGE;
				cs->read_buf = new char[cs->message_len];
				//cs->read_buf = NULL;
				cs->read_message_len = 0;
			}
			else
			{
				LOG_ERROR("NetworkInterface::handle_request -Invalid request from:%s\n", cs->remote_ip);
				printf("NetworkInterface::handle_request -Invalid request from:%s\n", cs->remote_ip);
				//ֱ�ӹر����󣬲������κ���Ӧ����ֹ�ͻ��˶�����̽
				bufferevent_free(bev);//
				session_free(cs);
				return;
			}
			
		}
		
	}
	if (cs->req_stat == MESSAGE_STATUS::MS_READ_MESSAGE && evbuffer_get_length(bufferevent_get_input(bev)) > 0)
	{
		i32 len = bufferevent_read(bev, cs->read_buf + cs->read_message_len, cs->message_len - cs->read_message_len);
		cs->read_message_len += len;//
		LOG_DEBUG("��ȡ��Ϣ��NetworkInterface::handle_request: bufferevnet_read: %d byte\n, read_message_len: %d, message_len:%d\n", len, cs->read_message_len, cs->message_len);
		//printf("NetworkInterface::handle_request: bufferevnet_read: %d byte, read_message_len: %d, message_len:%d\n", len, cs->read_message_len, cs->message_len);

		if (cs->read_message_len == cs->message_len)
		{
			//printf("!!!cs->read_buf: %s\n", cs->read_buf);
			cs->session_stat = SESSION_STATUS::SS_RESPONSE;
			iEvent *ev = DispatchMsgService::getInstance()->parseEvent(cs->read_buf, cs->read_message_len, cs->eid);
			//�ͷ�cs->read_buf�Ŀռ�
			delete[] cs->read_buf;
			cs->read_buf = nullptr;
			cs->read_message_len = 0;

			if (ev)
			{			
				ev->set_args(ev);				
				DispatchMsgService::getInstance()->enqueue(ev);
			}
			//ev->set_args(cs);
			else
			{
				LOG_ERROR("NetworkInterface::handle_request ev is null, remote ip:%s, eid=%d\n", cs->remote_ip, cs->eid);
				printf("NetworkInterface::handle_request ev is null, remote ip:%s, eid=%d\n", cs->remote_ip, cs->eid);
				//ֱ�ӹر����󣬲������κ���Ӧ����ֹ�ͻ��˶�����̽
				bufferevent_free(bev);
				session_free(cs);
				return;
			}
		}
		
	}
		
}

void NetworkInterface::handle_response(bufferevent * bev, void * arg)
{
	LOG_DEBUG("NetworkInterface::handle_response������");
	
	//ConnectSession* cs = (ConnectSession*)(arg);
	//bufferevent_write(bev, cs->read_buf, 100);
	//LOG_DEBUG("cs->read_buf:%s\n", cs->read_buf);
}

//��ʱ�����ӹرգ���д������쳣���ָ�����õĳ���ص�����
void NetworkInterface::handle_error(bufferevent * bev, short event, void * arg)
{
	LOG_DEBUG("NetworkInterface::handle_error������\n");

	ConnectSession *cs = (ConnectSession *)(arg);
	if (event & BEV_EVENT_EOF)
	{
		LOG_DEBUG("client[%s][%p] connection closed.\n", cs->remote_ip, cs->bev);
		DispatchMsgService::setDecClientNumber();
		LOG_DEBUG("current client number��%d", DispatchMsgService::getClientNumber());
	}
	else if ((event & BEV_EVENT_TIMEOUT) && (event & BEV_EVENT_READING))
	{
		LOG_WARN("client[%s][%p] reading timeout", cs->remote_ip, cs->bev);
		DispatchMsgService::setDecClientNumber();
		LOG_DEBUG("current client number��%d", DispatchMsgService::getClientNumber());
		
	}
	else if ((event & BEV_EVENT_TIMEOUT) && (event & BEV_EVENT_WRITING))
	{
		LOG_WARN("NetworkInterface::writting timeout ...\
				 client ip: %s\n", cs->remote_ip);
	}
	else if (event & BEV_EVENT_ERROR)
	{
		LOG_WARN("NetworkInterface::other some error ...\
				 client ip: %s\n", cs->remote_ip);
	}

	//�رոÿͻ���bev, ��cs��д������
	bufferevent_free(bev);
	session_free(cs);
}

void NetworkInterface::network_event_dispatch()
{
	event_base_loop(base_, EVLOOP_NONBLOCK);
	//������Ӧ�¼����ظ���Ӧ��Ϣ --- δ�����
	//printf("event_base_loop(base_, EVLOOP_NONBLOCK); before---\n");
	DispatchMsgService::getInstance()->handleAllResponseEvent(this);
}

void NetworkInterface::send_response_message(ConnectSession * cs)
{
	if (cs->response == nullptr)
	{
		bufferevent_free(cs->bev);
		if (cs->request)
		{
			delete cs->request;
			cs->request = nullptr;
		}
		session_free(cs);
	}
	else
	{
		//���������� 2021-9-25 16:03 2021-9-27����ˣ�bev���������⣬
		
		if (cs->bev)
		{
			LOG_DEBUG("���л����ݳɹ�!");
			LOG_DEBUG("���ڻ�Ӧ�ͻ���cs->bev[%p],message[%s]......", cs->bev, cs->write_buf);			
			
			bufferevent_write(cs->bev, cs->write_buf, cs->message_len + MESSAGE_HEADER_LEN);
			LOG_DEBUG("��Ӧ�ͻ��˳ɹ���");
		}
		
		else
			LOG_ERROR("cs->bev is empty! cann't send to client\n, NetworkInterface.cpp, line[304]");
		if(cs)	
			session_reset(cs);
	}
}
