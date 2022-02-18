#ifndef ITI_LIB_HTTP_ROUTER_MUX_CPP
#define ITI_LIB_HTTP_ROUTER_MUX_CPP

#include "pch.h"

#include "router.mux.h"

#include <memory>
#include <stdexcept>
#include <tuple>

#include "context.h"
#include "fmt/format.h"

using iti::http::BasicHandler;
using iti::http::IHandler;
using iti::http::Request;
using iti::http::Response;
using iti::http::StatusCode;
using iti::http::router::IRouter;
using iti::http::router::IRoutes;
using iti::http::router::Route;
using iti::http::router::RoutingContext;

// helpers
// ----------------------------------------------------------------------------
// `chain_helper()` is in router.cpp
extern std::shared_ptr<IHandler>
chain_helper(std::vector<iti::http::router::middleware> middlewares,
             std::shared_ptr<IHandler> endpoint);

void method_not_allowed_default_handler(const Request &req, Response &resp) {
	resp.status = StatusCode::Status405MethodNotAllowed;
	resp.write(fmt::format("HTTP Method ({}) not allowed", req.method.str()));
}

void not_found_default_handler(const Request &req, Response &resp) {
	resp.status = StatusCode::Status404NotFound;
	resp.write(fmt::format("Resource ({}) not found", req.url.path));
}

// mux
// ----------------------------------------------------------------------------

void iti::http::router::Mux::handle_request(const Request &req,
                                            Response &resp) {
	// Ensure the mux has some routes defined on the mux
	if (handler == nullptr) {
		not_found_handler(req, resp);
		return;
	}

	// Check if a routing context already exists from a parent router.
	std::shared_ptr<RoutingContext> rctx = nullptr;
	bool gotCtx = req.context.try_get_value(RoutingContext::routeCtxKey, rctx);

	if (gotCtx && handler != nullptr) {
		handler->handle_request(req, resp);
		return;
	}

	rctx = std::make_shared<RoutingContext>();

	rctx->routes = shared_from_this();
	rctx->set_parent_context(req.context);

	req.context.set_value(RoutingContext::routeCtxKey, rctx);

	// Serve the request
	handler->handle_request(req, resp);
}

// Use appends a middleware handler to the Mux middleware stack.
//
// The middleware stack for any Mux will execute before searching for a matching
// route to a specific handler, which provides opportunity to respond early,
// change the course of the request execution, or set request-scoped values for
// the next http.Handler.
void iti::http::router::Mux::use(iti::http::router::middleware middleware) {
	if (handler != nullptr) {
		throw std::logic_error(
		    "mux: all middlewares must be defined before routes on a mux");
	}

	middlewares.emplace_back(std::move(middleware));
}
// Use appends a middleware handler to the Mux middleware stack.
//
// The middleware stack for any Mux will execute before searching for a matching
// route to a specific handler, which provides opportunity to respond early,
// change the course of the request execution, or set request-scoped values for
// the next http.Handler.
void iti::http::router::Mux::use(
    const std::vector<iti::http::router::middleware> &mws) {
	if (handler != nullptr) {
		throw std::logic_error(
		    "mux: all middlewares must be defined before routes on a mux");
	}

	middlewares.insert(middlewares.end(), mws.begin(), mws.end());
}

// With adds inline middlewares for an endpoint handler.
std::shared_ptr<IRouter>
iti::http::router::Mux::with(iti::http::router::middleware middleware) {
	std::vector<iti::http::router::middleware> mws;
	if (middleware != nullptr) {
		mws.emplace_back(middleware);
	}
	return with(mws);
}
// With adds inline middlewares for an endpoint handler.
std::shared_ptr<IRouter> iti::http::router::Mux::with(
    const std::vector<iti::http::router::middleware> &additionalMiddlewares) {
	// Similarly as in handle(), we must build the mux handler once additional
	// middleware registration isn't allowed for this stack, like now.
	if (!isInline && handler == nullptr) {
		update_route_handler();
	}

	// Copy middlewares from parent inline muxs
	Middlewares mws;
	if (isInline) {
		mws.collection.insert(mws.collection.end(), middlewares.begin(),
		                      middlewares.end());
	}
	mws.collection.insert(mws.collection.end(), additionalMiddlewares.begin(),
	                      additionalMiddlewares.end());

	Mux *im = new Mux;

	im->isInline                = true;
	im->parent                  = shared_from_this();
	im->tree                    = tree;
	im->middlewares             = mws.collection;
	im->notFoundHandler         = notFoundHandler;
	im->methodNotAllowedHandler = methodNotAllowedHandler;

	std::shared_ptr<IRouter> spim;
	spim.reset(im);

	return spim;
}

// group creates a new inline-Mux with a fresh middleware stack. It's useful
// for a group of handlers along the same routing path that use an additional
// set of middlewares.
std::shared_ptr<IRouter> iti::http::router::Mux::group(
    std::function<void(std::shared_ptr<IRouter> r)> fn) {

	auto im = with(nullptr);

	if (fn != nullptr) {
		fn(im);
	}
	return im;
}

// route creates a new Mux with a fresh middleware stack and mounts it
// along the `pattern` as a subrouter. Effectively, this is a short-hand
// call to Mount.
std::shared_ptr<IRouter> iti::http::router::Mux::route(
    const std::string &pattern,
    std::function<void(std::shared_ptr<IRouter> r)> fn) {
	if (fn == nullptr) {
		throw std::logic_error(fmt::format(
		    "mux: attempting to route() a nullptr subrouter on '{}'", pattern));
	}

	std::shared_ptr<IRouter> subr;
	subr.reset(new Mux);
	fn(subr);
	mount(pattern, subr);
	return subr;
}

// mount attaches another IHandler or IRouter as a subrouter along a
// routing path. It's very useful to split up a large API as many independent
// routers and compose them as a single service using mount.
//
// Note that `mount()` simply sets a wildcard along the `pattern` that will
// continue routing at the `handler`, which in most cases is another IRouter.
// As a result, if you define two Mount() routes on the exact same pattern the
// mount will throw a std::logic_error.
void iti::http::router::Mux::mount(
    const std::string &pattern, std::shared_ptr<iti::http::router::IRouter> r) {
	if (r == nullptr) {
		throw std::logic_error(fmt::format(
		    "mux: attempting to mount() a nullptr handler on '{}'", pattern));
	}

	// Provide runtime safety for ensuring a pattern isn't mounted on an
	// existing routing pattern.
	if (tree->find_pattern(pattern + "*") ||
	    tree->find_pattern(pattern + "/*")) {
		throw std::logic_error(fmt::format(
		    "mux: attempting to mount() a handler on an existing path, '{}'",
		    pattern));
	}

	// Assign sub-Router's with the parent not found & method not allowed
	// handler if not specified.
	Mux *subr = dynamic_cast<Mux *>(r.get());
	if (subr != nullptr && subr->notFoundHandler == nullptr &&
	    notFoundHandler != nullptr) {
		subr->set_not_found(notFoundHandler);
	}

	if (subr != nullptr && subr->methodNotAllowedHandler == nullptr &&
	    methodNotAllowedHandler != nullptr) {
		subr->set_method_not_allowed(methodNotAllowedHandler);
	}

	auto func = [mx = shared_from_this(), this, r = r](const Request &req,
	                                                   Response &resp) {
		std::shared_ptr<RoutingContext> rctx = nullptr;
		req.context.try_get_value(RoutingContext::routeCtxKey, rctx);

		// shift the url path past the previous subrouter
		rctx->routePath = mx->next_route_path(rctx);

		// reset the wildcard URLParam which connects the subrouter
		long long n = (long long)rctx->urlParams.keys.size() - 1;
		if (n >= 0 && rctx->urlParams.keys[n] == "*" &&
		    (long long)rctx->urlParams.values.size() > n) {
			rctx->urlParams.values[n] = "";
		}

		r->handle_request(req, resp);
	};

	auto mountHandler = IHandler::make_handler(new BasicHandler(func));

	std::string newPattern{pattern};
	if (newPattern.empty() || newPattern[newPattern.size() - 1] != '/') {
		handle_impl((Method::Value)(Method::ALL | Method::unknown), newPattern,
		            mountHandler);
		handle_impl((Method::Value)(Method::ALL | Method::unknown),
		            newPattern + "/", mountHandler);
		newPattern += "/";
	}

	auto method = (Method::Value)(Method::ALL | Method::unknown);

	auto n = handle_impl(method, newPattern + "*", mountHandler);

	n->subroutes = r;
}

std::vector<Route> iti::http::router::Mux::get_routes() {
	return tree->get_routes();
}

iti::http::router::Middlewares iti::http::router::Mux::get_middlewares() {
	Middlewares mws;
	mws.collection = middlewares;
	return mws;
}

bool iti::http::router::Mux::match(std::shared_ptr<RoutingContext> rctx,
                                   const std::string &method,
                                   const std::string &path) {
	Method m;
	if (!Method::try_parse(method, m)) {
		return false;
	}

	auto result = tree->find_route(rctx, m, path);
	auto node   = std::get<0>(result);
	auto h      = std::get<2>(result);

	if (node != nullptr && node->subroutes != nullptr) {
		rctx->routePath = next_route_path(rctx);
		return node->subroutes->match(rctx, method, rctx->routePath);
	}

	return h != nullptr;
}

void iti::http::router::Mux::handle(const std::string &pattern,
                                    std::shared_ptr<iti::http::IHandler> h) {
	handle_impl(Method::ALL, pattern, h);
}

void iti::http::router::Mux::handle_func(const std::string &pattern,
                                         iti::http::IHandler::handlerFunc h) {
	handle_impl(Method::ALL, pattern,
	            IHandler::make_handler(new BasicHandler(std::move(h))));
}

void iti::http::router::Mux::method(iti::http::Method method,
                                    const std::string &pattern,
                                    std::shared_ptr<iti::http::IHandler> h) {

	handle_impl(method, pattern, h);
}

void iti::http::router::Mux::method_func(iti::http::Method method,
                                         const std::string &pattern,
                                         iti::http::IHandler::handlerFunc h) {
	handle_impl(method, pattern,
	            IHandler::make_handler(new BasicHandler(std::move(h))));
}

void iti::http::router::Mux::connect(const std::string &pattern,
                                     iti::http::IHandler::handlerFunc h) {
	handle_impl(Method::CONNECT, pattern,
	            IHandler::make_handler(new BasicHandler(std::move(h))));
}
void iti::http::router::Mux::del(const std::string &pattern,
                                 iti::http::IHandler::handlerFunc h) {
	handle_impl(Method::DEL, pattern,
	            IHandler::make_handler(new BasicHandler(std::move(h))));
}
void iti::http::router::Mux::get(const std::string &pattern,
                                 iti::http::IHandler::handlerFunc h) {
	handle_impl(Method::GET, pattern,
	            IHandler::make_handler(new BasicHandler(std::move(h))));
}
void iti::http::router::Mux::head(const std::string &pattern,
                                  iti::http::IHandler::handlerFunc h) {
	handle_impl(Method::HEAD, pattern,
	            IHandler::make_handler(new BasicHandler(std::move(h))));
}
void iti::http::router::Mux::options(const std::string &pattern,
                                     iti::http::IHandler::handlerFunc h) {
	handle_impl(Method::OPTIONS, pattern,
	            IHandler::make_handler(new BasicHandler(std::move(h))));
}
void iti::http::router::Mux::patch(const std::string &pattern,
                                   iti::http::IHandler::handlerFunc h) {
	handle_impl(Method::PATCH, pattern,
	            IHandler::make_handler(new BasicHandler(std::move(h))));
}
void iti::http::router::Mux::post(const std::string &pattern,
                                  iti::http::IHandler::handlerFunc h) {
	handle_impl(Method::POST, pattern,
	            IHandler::make_handler(new BasicHandler(std::move(h))));
}
void iti::http::router::Mux::put(const std::string &pattern,
                                 iti::http::IHandler::handlerFunc h) {
	handle_impl(Method::PUT, pattern,
	            IHandler::make_handler(new BasicHandler(std::move(h))));
}
void iti::http::router::Mux::trace(const std::string &pattern,
                                   iti::http::IHandler::handlerFunc h) {
	handle_impl(Method::TRACE, pattern,
	            IHandler::make_handler(new BasicHandler(std::move(h))));
}

void iti::http::router::Mux::set_not_found(iti::http::IHandler::handlerFunc h) {

	// if we got nothing then reset to the "default" 404 handler
	if (h == nullptr) {
		h = not_found_default_handler;
	}

	auto hFn = IHandler::make_handler(new BasicHandler(std::move(h)));

	set_not_found(hFn);
}
void iti::http::router::Mux::set_not_found(
    std::shared_ptr<iti::http::IHandler> h) {

	std::shared_ptr<Mux> m = shared_from_this();

	// if we got nothing then reset to the "default" 404 handler
	if (h == nullptr) {
		h = IHandler::make_handler(new BasicHandler(not_found_default_handler));
	}

	if (isInline && parent != nullptr) {
		m = parent;
		Middlewares mws;
		mws.collection = middlewares;

		h = mws.make_handler(h);
	}

	// Update the notFoundHandler from this point forward
	m->notFoundHandler = h;
	m->update_subroutes([&](Mux &subMux) {
		if (subMux.notFoundHandler == nullptr) {
			subMux.set_not_found(h);
		}
	});
}

void iti::http::router::Mux::set_method_not_allowed(
    iti::http::IHandler::handlerFunc h) {

	// if we got nothing then reset to the "default" 405 handler
	if (h == nullptr) {
		h = method_not_allowed_default_handler;
	}

	auto hFn = IHandler::make_handler(new BasicHandler(std::move(h)));

	set_method_not_allowed(hFn);
}

void iti::http::router::Mux::set_method_not_allowed(
    std::shared_ptr<iti::http::IHandler> h) {

	auto m = shared_from_this();

	// if we got nothing then reset to the "default" 405 handler
	if (h == nullptr) {
		h = IHandler::make_handler(
		    new BasicHandler(method_not_allowed_default_handler));
	}

	if (isInline && parent != nullptr) {
		m = parent;
		Middlewares mws;
		mws.collection = middlewares;

		h = mws.make_handler(h);
	}

	// Update the methodNotAllowedHandler from this point forward
	m->notFoundHandler = h;
	m->update_subroutes([&](Mux &subMux) {
		if (subMux.methodNotAllowedHandler == nullptr) {
			subMux.set_method_not_allowed(h);
		}
	});
}

std::shared_ptr<iti::http::router::Node>
iti::http::router::Mux::handle_impl(iti::http::Method method,
                                    const std::string &pattern,
                                    std::shared_ptr<iti::http::IHandler> h) {
	if (pattern.empty() || pattern[0] != '/') {
		throw std::logic_error(fmt::format(
		    "mux: routing pattern must begin with '/' in '{}'", pattern));
	}

	// Build the computed routing handler for this routing pattern.
	if (!isInline && handler == nullptr) {
		update_route_handler();
	}
	// Build endpoint handler with inline middlewares for the route
	std::shared_ptr<iti::http::IHandler> chainedHandler;
	if (isInline) {
		handler = route_http();
		Middlewares mws;
		mws.collection = middlewares;
		chainedHandler = mws.make_handler(h);
	} else {
		chainedHandler = h;
	}

	// Add the endpoint to the tree and return the node
	return tree->insert_route(method, pattern, chainedHandler);
}

std::shared_ptr<IHandler> iti::http::router::Mux::route_http() {
	auto func = [mx = this](const Request &req, Response &resp) {
		// Grab the route context object
		std::shared_ptr<RoutingContext> rctx = nullptr;
		if (!req.context.try_get_value(RoutingContext::routeCtxKey, rctx) ||
		    rctx == nullptr) {
			throw std::exception(
			    "mux: do not have RoutingContext when we should");
		}

		// The request routing path
		auto routePath = rctx->routePath;
		if (routePath.empty()) {
			routePath = req.url.path;
			if (routePath.empty()) {
				routePath = "/";
			}
		}

		if (rctx->routeMethod == Method::unknown) {
			rctx->routeMethod = req.method;
		}

		if (!rctx->routeMethod.is_valid()) {
			mx->method_not_allowed_handler(req, resp);
			return;
		}

		// Find the route
		auto result = mx->tree->find_route(rctx, rctx->routeMethod, routePath);
		std::shared_ptr<IHandler> h = std::get<2>(result);
		if (h != nullptr) {
			h->handle_request(req, resp);
			return;
		}

		if (rctx->get_method_not_allowed_hint()) {
			mx->method_not_allowed_handler(req, resp);
		}

		mx->not_found_handler(req, resp);
	};

	return IHandler::make_handler(new BasicHandler(func));
}

// method_not_allowed_handler returns the default Mux 405 responder whenever
// a method cannot be resolved for a route.
void iti::http::router::Mux::method_not_allowed_handler(const Request &req,
                                                        Response &resp) {
	if (methodNotAllowedHandler == nullptr) {
		method_not_allowed_default_handler(req, resp);
		return;
	}

	methodNotAllowedHandler->handle_request(req, resp);
}

// not_found_handler returns the default Mux 404 responder whenever a route
// cannot be found.
void iti::http::router::Mux::not_found_handler(const Request &req,
                                               Response &resp) {
	if (notFoundHandler == nullptr) {
		not_found_default_handler(req, resp);
		return;
	}

	notFoundHandler->handle_request(req, resp);
}

// update_route_handler builds the single mux handler that is a chain of the
// middleware stack, as defined by calls to Use(), and the tree router (Mux)
// itself. After this point, no other middlewares can be registered on this
// Mux's stack. But you can still compose additional middlewares via Group()'s
// or using a chained middleware handler.
void iti::http::router::Mux::update_route_handler() {
	handler = chain_helper(middlewares, route_http());
}

void iti::http::router::Mux::update_subroutes(
    std::function<void(Mux &subMux)> fn) {
	for (auto &r : tree->get_routes()) {
		Mux *sm = dynamic_cast<Mux *>(r.subroutes.get());

		if (sm == nullptr) {
			continue;
		}

		fn(*sm);
	}
}

std::string
iti::http::router::Mux::next_route_path(std::shared_ptr<RoutingContext> rctx) {
	std::string routePath{"/"};

	// index of last param in list
	auto routeParams = rctx->get_route_params();
	long long nx     = (long long)routeParams.keys.size() - 1;
	if (nx >= 0 && routeParams.keys[nx] == "*" &&
	    routeParams.values.size() > nx) {
		routePath = "/" + routeParams.values[nx];
	}
	return routePath;
}

#endif // ITI_LIB_HTTP_ROUTER_MUX_CPP
