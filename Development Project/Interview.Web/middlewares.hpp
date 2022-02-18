#pragma once

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

#include "json.hpp"

#include "StrUtils.h"
#include "http.h"
#include "router.h"

namespace middlewares {
std::shared_ptr<iti::http::IHandler>
logging(std::shared_ptr<iti::http::IHandler> next) {
	auto func = [nxt = std::move(next)](const iti::http::Request &req,
	                                    iti::http::Response &resp) {
		using iti::http::router::RoutingContext;
		using clock_type = std::chrono::high_resolution_clock;

		// get or create routing context
		std::shared_ptr<RoutingContext> rctx =
		    RoutingContext::get_create_ctx_from_request(req);

		std::string_view path{rctx->routePath};
		if (path.empty()) {
			path = req.url.path;
		}

		std::chrono::time_point<clock_type> begin = clock_type::now();

		void *pReq = (void *)&req;

		std::string msg =
		    fmt::format("LoggingMiddleware: {}: Path: {}", pReq, path);

		std::cout << msg << '\n';

		if (nxt != nullptr) {
			nxt->handle_request(req, resp);
		}

		auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
		    clock_type::now() - begin);

		msg = fmt::format("LoggingMiddleware: {}: Exit {}", pReq,
		                  iti::strutils::humanize_duration(duration));
		std::cout << msg << '\n';
	};

	return iti::http::IHandler::make_handler(new iti::http::BasicHandler(func));
}

std::shared_ptr<iti::http::IHandler>
trim_trailing_slash(std::shared_ptr<iti::http::IHandler> next) {
	auto func = [nxt = std::move(next)](const iti::http::Request &req,
	                                    iti::http::Response &resp) {
		using iti::http::router::RoutingContext;

		// get or create routing context
		std::shared_ptr<RoutingContext> rctx =
		    RoutingContext::get_create_ctx_from_request(req);

		// make sure we populate routePath
		if (rctx->routePath.empty()) {
			rctx->routePath = req.url.path;
		}

		// trim the trailing slash ('/')
		// we don't want to empty the path
		auto newroutePathSize = rctx->routePath.size() - 1;
		if (newroutePathSize > 0 && rctx->routePath[newroutePathSize] == '/') {
			rctx->routePath.resize(newroutePathSize);
		}

		if (nxt != nullptr) {
			nxt->handle_request(req, resp);
		}
	};

	return iti::http::IHandler::make_handler(new iti::http::BasicHandler(func));
}

std::shared_ptr<iti::http::IHandler>
extract_id(std::shared_ptr<iti::http::IHandler> next) {
	auto func = [nxt = std::move(next)](const iti::http::Request &req,
	                                    iti::http::Response &resp) {
		using iti::http::StatusCode;
		using iti::http::router::RoutingContext;
		using nlohmann::json;

		// get routing context
		std::shared_ptr<RoutingContext> rctx = nullptr;
		if (!req.context.try_get_value(RoutingContext::routeCtxKey, rctx)) {
			resp.header.set("Content-Type", "application/json");
			resp.status = StatusCode::Status500InternalServerError;

			json j;
			j["code"]    = resp.status;
			j["message"] = "RoutingContext was not present.";

			resp.write(j.dump(4));
			return;
		}

		std::string rawID = rctx->get_url_param("id");
		long long id{};
		const auto idRes =
		    std::from_chars(rawID.data(), rawID.data() + rawID.size(), id);

		if (idRes.ec != std::errc()) {
			resp.header.set("Content-Type", "application/json");
			resp.status = StatusCode::Status400BadRequest;

			json j;
			j["code"]    = resp.status;
			j["message"] = req.url.path + ": id not found or is 0";

			resp.write(j.dump(4));
			return;
		}

		req.context.set_value("id", id);

		if (nxt != nullptr) {
			nxt->handle_request(req, resp);
		}
	};

	return iti::http::IHandler::make_handler(new iti::http::BasicHandler(func));
}

} // namespace middlewares
