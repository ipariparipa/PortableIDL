
#include "datetime_test.h"

#include <cppunit/config/SourcePrefix.h>

#include <iostream>

#include <pidlCore/datetime.h>

CPPUNIT_TEST_SUITE_REGISTRATION(DateTime_Test);

void DateTime_Test::setUp()
{
}

void DateTime_Test::tearDown()
{
}

void DateTime_Test::tm_test()
{
    tm tm_1;
    time_t t;
    time(&t);
#if PIDL_OS == PIDL_OS_WINDOWS
    localtime_s(&tm_1 , &t);
#else
    localtime_r(&t, &tm_1);
#endif
    PIDL::DateTime dt;
    CPPUNIT_ASSERT(PIDL::toDateTime(tm_1, dt));
    tm tm_2;
    CPPUNIT_ASSERT(PIDL::fromDateTime(dt, tm_2));
    CPPUNIT_ASSERT_EQUAL(mktime(&tm_1), mktime(&tm_2));
}

void DateTime_Test::chrono_test()
{
    auto now = std::chrono::system_clock::now();

    PIDL::DateTime dt = PIDL::toDateTime(now);
    std::chrono::system_clock::time_point tp;
    CPPUNIT_ASSERT(PIDL::fromDateTime(dt, tp));
    CPPUNIT_ASSERT_EQUAL(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count(),
                         std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count());
}

void DateTime_Test::local_test()
{
    //DST:
    {
        auto tp = std::chrono::system_clock::time_point(std::chrono::seconds(1596364463)); //2020-08-02 10:34:23 UTC
        PIDL::DateTime dt;
        CPPUNIT_ASSERT(PIDL::toDateTime(tp, dt));
        CPPUNIT_ASSERT_EQUAL(static_cast<short>(12), dt.hour);

        {
            tm tmp;
            PIDL::fromDateTime(dt, tmp);
#if PIDL_OS != PIDL_OS_WINDOWS
            CPPUNIT_ASSERT_EQUAL(std::chrono::duration_cast<std::chrono::hours>(std::chrono::hours(10 /*GMT*/) + std::chrono::seconds(tmp.tm_gmtoff)).count(), std::chrono::hours(tmp.tm_hour).count());
#endif
        }

        CPPUNIT_ASSERT_EQUAL(tp.time_since_epoch().count(), PIDL::toTimepoint(dt).time_since_epoch().count());
    }

    //no DST:
    {
        auto tp = std::chrono::system_clock::time_point(std::chrono::seconds(1605180863)); //2020-11-02 10:34:23 UTC
        PIDL::DateTime dt;
        CPPUNIT_ASSERT(PIDL::toDateTime(tp, dt));

        {
            tm tmp;
            PIDL::fromDateTime(dt, tmp);
#if PIDL_OS != PIDL_OS_WINDOWS
            CPPUNIT_ASSERT_EQUAL(std::chrono::duration_cast<std::chrono::hours>(std::chrono::hours(11 /*GMT*/) + std::chrono::seconds(tmp.tm_gmtoff)).count(), std::chrono::hours(tmp.tm_hour).count());
#endif
        }

        CPPUNIT_ASSERT_EQUAL(static_cast<short>(12), dt.hour);
        CPPUNIT_ASSERT_EQUAL(tp.time_since_epoch().count(), PIDL::toTimepoint(dt).time_since_epoch().count());
    }
}

void DateTime_Test::utc_test()
{
    //DST:
    {
        PIDL::DateTime dt;
        dt.year = 2020;
        dt.month = 8;
        dt.day = 2;
        dt.hour = 10;
        dt.minute= 34;
        dt.second = 23;
        dt.kind = PIDL::DateTime::UTC;

        CPPUNIT_ASSERT_EQUAL(std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::seconds(1596364463)).count(), PIDL::toTimepoint(dt).time_since_epoch().count());
    }

    //no DST:
    {
        PIDL::DateTime dt;
        dt.year = 2020;
        dt.month = 12;
        dt.day = 2;
        dt.hour = 10;
        dt.minute= 34;
        dt.second = 23;
        dt.kind = PIDL::DateTime::UTC;

        CPPUNIT_ASSERT_EQUAL(std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::seconds(1606905263)).count(), PIDL::toTimepoint(dt).time_since_epoch().count());
    }
}
