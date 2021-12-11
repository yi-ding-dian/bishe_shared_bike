#include <iostream>

#include "bike.pb.h"
#include "ievent.h"
#include "events_def.h"
#include "user_event_handler.h"
#include "DispatchMsgService.h"
#include "NetworkInterface.h"
#include <unistd.h>
#include "iniconfig.h"
#include "Logger.h"
#include "sqlconnection.h"
#include "SqlTables.h"
#include "BusProcessor.h"

using namespace std;

int main(int argc, char **argv)
{
	/*
	//printf("argv[1]:%s\n argv[2]:%s\n", argv[1], argv[2]);
	if (argc != 3) {
		printf("Please input shbk <config file path>!\n");
		return -1;
	}
	//printf("please input shbk !");
	//如果初始化日记失败，不成功
	
	
	if (!Logger::instance()->init(std::string(argv[2])))
	{
		fprintf(stderr, "init log module failed.\n");
		return -2;
	}

	Iniconfig* config = Iniconfig::getInstance();
	if (!config->loadfile(std::string(argv[1])))
	{
		printf("load %s failed.\n", argv[1]);
		LOG_ERROR("load %s failed.", argv[1]);
		Logger::instance()->GetHandle()->error("load %s failed.", argv[1]);

		return -3;
	}*/
	if (!Logger::instance()->init(std::string("/home/yidiandian/projects/conf/log.conf")))
	{
		fprintf(stderr, "init log module failed.\n");
		return -2;
	}

	Iniconfig* config = Iniconfig::getInstance();
	if (!config->loadfile(std::string("/home/yidiandian/projects/conf/shared_bike.ini")))
	{
		printf("load %s failed.\n", argv[1]);
		LOG_ERROR("load %s failed.", argv[1]);
		Logger::instance()->GetHandle()->error("load %s failed.", argv[1]);

		return -3;
	}
	
	st_env_config conf_args = config->getconfig();
	LOG_INFO("[data ip]:%s, port: %d, user:%s, pwd:%s, db:%s [server] port:%d\n",
		conf_args.db_ip.c_str(), conf_args.db_port, conf_args.db_user.c_str(), 
		conf_args.db_pwd.c_str(), conf_args.db_name.c_str(), conf_args.svr_port);
	//MysqlConnection con;
	
	
	std::shared_ptr<MysqlConnection> mysqlconn(new MysqlConnection);

	if (!mysqlconn->Init(conf_args.db_ip.c_str(), conf_args.db_port, conf_args.db_user.c_str(),
		conf_args.db_pwd.c_str(), conf_args.db_name.c_str()))
	{
		LOG_ERROR("Database init failed. exit!\n");
		return -4;
	}

	//连接数据库
	BusinessProcessor busPro(mysqlconn);
	busPro.init();
	
	DispatchMsgService* DMS = DispatchMsgService::getInstance();
	DMS->open();
	
	NetworkInterface *NTIF = new NetworkInterface();
	NTIF->start(conf_args.svr_port);
	LOG_DEBUG("等待客户端的连接.......\n\n");
	printf("等待客户端的连接.......\n\n");
	while (1)
	{
		NTIF->network_event_dispatch();
		//sleep(10);
		//LOG_DEBUG("network_event_dispatch.......\n\n");
	}
	
	return 0;
}