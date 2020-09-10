
#ifndef __datetime_test_h__
#define __datetime_test_h__

#include <cppunit/extensions/HelperMacros.h>

class DateTime_Test : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(DateTime_Test);
    CPPUNIT_TEST(tm_test);
    CPPUNIT_TEST(chrono_test);
    CPPUNIT_TEST_SUITE_END();

public:
    virtual void setUp() override;

    virtual void tearDown() override;

protected:
    void tm_test();
    void chrono_test();
};

#endif //__datetime_test_h__
