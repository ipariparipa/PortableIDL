
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
    localtime_r(&t, &tm_1);
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
