
#ifndef pidlCore__datetime_h
#define pidlCore__datetime_h

#include "config.h"

#include <time.h>
#include <chrono>

namespace PIDL {

	struct DateTime
	{
		enum Kind
		{
			None, UTC, Local
		};

        short year = 0;
        short month = 1;
        short day = 0;
        short hour = 0;
        short minute = 0;
        short second = 0;
        int nanosecond = 0;

		Kind kind = None;
	};

	extern PIDL_CORE__FUNCTION bool fromDateTime(const DateTime & dt, tm & ret);
    extern PIDL_CORE__FUNCTION bool fromDateTime(const DateTime & dt, std::chrono::system_clock::time_point & ret);
    extern PIDL_CORE__FUNCTION tm fromDateTime(const DateTime & dt);
    extern PIDL_CORE__FUNCTION tm toTm(const DateTime & dt);
    extern PIDL_CORE__FUNCTION std::chrono::system_clock::time_point toTimepoint(const DateTime & dt);
    extern PIDL_CORE__FUNCTION bool toDateTime(const tm & t, DateTime & ret);
    extern PIDL_CORE__FUNCTION bool toDateTime(std::chrono::system_clock::time_point t, DateTime & ret);

    template<typename T>
    DateTime toDateTime(const T & t)
    {
        DateTime ret;
        toDateTime(t, ret);
        return ret;
    }

    template<typename T>
    struct FromDateTime
    {
        FromDateTime() = default;
        T operator() (const DateTime & dt) = delete;
    };

    template<>
    struct FromDateTime<tm>
    {
        FromDateTime() = default;
        tm operator() (const DateTime & dt)
        {
            return toTm(dt);
        }
    };

    template<>
    struct FromDateTime<std::chrono::system_clock::time_point>
    {
        FromDateTime() = default;
        std::chrono::system_clock::time_point operator() (const DateTime & dt)
        {
            return toTimepoint(dt);
        }
    };
}

#endif // pidlCore__datetime_h
