
#include "include/pidlCore/datetime.h"
#include <string.h>

namespace PIDL {
    namespace {
        time_t timeutc (const struct tm *tm)
        {
            auto is_leap = [](int y) {
                y += 1900;
                return (y % 4) == 0 && ((y % 100) != 0 || (y % 400) == 0);
            };

            static const unsigned ndays[2][12] = {
                {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
                {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
            };
            time_t res = 0;
            int i;

            for (i = 70; i < tm->tm_year; ++i)
                res += is_leap(i) ? 366 : 365;

            for (i = 0; i < tm->tm_mon; ++i)
                res += ndays[is_leap(tm->tm_year)][i];
            res += tm->tm_mday - 1;
            res *= 24;
            res += tm->tm_hour;
            res *= 60;
            res += tm->tm_min;
            res *= 60;
            res += tm->tm_sec;
            return res;
        }
    }

    extern PIDL_CORE__FUNCTION bool fromDateTime(const DateTime & dt, tm & ret)
	{
        std::chrono::system_clock::time_point tmp;
        if(!fromDateTime(dt, tmp))
            return false;

        time_t t = std::chrono::duration_cast<std::chrono::seconds>(tmp.time_since_epoch()).count();
        localtime_r(&t, &ret);
        if(ret.tm_mon < 0)
            return false;

        return true;
	}

    extern PIDL_CORE__FUNCTION bool fromDateTime(const DateTime & dt, std::chrono::system_clock::time_point & ret)
    {
        tm tmp;
        memset(&tmp, 0, sizeof(tm));

        tmp.tm_year = dt.year - 1900;
        tmp.tm_mon = dt.month - 1;
        tmp.tm_mday = dt.day;
        tmp.tm_hour = dt.hour;
        tmp.tm_min = dt.minute;
        tmp.tm_sec = dt.second;
        tmp.tm_isdst = -1;

        switch(dt.kind)
        {
        case DateTime::UTC:
            tmp.tm_zone = "UTC";
            ret = std::chrono::system_clock::time_point(std::chrono::seconds(timeutc(&tmp)));
            break;
        case DateTime::None:
        case DateTime::Local:
            ret = std::chrono::system_clock::time_point(std::chrono::seconds(mktime(&tmp)));
            break;
        }

        if(tmp.tm_mon < 0)
            return false;

        ret += std::chrono::nanoseconds(dt.nanosecond);

        return true;
    }

	extern PIDL_CORE__FUNCTION tm fromDateTime(const DateTime & dt)
	{
        if(tm ret; fromDateTime(dt, ret))
            return ret;

        throw Exception(-1, "value cannot be converted from DateTime to tm");
	}

    extern PIDL_CORE__FUNCTION tm toTm(const DateTime & dt)
    {
        if(tm ret; fromDateTime(dt, ret))
            return ret;

        throw Exception(-1, "value cannot be converted from DateTime to tm");
    }

    extern PIDL_CORE__FUNCTION std::chrono::system_clock::time_point toTimepoint(const DateTime & dt)
    {
        if(std::chrono::system_clock::time_point ret; fromDateTime(dt, ret))
            return ret;

        throw Exception(-1, "value cannot be converted from DateTime to time_point");
    }

	extern PIDL_CORE__FUNCTION bool toDateTime(const tm & t, DateTime & ret)
	{
        ret.year = static_cast<short>(t.tm_year + 1900);
        ret.month = static_cast<short>(t.tm_mon + 1);
        ret.day = static_cast<short>(t.tm_mday);
        ret.hour = static_cast<short>(t.tm_hour);
        ret.minute = static_cast<short>(t.tm_min);
        ret.second = static_cast<short>(t.tm_sec);
        ret.nanosecond = 0;
        if(t.tm_zone && strcmp(t.tm_zone, "UTC") == 0)
            ret.kind = DateTime::UTC;
        else
            ret.kind = DateTime::Local;
		return ret.month <= 12;
	}

    extern PIDL_CORE__FUNCTION bool toDateTime(std::chrono::system_clock::time_point t, DateTime & ret)
    {
        time_t tt = std::chrono::duration_cast<std::chrono::seconds>(t.time_since_epoch()).count();
        auto tmp = localtime(&tt);

        if(!toDateTime(*tmp, ret))
            return false;

        ret.nanosecond = static_cast<int>(std::chrono::duration_cast<std::chrono::nanoseconds>(t.time_since_epoch() - std::chrono::seconds(tt)).count());

        return true;
    }
}
