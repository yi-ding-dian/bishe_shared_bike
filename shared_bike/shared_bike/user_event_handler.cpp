#include "user_event_handler.h"
#include "DispatchMsgService.h"
#include "sqlconnection.h"
#include "iniconfig.h"
#include "user_service.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>

UserEventHandler::UserEventHandler() :iEventHandler("UserEventHandler")
{
	//未来需要订阅事件的处理
	DispatchMsgService::getInstance()->subscribe(EEVENTID_GET_MOBILE_CODE_REQ, this);
	DispatchMsgService::getInstance()->subscribe(EEVENTID_LOGIN_REQ, this);
	thread_mutex_create(&pm_);
}

UserEventHandler::~UserEventHandler()
{
	//未来需要实现退定事件的处理
	DispatchMsgService::getInstance()->unsubscribe(EEVENTID_GET_MOBILE_CODE_REQ, this);
	DispatchMsgService::getInstance()->unsubscribe(EEVENTID_LOGIN_REQ, this);
	thread_mutex_destroy(&pm_);
}

iEvent * UserEventHandler::handle(const iEvent * ev)
{
	if (ev == NULL)
	{
		printf("input ev is NULL");
	}

	u32 eid = ev->get_eid();

	if (eid == EEVENTID_GET_MOBILE_CODE_REQ)
	{
		return handle_mobile_code_req((MobileCodeReqEv*)ev);
	}
	else if (eid == EEVENTID_LOGIN_REQ)
	{
		iEvent* reqevent = handle_login_req((LoginReqEv*)ev);
		return reqevent;
	}
	else if (eid == EEVENTID_RECHARGE_REQ)
	{
		//return handle_recharge_req((RechargeEv*)ev);
	}
	else if (eid == EEVENTID_GET_ACCOUNT_BALANCE_REQ)
	{
		//return handle_get_account_balance_req((GetAccountBalanceEv*)ev));
	}
	else if (eid == EEVENTID_LIST_ACCOUNT_RECORDS_REQ)
	{
		//return handle_list_account_records_req((ListAccountRecordsReqEv*)ev);
	}

	return NULL;
}
//	handle_mobile_code_req((MobileCodeReqEv*)ev);
MobileCodeRspEv * UserEventHandler::handle_mobile_code_req(MobileCodeReqEv * ev)
{
	i32 icode = 0;
	std::string mobile_ = ev->get_mobile();
	LOG_DEBUG("try to get moblie phone %s validate code .", mobile_.c_str());
	LOG_DEBUG("正在为用户[%s]生成随机验证码......", mobile_.c_str());

	//printf("try to get moblie phone %s validate code .\n", mobile_.c_str());
	icode = code_gen();
	thread_mutex_lock(&pm_);
	auto iter = m2c_.find(mobile_);
	
	/*
	if (iter != m2c_.end())
	{
		LOG_WARN("用户手机号[%s]和验证码[%d]匹配失败！\n", str_mobile, icode);
		return new LoginResEv(ERRC_INVALID_DATA);
	}*/
	m2c_[mobile_] = icode;
	thread_mutex_unlock(&pm_);
	//printf("mobile:%s, code:%d\n", mobile_.c_str(), icode);
	LOG_DEBUG("用户[%s]验证码[%d]已生成！", mobile_.c_str(), icode);

	return new MobileCodeRspEv(200, icode);
}

i32 UserEventHandler::code_gen()
{
	i32 code = 0;
	srand((unsigned int)time(NULL));

	code = (unsigned int)(rand() % (999999 - 100000) + 100000);
	LOG_DEBUG("随机验证码已生成！");
	return code;
}

LoginResEv * UserEventHandler::handle_login_req(LoginReqEv * ev)
{
	LoginResEv *loginEv = nullptr;
	std::string mobile = ev->get_mobile();//取出手机号码
	const char* str_mobile = mobile.c_str();
	i32 icode = ev->get_icode();//拿出验证码
	LOG_DEBUG("正在匹配用户手机[%s]验证码[%d]......", str_mobile, icode);
	thread_mutex_lock(&pm_);

	auto iter = m2c_.find(mobile);
	if (iter == m2c_.end())
	{
		return NULL;
	}

	if (((iter != m2c_.end()) && (icode != iter->second)) || (iter == m2c_.end()))
	{
		LOG_WARN("用户手机号[%s]和验证码[%d]匹配失败！\n", str_mobile, icode);
		thread_mutex_unlock(&pm_);
		return new LoginResEv(ERRC_INVALID_DATA);
	}
	thread_mutex_unlock(&pm_);
	LOG_DEBUG("用户手机[%s]验证码[%d]匹配成功！", str_mobile, icode);
	/*
	if (loginEv)
	{
		LOG_DEBUG("识别到用户[%s]当前已经登陆！驳回再次登陆的请求!\n", str_mobile);
		return loginEv;
	}*/
	std::shared_ptr<MysqlConnection> mysqlconn(new MysqlConnection);
	_st_env_config conf_args = Iniconfig::getInstance()->getconfig();
	LOG_DEBUG("正在打开数据库.....");
	if (!mysqlconn->Init(conf_args.db_ip.c_str(), conf_args.db_port, conf_args.db_user.c_str(),
		conf_args.db_pwd.c_str(), conf_args.db_name.c_str()))
	{
		LOG_ERROR("UserEventHandler::handle_login_req - Database init failed. exit!\n");
		return new LoginResEv(ERRO_PROCCESS_FALED);
	}
	LOG_DEBUG("打开数据库成功！");
	UserService us(mysqlconn);
	bool result = false;
	if (!us.exist(mobile))
	{
		LOG_DEBUG("该用户为共享单车新用户，正在为该手机号[%s]初始化账户......", str_mobile);
		//std::cout << "us.insert(mobile)=" << mobile << std::endl;
		result = us.insert(mobile);
		if (!result)
		{
			LOG_DEBUG("新用户[%s]信息初始化失败！", mobile.c_str());
			return new LoginResEv(ERRO_PROCCESS_FALED);
		}
		LOG_DEBUG("新用户[%s]信息初始化完成，信息已录入数据库.", str_mobile);
	}
	
	LOG_DEBUG("用户[%s]登陆成功!\n", str_mobile);
	
	//iEvent* event = new LoginResEv(ERRC_SUCCESS);
	
	loginEv = new LoginResEv(ERRC_SUCCESS);
	/*LOG_DEBUG("loginEv->ByteSize():%d", loginEv->Bytesize());
	LOG_DEBUG("loginEv->get_code():%d", loginEv->get_code());
	LOG_DEBUG("loginEv->get_desc():%s", loginEv->get_desc().c_str());
	iEvent* resEv = dynamic_cast<LoginResEv*>(loginEv);
	
	LOG_DEBUG("loginEv->ByteSize():%d", loginEv->Bytesize());
	LOG_DEBUG("loginEv->get_code():%d", loginEv->get_code());
	LOG_DEBUG("loginEv->get_desc():%s", loginEv->get_desc().c_str());
	*/
	return loginEv;
}