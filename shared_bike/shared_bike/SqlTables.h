#ifndef __SQLTABLES_H_
#define __SQLTABLES_H_
#include "sqlconnection.h"
#include <memory>
#include "glo_def.h"

class SqlTables
{
public:
	SqlTables(std::shared_ptr<MysqlConnection> sqlconn) :sqlconn_(sqlconn)
	{

	}
	//~SqlTables();

	bool CreateUserInfo()
	{
		LOG_DEBUG("正在创建用户信息表......");
		const char* pUserInfoTable = "\
								CREATE TABLE IF NOT EXISTS userinfo( \
								id			int(16)			NOT NULL primary key auto_increment, \
								username	varchar(128)	NOT NULL default 'bikeuser', \
								mobile		varchar(16)		NOT NULL default '13000000000', \						
								registertm	timestamp		NOT NULL default CURRENT_TIMESTAMP, \
								money		int(4)			NOT NULL default 0, \
								INDEX		mobile_index(mobile) \
								)";
		if (!sqlconn_->Execute(pUserInfoTable))
		{
			LOG_ERROR("create table userinfo table failed. error msg:%s", sqlconn_->GetErrInfo());
			printf("create bikeinfo table  table failed. error msg:%s", sqlconn_->GetErrInfo());
			return false;
		}
		LOG_DEBUG("用户信息表创建成功！");
		return true;
	}

	bool CreateBikeTable()
	{
		LOG_DEBUG("正在创建单车信息表......");
		const char* pBikeInfoTable = " \
						CREATE TABLE IF NOT EXISTS bikeinfo( \
						id			int				NOT NULL primary key auto_increment, \
						devno		int				NOT NULL, \
						status		tinyint(1)		NOT NULL default 0, \
						trouble		int				NOT NULL default 0, \
						tmsg		varchar(256)	NOT NULL default '', \
						latitude	double(10,6)	NOT NULL default 0, \
						longitude	double(10,6)	NOT NULL default 0, \
						unique(devno) \
						)";
		

		//const char* pBikeInfoTable = "create table if not exists bikeinfo(id int, name char(6));";
		if (!sqlconn_->Execute(pBikeInfoTable))
		{
			LOG_ERROR("create bikeinfo table failed. error msg:%s", sqlconn_->GetErrInfo());
			printf("create bikeinfo table failed. error msg:%s", sqlconn_->GetErrInfo());
			return false;
		}
		LOG_DEBUG("单车信息表创建成功！");
		return true;

	}

private:
	std::shared_ptr<MysqlConnection> sqlconn_;
};


#endif // !__SQLTABLES_H_
