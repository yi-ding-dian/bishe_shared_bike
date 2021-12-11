#ifndef __BUSPROCESSOR_H_
#define __BUSPROCESSOR_H_

#include <memory>
#include "user_event_handler.h"
#include "sqlconnection.h"
#include "SqlTables.h"

class BusinessProcessor
{
public:
	/*智能指针是指向动态分配(堆)对象指针，用于生存期控制，能够确保自动正确的销毁动态分配的对象，防止内存泄露。
		它的一种通用实现技术是使用引用计数。每次使用它，内部的引用计数加1，
		每次析构一次，内部引用计数减1，减为0时，删除所指向的堆内存
		shared_ptr基于”引用计数”模型实现，多个shared_ptr可指向同一个动态对象，并维护一个共享的引用计数器，
		记录了引用同一对象的shared_ptr实例的数量。当最后一个指向动态对象的shared_ptr销毁时，
		会自动销毁其所指对象(通过delete操作符
	*/
	BusinessProcessor(std::shared_ptr<MysqlConnection> sqlconn);
	virtual ~BusinessProcessor();

	bool init();

private:
	std::shared_ptr<MysqlConnection> mysqlconn_;
	//MysqlConnection* mysqlcon;
	std::shared_ptr<UserEventHandler> ueh_;
};


#endif // !__BUSPROCESSOR_H_
