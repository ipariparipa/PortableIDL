
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
		return ret.tm_year >= 0 && ret.tm_mon >= 0 && dt.millisecond == 0 && dt.kind != DateTime::UTC;
	}

    extern PIDL_CORE__FUNCTION bool fromDateTime(const DateTime & dt, std::chrono::system_clock::time_point & ret)
    {
        tm tmp;
        if(!fromDateTime(dt, tmp))
            return false;
        ret = std::chrono::system_clock::time_point(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds(mktime(&tmp))) + std::chrono::milliseconds(dt.millisecond));

        return true;
    }

	extern PIDL_CORE__FUNCTION tm fromDateTime(const DateTime & dt)
	{
		tm ret;
		fromDateTime(dt, ret);
		return ret;
	}

	extern PIDL_CORE__FUNCTION bool toDateTime(const tm & t, DateTime & ret)
	{
        ret.year = static_cast<short>(t.tm_year + 1900);
        ret.month = static_cast<short>(t.tm_mon + 1);
        ret.day = static_cast<short>(t.tm_mday);
        ret.hour = static_cast<short>(t.tm_hour);
        ret.minute = static_cast<short>(t.tm_min);
        ret.second = static_cast<short>(t.tm_sec);
		ret.millisecond = 0;
		ret.kind = DateTime::Local;
		return ret.month <= 12;
	}

    extern PIDL_CORE__FUNCTION bool toDateTime(std::chrono::system_clock::time_point t, DateTime & ret)
    {
        time_t tt = std::chrono::duration_cast<std::chrono::seconds>(t.time_since_epoch()).count();
        auto tmp = localtime(&tt);

        if(!toDateTime(*tmp, ret))
            return false;

        ret.millisecond = static_cast<short>(std::chrono::duration_cast<std::chrono::milliseconds>(t.time_since_epoch()).count() - tt * 1000);

        return true;
    }

	extern PIDL_CORE__FUNCTION DateTime toDateTime(const tm & t)
	{
		DateTime ret;
		toDateTime(t, ret);
		return ret;
	}

}
