#include "sqlconnection.h"


MysqlConnection::MysqlConnection(): mysql_(NULL)
{
	mysql_ = (MYSQL*)malloc(sizeof(MYSQL));
}

MysqlConnection::~MysqlConnection()
{
	if (mysql_)
	{
		mysql_close(mysql_);
		free(mysql_);
		mysql_ = NULL;
	}
	return;
}

bool MysqlConnection::Init(const char * szHost, int nPort, const char * szUser, const char * szPasswd, const char * szDb)
{
	mysql_ = mysql_init(mysql_);
	if (mysql_ == NULL)
	{
		LOG_ERROR("init mysql failed %s, %d", this->GetErrInfo(), errno);
		return false;
	}
	char cAuto = 1;
	if (mysql_options(mysql_, mysql_option::MYSQL_OPT_RECONNECT, &cAuto) != 0)
	{
		LOG_ERROR("mysql_options MYSQL_OPT_RECONNECT failed ");
	}

	if (mysql_real_connect(mysql_, szHost, szUser, szPasswd, szDb, nPort, NULL, 0) == NULL)
	{
		LOG_ERROR("connect mysql failed %s, %d", this->GetErrInfo(), errno);
		return false;
	}
	
	return true;
}

bool MysqlConnection::Execute(const char * szSql)
{
	//如果查询成功，返回0。如果出现错误，返回非0值。
	
	int len = strlen(szSql);

	/*if (mysql_real_query(mysql_, szSql, strlen(szSql) != 0))
	{
		if (mysql_errno(mysql_) == CR_SERVER_GONE_ERROR)
		{
			Reconnect();
		}
		return false;
	}*/
	
	if (mysql_real_query(mysql_, szSql, strlen(szSql)) != 0)//
	{
		if (mysql_errno(mysql_) == CR_SERVER_GONE_ERROR)
		{			
			Reconnect();
		}
		return false;
	}
	return true;
}

bool MysqlConnection::Execute(const char * szSql, SqlRecordSet& recordSet)
{
	if (mysql_real_query(mysql_, szSql, strlen(szSql)) != 0)//
	{
		if (mysql_errno(mysql_) == CR_SERVER_GONE_ERROR)
		{

			Reconnect();
		}
		LOG_ERROR("sql语句执行失败！\n");
		return false;
	}

	MYSQL_RES* p_Res = mysql_store_result(mysql_);
	if (!p_Res)
	{
		return false;
	}

	recordSet.SetResult(p_Res);
	
	return true;
}

int MysqlConnection::EscapeString(const char * pSrc, int nSrclen, char * pDest)
{
	if (!mysql_)
	{
		return 0;
	}
	mysql_real_escape_string(mysql_, pDest, pSrc,  nSrclen);
}

void MysqlConnection::close()
{
}

const char * MysqlConnection::GetErrInfo()
{
	return mysql_error(mysql_);
}

void MysqlConnection::Reconnect()
{
	mysql_ping(mysql_);
}