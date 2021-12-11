#ifndef BRKS_COMMON_EVENTS_DEF_H_
#define BRKS_COMMON_EVENTS_DEF_H_

#include "ievent.h"
#include <string>
#include "bike.pb.h"


class MobileCodeReqEv :public iEvent
{
public:
	MobileCodeReqEv(const std::string& mobile) :iEvent(EEVENTID_GET_MOBILE_CODE_REQ, iEvent::generateSeqNo())
	{
		msg_.set_mobile(mobile);
	};

	const std::string& get_mobile() { return msg_.mobile(); };
	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len) { return msg_.SerializeToArray(buf, len); };


private:
	tutorial::mobile_request msg_;
};

class MobileCodeRspEv : public iEvent
{
public:
	MobileCodeRspEv(i32 code, i32 icode) :
		iEvent(EEVENTID_GET_MOBILE_CODE_RSP, iEvent::generateSeqNo())
	{
		msg_.set_code(code);  //����
		msg_.set_icode(icode);//��֤��
		msg_.set_data(getReasonByErrorCode(code));
	}

	const i32 get_code() { return msg_.code(); };
	const i32 get_icode() { return msg_.icode(); };
	const std::string& get_data() { return msg_.data(); };

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len) { return msg_.SerializeToArray(buf, len); };

private:
	tutorial::mobile_response msg_;
};

class ExitRspEv: public iEvent
{
public:
	ExitRspEv() :iEvent(EEVNETID_EXIT_RSP, iEvent::generateSeqNo()) {};
	//~ExitRspEv();

private:

};
//��½����
class LoginReqEv: public iEvent
{
public:
	LoginReqEv(const std::string& mobile, i32 icode) :
		iEvent(EEVENTID_LOGIN_REQ, iEvent::generateSeqNo())
	{
		msg_.set_mobile(mobile);
		msg_.set_icode(icode);
	}
	
	const std::string& get_mobile() { return msg_.mobile(); };
	const i32 get_icode() { return msg_.icode(); };

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len) { return msg_.SerializeToArray(buf, len); };

private:
	tutorial::login_request msg_;
};
//��½��Ӧ
class LoginResEv : public iEvent
{
public:
	LoginResEv(i32 code) :
		iEvent(EEVENTID_LIST_ACCOUNT_RECORDS_REQ, iEvent::generateSeqNo())
	{
		msg_.set_code(code);
		msg_.set_desc(getReasonByErrorCode(code));
	}

	const i32 & get_code() { return msg_.code(); };
	const std::string get_desc() { return msg_.desc(); };


	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len) { return msg_.SerializeToArray(buf, len); };

private:
	tutorial::login_response msg_;
};

//list_account_records_request �˻���¼����
class RecordsRequestEv : public iEvent
{
public:
	RecordsRequestEv(std::string mobile) :
		iEvent(EEVENTID_LIST_ACCOUNT_RECORDS_REQ, iEvent::generateSeqNo())
	{
		msg_.set_mobile(mobile);		
	}
	const std::string get_mobile() { return msg_.mobile(); };

	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize();};
	virtual bool SerializeToArray(char* buf, int len) { return msg_.SerializeToArray(buf, len); };

private:
	tutorial::list_account_records_request msg_;
};

//EEVENTID_ACCOUNT_RECORDS_RSP �˻���¼��Ӧ
/*
message list_account_records_response
{
	required int32   code   = 1;    // ��Ӧ����
	optional string  desc   = 2;    // ��֤��
	message account_record
	{
		required int32  type      = 1; // 0 : ��������,  1 : ��ֵ, 2 : �˿�
		required int32  limit     = 2; // ���ѻ��߳�ֵ���
		required uint64 timestamp = 3; // ��¼����ʱ��ʱ���
	}

	repeated account_record records = 3;
}
*/
class RecordsResponseEv : public iEvent
{
public:
	RecordsResponseEv(i32 code) :
		iEvent(EEVENTID_ACCOUNT_RECORDS_RSP, iEvent::generateSeqNo())
	{
		tutorial::list_account_records_response_account_record* arRes = msg_.add_records();
		msg_.set_code(code);
		msg_.set_desc(getReasonByErrorCode(code));
		arRes->set_type(0);
		arRes->set_limit(1);
		arRes->set_timestamp();
	}

	const i32& get_code() { return msg_.code(); };
	const std::string get_desc() { return msg_.desc(); };


	virtual std::ostream& dump(std::ostream& out) const;
	virtual i32 Bytesize() { return msg_.ByteSize(); };
	virtual bool SerializeToArray(char* buf, int len) { return msg_.SerializeToArray(buf, len); };

private:
	tutorial::list_account_records_response msg_;
};


#endif // !BRKS_COMMON_EVENTS_DEF_H_

