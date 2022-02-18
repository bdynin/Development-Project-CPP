#ifndef ITI_LIB_HTTP_ROUTER_CONTEXT_H
#define ITI_LIB_HTTP_ROUTER_CONTEXT_H

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "context.h"
#include "http.h"
#include "router.RouteParams.h"

namespace iti {
namespace http {
namespace router {

class IRoutes;
class RoutingContext;
class Node;

// Context is the default routing context set on the root node of a
// request context to track route patterns, URL parameters and
// an optional routing path.
class RoutingContext {
	friend class Node;

  public:
	static constexpr const std::string_view routeCtxKey =
	    "iti::http::router::RoutingContext";

	static std::shared_ptr<RoutingContext>
	get_create_ctx_from_request(const Request &req);

	static std::string get_url_param_from_ctx(const iti::Context &ctx,
	                                          const std::string &key);

	RoutingContext &operator=(RoutingContext &&) = default;

	std::shared_ptr<iti::http::router::IRoutes> routes = nullptr;

	// Routing path/method override used during the route search.
	// See "router.mux.h": `Mux.route_http()` method.
	std::string routePath;

	iti::http::Method routeMethod = iti::http::Method::unknown;

	// urlParams are the stack of routeParams captured during the
	// routing lifecycle across a stack of sub-routers.
	RouteParams urlParams;

	// Routing pattern stack throughout the lifecycle of the request,
	// across all connected routers. It is a record of all matching
	// patterns across a stack of sub-routers.
	std::vector<std::string> routePatterns;

	// Reset a routing context to its initial state.
	void reset();

	// get_url_param returns the corresponding URL parameter value from the
	// request routing context.
	std::string get_url_param(const std::string &key);

	std::string join_route_patterns() const;

	void set_parent_context(const iti::Context &ctx) { parentCtx = ctx; }

	RouteParams get_route_params();

	bool get_method_not_allowed_hint() const { return methodNotAllowed; }

  protected:
	iti::Context parentCtx;

	// Route parameters matched for the current sub-router. It is
	// intentionally private so it cant be tampered.
	RouteParams routeParams;

	// The endpoint routing pattern that matched the request URI path
	// or `RoutePath` of the current sub-router. This value will update
	// during the lifecycle of a request passing through a stack of
	// sub-routers.
	std::string routePattern;

	// methodNotAllowed hint
	bool methodNotAllowed = false;
};

} // namespace router
} // namespace http
} // namespace iti

#endif // ITI_LIB_HTTP_ROUTER_CONTEXT_H
