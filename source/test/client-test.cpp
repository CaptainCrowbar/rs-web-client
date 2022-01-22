#include "rs-web-client/client.hpp"
#include "rs-web-client/http.hpp"
#include "rs-io/uri.hpp"
#include "rs-unit-test.hpp"
#include <string>

using namespace RS::IO;
using namespace RS::WebClient;

void test_rs_web_client_http_get() {

    Client client;
    Uri uri;
    Headers head;
    Headers::const_iterator it;
    std::string body, key;
    HttpStatus status = {};

    TEST(client.native_handle() != nullptr);
    TRY(uri =Uri("https://en.wikipedia.org/wiki/Main_Page"));
    TRY(status = client.http_get(uri, head, body));
    TEST_EQUAL(status, HttpStatus::ok);
    TEST_EQUAL(int(status), 200);
    TEST_EQUAL(to_string(status), "200 OK");
    TEST(head.size() > 20);
    TEST(body.size() > 10'000);
    TEST_MATCH(body.substr(0, 100), "^<!DOCTYPE html>\n");

    key = "content-length";
    TEST_EQUAL(head.count(key), 1u);
    TRY(it = head.find(key));
    TEST(it != head.end());
    if (it != head.end())
        TEST_MATCH(it->second, R"(^\d+$)");

    key = "content-type";
    TEST_EQUAL(head.count(key), 1u);
    TRY(it = head.find(key));
    TEST(it != head.end());
    if (it != head.end())
        TEST_MATCH(it->second, "^text/html");

    key = "date";
    TEST_EQUAL(head.count(key), 1u);
    TRY(it = head.find(key));
    TEST(it != head.end());
    if (it != head.end())
        TEST_MATCH(it->second, R"(^[A-Z][a-z]{2}, \d\d [A-Z][a-z]{2} 20\d\d \d\d:\d\d:\d\d GMT$)");

    key = "last-modified";
    TEST_EQUAL(head.count(key), 1u);
    TRY(it = head.find(key));
    TEST(it != head.end());
    if (it != head.end())
        TEST_MATCH(it->second, R"(^[A-Z][a-z]{2}, \d\d [A-Z][a-z]{2} 20\d\d \d\d:\d\d:\d\d GMT$)");

}

void test_rs_web_client_http_head() {

    Client client;
    Uri uri;
    Headers head;
    Headers::const_iterator it;
    std::string key;
    HttpStatus status = {};

    TRY(uri =Uri("https://en.wikipedia.org/wiki/Main_Page"));
    TRY(status = client.http_head(uri, head));
    TEST_EQUAL(status, HttpStatus::ok);
    TEST_EQUAL(int(status), 200);
    TEST_EQUAL(to_string(status), "200 OK");
    TEST(head.size() > 20);

    key = "content-length";
    TEST_EQUAL(head.count(key), 1u);
    TRY(it = head.find(key));
    TEST(it != head.end());
    if (it != head.end())
        TEST_MATCH(it->second, R"(^\d+$)");

    key = "content-type";
    TEST_EQUAL(head.count(key), 1u);
    TRY(it = head.find(key));
    TEST(it != head.end());
    if (it != head.end())
        TEST_MATCH(it->second, "^text/html");

    key = "date";
    TEST_EQUAL(head.count(key), 1u);
    TRY(it = head.find(key));
    TEST(it != head.end());
    if (it != head.end())
        TEST_MATCH(it->second, R"(^[A-Z][a-z]{2}, \d\d [A-Z][a-z]{2} 20\d\d \d\d:\d\d:\d\d GMT$)");

    key = "last-modified";
    TEST_EQUAL(head.count(key), 1u);
    TRY(it = head.find(key));
    TEST(it != head.end());
    if (it != head.end())
        TEST_MATCH(it->second, R"(^[A-Z][a-z]{2}, \d\d [A-Z][a-z]{2} 20\d\d \d\d:\d\d:\d\d GMT$)");

}
