// This file is generated by the rs-update-tests script

#include "rs-unit-test.hpp"

int main(int argc, char** argv) {

    RS::UnitTest::begin_tests(argc, argv);

    // client-test.cpp
    UNIT_TEST(rs_web_client_http_get)
    UNIT_TEST(rs_web_client_http_head)

    // version-test.cpp
    UNIT_TEST(rs_web_client_version)

    // unit-test.cpp

    return RS::UnitTest::end_tests();

}
