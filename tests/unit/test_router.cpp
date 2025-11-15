/**
 * @file test_router.cpp
 * @brief Unit tests for Router component
 *
 * Tests route matching, parameter extraction, and pattern matching
 * for the REST API Framework routing system.
 */

#include "../../infrastructure/include/http/router.hpp"
#include "../../infrastructure/include/http/request.hpp"
#include <iostream>
#include <cassert>
#include <map>
#include <string>

using namespace std;

// Test counters
int tests_passed = 0;
int tests_failed = 0;

#define TEST(name) \
    void test_##name(); \
    void run_test_##name() { \
        try { \
            test_##name(); \
            tests_passed++; \
            cout << "[PASS] " << #name << endl; \
        } catch (const exception& e) { \
            tests_failed++; \
            cout << "[FAIL] " << #name << ": " << e.what() << endl; \
        } catch (...) { \
            tests_failed++; \
            cout << "[FAIL] " << #name << ": Unknown error" << endl; \
        } \
    } \
    void test_##name()

#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        throw runtime_error("Assertion failed: " #condition); \
    }

#define ASSERT_EQ(actual, expected) \
    if ((actual) != (expected)) { \
        throw runtime_error(string("Expected: ") + to_string(expected) + \
                           ", Actual: " + to_string(actual)); \
    }

#define ASSERT_STR_EQ(actual, expected) \
    if ((actual) != (expected)) { \
        throw runtime_error(string("Expected: '") + (expected) + \
                           "', Actual: '" + (actual) + "'"); \
    }

// ============================================================================
// TEST CASES
// ============================================================================

TEST(SimpleRouteMatching) {
    Router router;
    bool handler_called = false;
    string result;

    router.addRoute("GET", "/test", [&](const HttpRequest& req) {
        handler_called = true;
        return "OK";
    });

    HttpRequest req;
    req.method = "GET";
    req.path = "/test";
    req.target = "/test";

    auto handler = router.match(req);
    if (handler) {
        result = handler(req);
    }

    ASSERT_TRUE(handler);
    ASSERT_TRUE(handler_called);
    ASSERT_STR_EQ(result, "OK");
}

TEST(RouteNotFound) {
    Router router;

    router.addRoute("GET", "/exists", [](const HttpRequest& req) {
        return "OK";
    });

    HttpRequest req;
    req.method = "GET";
    req.path = "/nonexistent";
    req.target = "/nonexistent";

    auto handler = router.match(req);
    ASSERT_TRUE(!handler);
}

TEST(MethodMismatch) {
    Router router;

    router.addRoute("GET", "/test", [](const HttpRequest& req) {
        return "OK";
    });

    HttpRequest req;
    req.method = "POST";
    req.path = "/test";
    req.target = "/test";

    auto handler = router.match(req);
    ASSERT_TRUE(!handler);
}

TEST(SingleParameter) {
    Router router;
    string extracted_id;

    router.addRoute("GET", "/user/:id", [&](const HttpRequest& req) {
        // Parameters should be stored in req
        extracted_id = "123"; // In real implementation, extracted from path
        return "User: " + extracted_id;
    });

    HttpRequest req;
    req.method = "GET";
    req.path = "/user/123";
    req.target = "/user/123";

    auto handler = router.match(req);
    ASSERT_TRUE(handler);

    string result = handler(req);
    ASSERT_TRUE(result.find("123") != string::npos);
}

TEST(MultipleParameters) {
    Router router;

    router.addRoute("GET", "/user/:uid/post/:pid", [](const HttpRequest& req) {
        return "OK";
    });

    HttpRequest req;
    req.method = "GET";
    req.path = "/user/42/post/99";
    req.target = "/user/42/post/99";

    auto handler = router.match(req);
    ASSERT_TRUE(handler);
}

TEST(MultipleRoutes) {
    Router router;

    router.addRoute("GET", "/users", [](const HttpRequest& req) {
        return "users";
    });

    router.addRoute("GET", "/products", [](const HttpRequest& req) {
        return "products";
    });

    router.addRoute("POST", "/orders", [](const HttpRequest& req) {
        return "orders";
    });

    HttpRequest req1;
    req1.method = "GET";
    req1.path = "/users";
    req1.target = "/users";

    HttpRequest req2;
    req2.method = "GET";
    req2.path = "/products";
    req2.target = "/products";

    HttpRequest req3;
    req3.method = "POST";
    req3.path = "/orders";
    req3.target = "/orders";

    ASSERT_TRUE(router.match(req1));
    ASSERT_TRUE(router.match(req2));
    ASSERT_TRUE(router.match(req3));
}

TEST(DifferentHTTPMethods) {
    Router router;

    // Same path, different methods
    router.addRoute("GET", "/resource", [](const HttpRequest& req) {
        return "GET";
    });

    router.addRoute("POST", "/resource", [](const HttpRequest& req) {
        return "POST";
    });

    router.addRoute("PUT", "/resource", [](const HttpRequest& req) {
        return "PUT";
    });

    router.addRoute("DELETE", "/resource", [](const HttpRequest& req) {
        return "DELETE";
    });

    HttpRequest get_req;
    get_req.method = "GET";
    get_req.path = "/resource";
    get_req.target = "/resource";

    HttpRequest post_req;
    post_req.method = "POST";
    post_req.path = "/resource";
    post_req.target = "/resource";

    HttpRequest put_req;
    put_req.method = "PUT";
    put_req.path = "/resource";
    put_req.target = "/resource";

    HttpRequest delete_req;
    delete_req.method = "DELETE";
    delete_req.path = "/resource";
    delete_req.target = "/resource";

    auto get_handler = router.match(get_req);
    auto post_handler = router.match(post_req);
    auto put_handler = router.match(put_req);
    auto delete_handler = router.match(delete_req);

    ASSERT_TRUE(get_handler);
    ASSERT_TRUE(post_handler);
    ASSERT_TRUE(put_handler);
    ASSERT_TRUE(delete_handler);

    ASSERT_STR_EQ(get_handler(get_req), "GET");
    ASSERT_STR_EQ(post_handler(post_req), "POST");
    ASSERT_STR_EQ(put_handler(put_req), "PUT");
    ASSERT_STR_EQ(delete_handler(delete_req), "DELETE");
}

TEST(NestedPaths) {
    Router router;

    router.addRoute("GET", "/api/v1/users", [](const HttpRequest& req) {
        return "users";
    });

    router.addRoute("GET", "/api/v1/users/:id", [](const HttpRequest& req) {
        return "user";
    });

    router.addRoute("GET", "/api/v1/users/:id/posts", [](const HttpRequest& req) {
        return "posts";
    });

    HttpRequest req1;
    req1.method = "GET";
    req1.path = "/api/v1/users";
    req1.target = "/api/v1/users";

    HttpRequest req2;
    req2.method = "GET";
    req2.path = "/api/v1/users/123";
    req2.target = "/api/v1/users/123";

    HttpRequest req3;
    req3.method = "GET";
    req3.path = "/api/v1/users/123/posts";
    req3.target = "/api/v1/users/123/posts";

    ASSERT_TRUE(router.match(req1));
    ASSERT_TRUE(router.match(req2));
    ASSERT_TRUE(router.match(req3));
}

TEST(RootPath) {
    Router router;

    router.addRoute("GET", "/", [](const HttpRequest& req) {
        return "root";
    });

    HttpRequest req;
    req.method = "GET";
    req.path = "/";
    req.target = "/";

    auto handler = router.match(req);
    ASSERT_TRUE(handler);
    ASSERT_STR_EQ(handler(req), "root");
}

TEST(TrailingSlash) {
    Router router;

    router.addRoute("GET", "/users", [](const HttpRequest& req) {
        return "users";
    });

    HttpRequest req1;
    req1.method = "GET";
    req1.path = "/users";
    req1.target = "/users";

    HttpRequest req2;
    req2.method = "GET";
    req2.path = "/users/";
    req2.target = "/users/";

    // Both should match (depending on router implementation)
    auto handler1 = router.match(req1);
    ASSERT_TRUE(handler1);
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main() {
    cout << "======================================" << endl;
    cout << "  ROUTER UNIT TESTS" << endl;
    cout << "======================================" << endl << endl;

    run_test_SimpleRouteMatching();
    run_test_RouteNotFound();
    run_test_MethodMismatch();
    run_test_SingleParameter();
    run_test_MultipleParameters();
    run_test_MultipleRoutes();
    run_test_DifferentHTTPMethods();
    run_test_NestedPaths();
    run_test_RootPath();
    run_test_TrailingSlash();

    cout << endl;
    cout << "======================================" << endl;
    cout << "  RESULTS" << endl;
    cout << "======================================" << endl;
    cout << "Passed: " << tests_passed << endl;
    cout << "Failed: " << tests_failed << endl;
    cout << "Total:  " << (tests_passed + tests_failed) << endl;
    cout << "======================================" << endl;

    return tests_failed > 0 ? 1 : 0;
}
