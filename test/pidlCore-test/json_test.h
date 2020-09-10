
#ifndef __json_test_h__
#define __json_test_h__

#include <cppunit/extensions/HelperMacros.h>

class JSON_Test : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(JSON_Test);
    CPPUNIT_TEST(double_as_int);
    CPPUNIT_TEST(neg_int);
    CPPUNIT_TEST(set_get_array);
    CPPUNIT_TEST_SUITE_END();

public:
    virtual void setUp() override;

    virtual void tearDown() override;

protected:
    void double_as_int();
    void neg_int();
    void set_get_array();
};

#endif //__json_test_h__
