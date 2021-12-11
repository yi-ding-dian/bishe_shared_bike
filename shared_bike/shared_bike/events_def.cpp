#include "events_def.h"
#include <iostream>
#include <sstream>

std::ostream& MobileCodeReqEv::dump(std::ostream& out) const
{
	out << "MobileCodeReq sn" << get_sn() << ",";
	out << "mobile=" << msg_.mobile() << std::endl;
	return out;
}

std::ostream & MobileCodeRspEv::dump(std::ostream & out) const
{
	out << "MobileCodeRspEv sn" << get_sn() << ",";
	out << "code=" << msg_.code() << std::endl;
	out << "icode=" << msg_.icode() << std::endl;
	out << "describel=" << msg_.data() << std::endl;
	return out;
}

std::ostream & LoginReqEv::dump(std::ostream & out) const
{
	out << "LoginReqEv sn=" << get_sn() << ",";
	out << "mobile:" << msg_.mobile() << std::endl;
	out << "icode:" << msg_.icode() << std::endl;

}

std::ostream & LoginResEv::dump(std::ostream & out) const
{
	out << "LoginResEv sn=" << get_sn() << ",";
	out << "code:" << msg_.code() << std::endl;
	out << "describle:" << msg_.desc() << std::endl;
}

std::ostream& RecordsRequestEv::dump(std::ostream& out) const
{
	// TODO: 在此处插入 return 语句
}

std::ostream& RecordsResponseEv::dump(std::ostream& out) const
{
	// TODO: 在此处插入 return 语句
}
