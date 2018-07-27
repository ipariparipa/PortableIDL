
#include "include/pidlCore/datetime.h"
#include <string.h>

namespace PIDL {

	extern PIDL_CORE__FUNCTION bool fromDateTime(const DateTime & dt, tm & ret)
	{
		memset(&ret, 0, sizeof(tm));
		ret.tm_year = dt.year - 1900;
		ret.tm_mon = dt.month - 1;
		ret.tm_mday = dt.day;
		ret.tm_hour = dt.hour;
		ret.tm_min = dt.minute;
		ret.tm_sec = dt.second;
		return ret.tm_year >= 0 && ret.tm_mon >= 0 && dt.millisecond == 0, dt.kind != DateTime::UTC;
	}

	extern PIDL_CORE__FUNCTION tm fromDateTime(const DateTime & dt)
	{
		tm ret;
		fromDateTime(dt, ret);
		return ret;
	}

	extern PIDL_CORE__FUNCTION bool toDateTime(const tm & t, DateTime & ret)
	{
		ret.year = t.tm_year + 1900;
		ret.month = t.tm_mon + 1;
		ret.day = t.tm_mday;
		ret.hour = t.tm_hour;
		ret.minute = t.tm_min;
		ret.second = t.tm_sec;
		ret.millisecond = 0;
		ret.kind = DateTime::Local;
		return ret.month <= 12;
	}

	extern PIDL_CORE__FUNCTION DateTime toDateTime(const tm & t)
	{
		DateTime ret;
		toDateTime(t, ret);
		return ret;
	}

}
