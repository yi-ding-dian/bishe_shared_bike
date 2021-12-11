#ifndef _GLO_DEF_H
#define _GLO_DEG_H

#include "Logger.h"

#ifdef _cplusplus

extern "C" {
#endif /*__cplusplus */

#define MAX_MESSAGE_LEN 367280

//#define LOG_DEBUG printf
//#define LOG_WARN printf
//#define LOG_ERROR printf

#define TURE	1
#define FALSE	0

#define INVALID_U32	0xFFFF
	/*Global type defintions */
	typedef unsigned char	u8;
	typedef unsigned short  u16;
	typedef unsigned int	u32;
	typedef signed char		i8;
	typedef signed short	i16;
	typedef signed int		i32;
	typedef float			r32;
	typedef double			r64;
	typedef double			r64;
	typedef long double		r128;

	typedef unsigned char BOOL;

	typedef u32	TBoolean;
	typedef i32 TDevid;

	typedef unsigned long long u64;
	typedef signed long long i64;

#ifdef __cplusplus





#endif // !_GLO_DEF_H
#endif
