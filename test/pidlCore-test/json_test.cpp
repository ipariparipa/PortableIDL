
#include "json_test.h"

#include <cppunit/config/SourcePrefix.h>

#include <iostream>

#include <pidlCore/jsontools.h>

CPPUNIT_TEST_SUITE_REGISTRATION(JSON_Test);

void JSON_Test::setUp()
{
}

void JSON_Test::tearDown()
{
}

void JSON_Test::double_as_int()
{
    rapidjson::Document doc;
    doc.SetObject();

    PIDL::JSONTools::addValue(doc, doc, "pos_double", 42.56);

    {
        double tmp_d;
        CPPUNIT_ASSERT(PIDL::JSONTools::getValue(doc, "pos_double", tmp_d));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(42.56, tmp_d, 0.001);

        int tmp_i;
        CPPUNIT_ASSERT(PIDL::JSONTools::getValue(doc, "pos_double", tmp_i));
        CPPUNIT_ASSERT_EQUAL(42, tmp_i);

        unsigned int tmp_ui;
        CPPUNIT_ASSERT(PIDL::JSONTools::getValue(doc, "pos_double", tmp_ui));
        CPPUNIT_ASSERT_EQUAL(42, tmp_i);

        long long tmp_ll;
        CPPUNIT_ASSERT(PIDL::JSONTools::getValue(doc, "pos_double", tmp_ll));
        CPPUNIT_ASSERT_EQUAL(42ll, tmp_ll);

        unsigned long long tmp_ull;
        CPPUNIT_ASSERT(PIDL::JSONTools::getValue(doc, "pos_double", tmp_ull));
        CPPUNIT_ASSERT_EQUAL(42ull, tmp_ull);
    }

    PIDL::JSONTools::addValue(doc, doc, "neg_double", -56.42);

    {
        double tmp_d;
        CPPUNIT_ASSERT(PIDL::JSONTools::getValue(doc, "neg_double", tmp_d));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-56.42, tmp_d, 0.001);

        int tmp_i;
        CPPUNIT_ASSERT(PIDL::JSONTools::getValue(doc, "neg_double", tmp_i));
        CPPUNIT_ASSERT_EQUAL(-56, tmp_i);

        unsigned int tmp_ui;
        CPPUNIT_ASSERT(!PIDL::JSONTools::getValue(doc, "neg_double", tmp_ui));

        long long tmp_ll;
        CPPUNIT_ASSERT(PIDL::JSONTools::getValue(doc, "neg_double", tmp_ll));
        CPPUNIT_ASSERT_EQUAL(-56ll, tmp_ll);

        unsigned long long tmp_ull;
        CPPUNIT_ASSERT(!PIDL::JSONTools::getValue(doc, "neg_double", tmp_ull));
    }

}

void JSON_Test::neg_int()
{
    rapidjson::Document doc;
    doc.SetObject();

    PIDL::JSONTools::addValue(doc, doc, "neg_int", -42);

    {
        double tmp_d;
        CPPUNIT_ASSERT(PIDL::JSONTools::getValue(doc, "neg_int", tmp_d));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-42, tmp_d, 0.001);

        int tmp_i;
        CPPUNIT_ASSERT(PIDL::JSONTools::getValue(doc, "neg_int", tmp_i));
        CPPUNIT_ASSERT_EQUAL(-42, tmp_i);

        unsigned int tmp_ui;
        CPPUNIT_ASSERT(!PIDL::JSONTools::getValue(doc, "neg_int", tmp_ui));

        long long tmp_ll;
        CPPUNIT_ASSERT(PIDL::JSONTools::getValue(doc, "neg_int", tmp_ll));
        CPPUNIT_ASSERT_EQUAL(-42ll, tmp_ll);

        unsigned long long tmp_ull;
        CPPUNIT_ASSERT(!PIDL::JSONTools::getValue(doc, "pos_double", tmp_ull));
    }
}

void JSON_Test::set_get_array()
{
    rapidjson::Document doc;
    doc.SetObject();

    std::vector<std::vector<std::string>> in = {{u8"tuskó", u8"fejsze"}, {u8"glettvas", u8"gipsz"}};
    std::vector<std::vector<std::string>> out1, out2;

    PIDL::JSONTools::addValue(doc, doc, "test", in);


    CPPUNIT_ASSERT(PIDL::JSONTools::getValue(doc, "test", out1));
    rapidjson::Value * v;
    CPPUNIT_ASSERT(PIDL::JSONTools::getValue(doc, "test", v));
    CPPUNIT_ASSERT(PIDL::JSONTools::getValue(*v, out2));

    CPPUNIT_ASSERT_EQUAL(in.size(), out1.size());
    CPPUNIT_ASSERT_EQUAL(in.size(), out2.size());

    for(size_t i = 0, l = in.size(); i < l; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(in[i].size(), out1[i].size());
        CPPUNIT_ASSERT_EQUAL(in[i].size(), out2[i].size());
        for(size_t j = 0, l = in[i].size(); i < l; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(in[i][j], out2[i][j]);
            CPPUNIT_ASSERT_EQUAL(in[i][j], out2[i][j]);
        }
    }
}
