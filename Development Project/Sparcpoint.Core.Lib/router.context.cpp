#ifndef ITI_LIB_HTTP_ROUTER_CONTEXT_CPP
#define ITI_LIB_HTTP_ROUTER_CONTEXT_CPP

#include "pch.h"

#include "router.context.h"

#include <algorithm>
#include <numeric>

#include "router.h"

using iti::http::Method;
using iti::http::router::RouteParams;

// routing context
// ----------------------------------------------------------------------------
void iti::http::router::RoutingContext::reset() {
	routes = nullptr;
	routePath.clear();
	routeMethod = Method::unknown;
	routePatterns.clear();
	urlParams.keys.clear();
	urlParams.values.clear();

	routePattern.clear();
	routeParams.keys.clear();
	routeParams.values.clear();
	methodNotAllowed = false;
}

std::string
iti::http::router::RoutingContext::get_url_param(const std::string &key) {
	if (urlParams.keys.empty()) {
		return std::string();
	}

	for (auto it = urlParams.keys.rbegin(); it != urlParams.keys.rend(); it++) {
		if (*it == key) {
			size_t idx = std::distance(urlParams.keys.begin(), it.base()) - 1;
			if (idx < urlParams.values.size()) {
				return urlParams.values[idx];
			}
		}
	}

	return std::string();
}

std::string iti::http::router::RoutingContext::join_route_patterns() const {
	// bail if there are no items in the list
	if (routePatterns.empty()) {
		return std::string();
	}

	std::string pattern = *routePatterns.begin();

	pattern = std::accumulate(
	    ++routePatterns.begin(),
	    routePatterns.end(), // the range 2nd to after-last
	    pattern,             // and start accumulating with the first item
	    [](const std::string &a, const std::string &b) { return a + "" + b; });

	// replace all wildcards (occurrences of "/*/") to "/".
	return strutils::replace_copy(pattern, "/*/", "/");
}

std::string iti::http::router::RoutingContext::get_url_param_from_ctx(
    const iti::Context &ctx, const std::string &key) {

	std::shared_ptr<RoutingContext> rctx = nullptr;
	if (ctx.try_get_value(RoutingContext::routeCtxKey, rctx) &&
	    rctx != nullptr) {
		return rctx->get_url_param(key);
	}

	return std::string();
}

RouteParams iti::http::router::RoutingContext::get_route_params() {
	return RouteParams(routeParams);
}

std::shared_ptr<iti::http::router::RoutingContext>
iti::http::router::RoutingContext::get_create_ctx_from_request(
    const Request &req) {
	std::shared_ptr<RoutingContext> rctx = nullptr;
	if (!req.context.try_get_value(RoutingContext::routeCtxKey, rctx)) {
		rctx = std::make_shared<RoutingContext>();
		rctx->set_parent_context(req.context);
		req.context.set_value(RoutingContext::routeCtxKey, rctx);
	}

	return rctx;
}

#endif // ITI_LIB_HTTP_ROUTER_CONTEXT_CPP
