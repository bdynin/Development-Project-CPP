#pragma once

#include <evhttp.h>
#include <numeric>

#include "http.h"

class evHttpResponse : public iti::http::Response {
  public:
	evHttpResponse() = default;
	evHttpResponse(struct evhttp_request *r) : req(r) {}

	void write(const std::string &body = "") override {
		responseReadyToSend = true;

		this->body = body;
	}

	bool process_response() {
		if (responseSent) {
			throw std::logic_error(
			    "evHttpResponse: response has already been sent.");
		}

		if (!responseReadyToSend) {
			return false;
		}

		auto buf = evbuffer_new();
		evbuffer_add(buf, body.data(), body.size());

		iti::http::StatusCode statusCode;
		if (iti::http::StatusCode::try_parse(status, statusCode)) {
			statusCode = iti::http::StatusCode::Status200OK;
		}

		auto outHeaders = evhttp_request_get_output_headers(req);
		auto headerEnd  = header.cend();
		for (auto it = header.cbegin(); it != headerEnd; it++) {
			// skip headers with no content
			if (it->second.empty()) {
				continue;
			}

			std::string hContent = *it->second.begin();

			hContent = std::accumulate(
			    ++it->second.begin(), it->second.end(), hContent,
			    [](const std::string &a, const std::string &b) {
				    return a + "; " + b;
			    });

			evhttp_add_header(outHeaders, it->first.c_str(), hContent.c_str());
		}

		// figure out the status code
		std::string statusCodeReason{statusCode.str()};
		statusCodeReason = statusCodeReason.substr(statusCodeReason.find(' '));

		evhttp_send_reply(req, status, statusCodeReason.c_str(), buf);

		evbuffer_free(buf);

		responseSent = true;
		return true;
	}

	bool get_ready_to_send() const {
		return responseReadyToSend && !responseSent;
	}

	bool get_response_sent() const { return responseSent; }

  protected:
	struct evhttp_request *req = nullptr;
	bool responseSent          = false;
	bool responseReadyToSend   = false;
	std::string body;
};
