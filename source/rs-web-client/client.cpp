#include "rs-web-client/client.hpp"
#include "rs-web-client/curl-api.hpp"
#include "rs-format/string.hpp"
#include <algorithm>
#include <cstdio>
#include <cstring>

using namespace RS::Format;
using namespace RS::IO;

namespace RS::WebClient {

    namespace Detail {

        const std::string* get_error_buffer(const Client& c) noexcept {
            if (c.error_buffer_.empty())
                return nullptr;
            else
                return &c.error_buffer_;
        }

    }

    // Client class

    Client::Client() {

        auto curl = curl_easy_init();
        if (curl == nullptr)
            throw CurlError(0, "curl_easy_init()");
        curl_ = curl;

        error_buffer_.resize(CURL_ERROR_SIZE, '\0');
        Detail::set_option<CURLOPT_ERRORBUFFER>(*this, error_buffer_.data());
        Detail::set_option<CURLOPT_NOPROGRESS>(*this, true);
        Detail::set_option<CURLOPT_NOSIGNAL>(*this, true);
        Detail::set_option<CURLOPT_HEADERFUNCTION>(*this, header_callback);
        Detail::set_option<CURLOPT_HEADERDATA>(*this, this);
        Detail::set_option<CURLOPT_WRITEFUNCTION>(*this, write_callback);
        Detail::set_option<CURLOPT_WRITEDATA>(*this, this);
        Detail::set_option<CURLOPT_ACCEPT_ENCODING>(*this, "");
        set_connect_timeout(default_connect_timeout);
        set_request_timeout(default_request_timeout);
        set_redirect_limit(default_redirect_limit);

    }

    Client& Client::operator=(Client&& c) noexcept {
        if (&c != this) {
            close();
            curl_ = std::exchange(c.curl_, nullptr);
        }
        return *this;
    }

    void Client::set_redirect_limit(int n) {
        Detail::set_option<CURLOPT_FOLLOWLOCATION>(*this, true);
        Detail::set_option<CURLOPT_MAXREDIRS>(*this, std::max(n, 0));
    }

    void Client::set_user_agent(const std::string& user_agent) {
        Detail::set_option<CURLOPT_USERAGENT>(*this, user_agent);
    }

    void Client::close() noexcept {
        if (curl_ != nullptr)
            curl_easy_cleanup(curl_);
    }

    HttpStatus Client::perform_get(const IO::Uri& uri, Headers* head, std::string* body) {
        head_ = head;
        body_ = body;
        if (head != nullptr)
            head->clear();
        if (body != nullptr)
            body->clear();
        Detail::set_option<CURLOPT_URL>(*this, uri.str());
        if (body == nullptr)
            Detail::set_option<CURLOPT_NOBODY>(*this, true);
        else
            Detail::set_option<CURLOPT_HTTPGET>(*this, true);
        Detail::check_api(*this, curl_easy_perform(curl_), "curl_easy_perform()", uri.str());
        body_ = nullptr;
        head_ = nullptr;
        int rc = 0;
        Detail::get_info<CURLINFO_RESPONSE_CODE>(*this, rc);
        return HttpStatus(rc);
    }

    void Client::set_connect_timeout_ms(std::chrono::milliseconds ms) {
        Detail::set_option<CURLOPT_CONNECTTIMEOUT_MS>(*this, ms.count());
    }

    void Client::set_request_timeout_ms(std::chrono::milliseconds ms) {
        Detail::set_option<CURLOPT_TIMEOUT_MS>(*this, ms.count());
    }

    size_t Client::header_callback(char* buffer, size_t /*size*/, size_t nitems, Client* client_ptr) {
        if (nitems == 0 || client_ptr->head_ == nullptr)
            return nitems;
        std::string line(buffer, nitems);
        if (line == "\r\n")
            return nitems;
        auto& headers = *client_ptr->head_;
        auto& prev = client_ptr->prev_header_;
        if (! headers.empty() && ascii_isspace(line[0])) {
            prev->second += " " + trim(line);
            return nitems;
        }
        auto [key,value] = partition(line, ":");
        prev = headers.insert({trim(key), trim(value)});
        return nitems;
    }

    size_t Client::write_callback(char* ptr, size_t /*size*/, size_t nmemb, Client* client_ptr) {
        if (client_ptr->body_ == nullptr)
            return nmemb;
        auto& to = *client_ptr->body_;
        size_t offset = to.size();
        to.resize(offset + nmemb);
        std::memcpy(to.data() + offset, ptr, nmemb);
        return nmemb;
    }

    // OnProgress class

    OnProgress::OnProgress(Client& c, callback on_download):
    client_(c), on_download_(on_download) {
        Detail::set_option<CURLOPT_XFERINFOFUNCTION>(c, progress_callback);
        Detail::set_option<CURLOPT_XFERINFODATA>(c, this);
        Detail::set_option<CURLOPT_NOPROGRESS>(c, false);
    }

    OnProgress::~OnProgress() noexcept {
        curl_easy_setopt(client_.native_handle(), CURLOPT_NOPROGRESS, 0L);
    }

    int OnProgress::progress_callback(OnProgress* ptr, int64_t dltotal, int64_t dlnow, int64_t /*ultotal*/, int64_t /*ulnow*/) noexcept {
        bool dl_ok = ! ptr->on_download_ || ptr->on_download_(dltotal, dlnow);
        if (dl_ok)
            return CURL_PROGRESSFUNC_CONTINUE;
        else
            return CURL_PROGRESSFUNC_CONTINUE - 1;
    }

}
