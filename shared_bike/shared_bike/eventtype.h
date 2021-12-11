#ifndef BRKS_COMMON_ENENT_TYPE_H_
#define BRKS_COMMON_ENENT_TYPE_H_

#include "glo_def.h"

typedef struct EErrorReason_		
{
	i32 code;
	const char* reason;
}EErrorReason;

/*�¼�ID*/
enum EventID
{
	EEVENTID_GET_MOBILE_CODE_REQ		= 0x01,//��ȡ�ֻ���֤������
	EEVENTID_GET_MOBILE_CODE_RSP		= 0x02,//�ֻ���֤����Ӧ

	EEVENTID_LOGIN_REQ					= 0x03,//��½����
	EEVENTID_LOGIN_RSP					= 0x04,//��½��Ӧ

	EEVENTID_RECHARGE_REQ				= 0x05,//�������
	EEVENTID_RECHARGE_RSP				= 0x06,//�����Ӧ

	EEVENTID_GET_ACCOUNT_BALANCE_REQ	= 0x07,//��ȡ�˺��������
	EEVENTID_ACCOUNT_BALANCE_RSP		= 0x08,//�˺������Ӧ

	EEVENTID_LIST_ACCOUNT_RECORDS_REQ   = 0x09,//��ȡ�˻���¼����
	EEVENTID_ACCOUNT_RECORDS_RSP		= 0x10,//�˻���¼��Ӧ

	EEVENTID_LIST_TRAVELS_REQ			= 0x11,//��ȡ�㼣����
	EEVENTID_LIST_TRAVELS_RSP			= 0x12,//�㼣��Ӧ

	EEVNETID_EXIT_RSP = 0xFE,
	EEVNETID_UNKOWN = 0xFF
};

/*�¼���Ӧ�������*/
enum EErrorCode
{
	ERRC_SUCCESS = 200,
	ERRC_INVALID_MSG = 400,
	ERRC_INVALID_DATA = 404,
	ERRC_METHOD_NOT_ALLOWED = 405,
	ERRO_PROCCESS_FALED = 406,
	ERRO_BIKE_IS_TOOK = 407,
	ERRO_BIKE_IS_RUNNING = 408,
	ERRO_BIKE_IS_DAMAGED = 409,
	ERRO_NULL = 0

};

const char* getReasonByErrorCode(i32 code);

#endif // !BRKS_COMMON_ENENT_TYPE_H_
