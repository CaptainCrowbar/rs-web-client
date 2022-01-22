#pragma once

#include "rs-web-client/curl-utility.hpp"
#include "rs-web-client/http.hpp"
#include "rs-io/uri.hpp"
#include <chrono>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>

namespace RS::WebClient {

    class Client;

    namespace Detail {

        const std::string* get_error_buffer(const Client& c) noexcept;

    }

    using Headers = std::unordered_multimap<std::string, std::string>;

    class Client:
    private Detail::CurlInit {

    public:

        static constexpr auto default_connect_timeout = std::chrono::seconds(15);
        static constexpr auto default_request_timeout = std::chrono::seconds(60);
        static constexpr int default_redirect_limit = 10;

        Client();
        ~Client() noexcept { close(); }
        Client(const Client& c) = delete;
        Client(Client&& c) noexcept: curl_(std::exchange(c.curl_, nullptr)) {}
        Client& operator=(const Client& c) = delete;
        Client& operator=(Client&& c) noexcept;

        HttpStatus http_get(const IO::Uri& uri, std::string& body) { return perform_get(uri, nullptr, &body); }
        HttpStatus http_get(const IO::Uri& uri, Headers& head, std::string& body) { return perform_get(uri, &head, &body); }
        HttpStatus http_head(const IO::Uri& uri, Headers& head) { return perform_get(uri, &head, nullptr); }

        template <typename R, typename P> void set_connect_timeout(std::chrono::duration<R, P> t);
        template <typename R, typename P> void set_request_timeout(std::chrono::duration<R, P> t);
        void set_redirect_limit(int n);
        void set_user_agent(const std::string& user_agent);

        Curl_easy* native_handle() const noexcept { return curl_; }

    private:

        friend class OnProgress;
        friend const std::string* Detail::get_error_buffer(const Client& c) noexcept;

        Curl_easy* curl_ = nullptr;
        std::string error_buffer_;
        Headers* head_ = nullptr;
        Headers::iterator prev_header_;
        std::string* body_ = nullptr;

        void close() noexcept;
        HttpStatus perform_get(const IO::Uri& uri, Headers* head, std::string* body);
        void set_connect_timeout_ms(std::chrono::milliseconds ms);
        void set_request_timeout_ms(std::chrono::milliseconds ms);

        static size_t header_callback(char* buffer, size_t size, size_t nitems, Client* client_ptr);
        static size_t write_callback(char* ptr, size_t size, size_t nmemb, Client* client_ptr);

    };

        template <typename R, typename P>
        void Client::set_request_timeout(std::chrono::duration<R, P> t) {
            using namespace std::chrono;
            set_request_timeout_ms(duration_cast<milliseconds>(t));
        }

        template <typename R, typename P>
        void Client::set_connect_timeout(std::chrono::duration<R, P> t) {
            using namespace std::chrono;
            set_connect_timeout_ms(duration_cast<milliseconds>(t));
        }

    class OnProgress {
    public:
        using callback = std::function<bool(int64_t dltotal, int64_t dlnow)>;
        OnProgress(Client& c, callback on_download);
        ~OnProgress() noexcept;
        OnProgress(const OnProgress&) = delete;
        OnProgress(OnProgress&&) = delete;
        OnProgress& operator=(const OnProgress&) = delete;
        OnProgress& operator=(OnProgress&&) = delete;
    private:
        Client& client_;
        callback on_download_;
        static int progress_callback(OnProgress* ptr, int64_t dltotal, int64_t dlnow, int64_t ultotal, int64_t ulnow) noexcept;
    };

}
