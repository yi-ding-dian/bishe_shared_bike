#include "user_service.h"

UserService::UserService(std::shared_ptr<MysqlConnection> sql_conn):sql_conn_(sql_conn)
{


}

bool UserService::exist(std::string & mobile)
{
	char sql_content[1024] = { 0 };

	sprintf(sql_content, "select * from userinfo where mobile = %s", \
		mobile.c_str());

	SqlRecordSet record_set;
	if (!sql_conn_->Execute(sql_content, record_set))
	{
		return false;
	}

	return (record_set.GetRowCount() != 0);
	//return true;
}

bool UserService::insert(std::string & mobile)
{
	char sql_content[1024] = { 0 };
	sprintf(sql_content, "insert into userinfo(username, mobile)  \
							values(\"����\", \"%s\")", mobile.c_str());
	SqlRecordSet record_set;
	LOG_DEBUG("executing sql insert into userinfo(username, mobile)....");
	LOG_DEBUG("sql: [%s]", sql_content);
	//if (!sql_conn_->Execute(sql_content, record_set))
	if (!sql_conn_->Execute(sql_content))
	{
		LOG_ERROR("ִ�����ݿ�������ʧ�ܣ�\n");
		return false;
	}
	LOG_DEBUG("ִ��sql: [%s]�ɹ���\n", sql_content);

	//return (record_set.GetRowCount() != 0);
	return true;
}
