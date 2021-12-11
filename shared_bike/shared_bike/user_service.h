#ifndef __USER_SERVICE_H_
#define __USER_SERVICE_H_

#include <memory>
#include "sqlconnection.h"

class UserService
{
public:
	UserService(std::shared_ptr<MysqlConnection> sql_conn);

	bool exist(std::string& mobile);
	bool insert(std::string& mobile);

private:
	std::shared_ptr<MysqlConnection> sql_conn_;

};

#endif // !__USER_SERVICE_H_




