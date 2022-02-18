#ifndef ITI_LIB_HTTP_ROUTER_H
#define ITI_LIB_HTTP_ROUTER_H

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "context.h"
#include "http.h"
#include "router.context.h"

namespace iti {
namespace http {
namespace router {

using middleware =
    std::function<std::shared_ptr<IHandler>(std::shared_ptr<IHandler>)>;

class IRoutes;
class Middlewares;

// Route describes the details of a routing handler.
// Handlers map key is an HTTP method
class Route {
  public:
	std::shared_ptr<IRoutes> subroutes;
	std::unordered_map<std::string, std::shared_ptr<iti::http::IHandler>>
	    handlers;
	std::string pattern;
};

class IRoutes {
  public:
	// Routes returns the routing tree in an easily traversable structure.
	virtual std::vector<Route> get_routes() = 0;

	// Middlewares returns the list of middlewares in use by the router.
	virtual iti::http::router::Middlewares get_middlewares() = 0;

	// Match searches the routing tree for a handler that matches
	// the method/path - similar to routing a http request, but without
	// executing the handler thereafter.
	virtual bool match(std::shared_ptr<iti::http::router::RoutingContext> rctx,
	                   const std::string &method, const std::string &path) = 0;
};

class Middlewares {
  public:
	std::vector<iti::http::router::middleware> collection;

	middleware operator[](size_t idx) {
		middleware m = nullptr;

		try {
			m = collection.at(idx);
		} catch (const std::out_of_range &) {
		}

		return m;
	}

	std::shared_ptr<iti::http::IHandler>
	make_handler(std::shared_ptr<iti::http::IHandler> h);
};

using walkFunc =
    std::function<void(const std::string &method, const std::string &route,
                       std::shared_ptr<iti::http::IHandler> handler,
                       iti::http::router::Middlewares middlewares)>;

void walk_routes(std::shared_ptr<IRoutes> routes, walkFunc walkFn);

class ChainHandler : public iti::http::IHandler {
  public:
	std::shared_ptr<iti::http::IHandler> endpoint;
	std::shared_ptr<iti::http::IHandler> chain;
	iti::http::router::Middlewares middlewares;

	void handle_request(const Request &req, Response &resp);
};

class IRouter : public iti::http::IHandler, public IRoutes {
  public:
	// use appends one or more middlewares onto the Router stack.
	virtual void use(iti::http::router::middleware middleware) = 0;
	virtual void
	use(const std::vector<iti::http::router::middleware> &middlewares) = 0;

	// with adds inline middlewares for an endpoint handler.
	virtual std::shared_ptr<IRouter>
	with(iti::http::router::middleware middleware) = 0;
	virtual std::shared_ptr<IRouter>
	with(const std::vector<iti::http::router::middleware> &middlewares) = 0;

	// group adds a new inline-Rohandle_requestuter along the current routing
	// path, with a fresh middleware stack for the inline-Router.
	virtual std::shared_ptr<IRouter>
	group(std::function<void(std::shared_ptr<IRouter> r)> fn) = 0;

	// route mounts a sub-Router along a `pattern`` string.
	virtual std::shared_ptr<IRouter>
	route(const std::string &pattern,
	      std::function<void(std::shared_ptr<IRouter> r)> fn) = 0;

	// mount attaches another IRouter along ./pattern/*
	virtual void mount(const std::string &pattern,
	                   std::shared_ptr<iti::http::router::IRouter> r) = 0;

	// handle and handle_func adds routes for `pattern` that matches
	// all HTTP methods.
	virtual void handle(const std::string &pattern,
	                    std::shared_ptr<iti::http::IHandler> h)  = 0;
	virtual void handle_func(const std::string &pattern,
	                         iti::http::IHandler::handlerFunc h) = 0;

	// method and method_func adds routes for `pattern` that matches
	// the `method` HTTP method.
	virtual void method(iti::http::Method method, const std::string &pattern,
	                    std::shared_ptr<iti::http::IHandler> h)  = 0;
	virtual void method_func(iti::http::Method method,
	                         const std::string &pattern,
	                         iti::http::IHandler::handlerFunc h) = 0;

	// HTTP-method routing along `pattern`
	virtual void connect(const std::string &pattern,
	                     iti::http::IHandler::handlerFunc h) = 0;
	virtual void del(const std::string &pattern,
	                 iti::http::IHandler::handlerFunc h)     = 0;
	virtual void get(const std::string &pattern,
	                 iti::http::IHandler::handlerFunc h)     = 0;
	virtual void head(const std::string &pattern,
	                  iti::http::IHandler::handlerFunc h)    = 0;
	virtual void options(const std::string &pattern,
	                     iti::http::IHandler::handlerFunc h) = 0;
	virtual void patch(const std::string &pattern,
	                   iti::http::IHandler::handlerFunc h)   = 0;
	virtual void post(const std::string &pattern,
	                  iti::http::IHandler::handlerFunc h)    = 0;
	virtual void put(const std::string &pattern,
	                 iti::http::IHandler::handlerFunc h)     = 0;
	virtual void trace(const std::string &pattern,
	                   iti::http::IHandler::handlerFunc h)   = 0;

	// set_not_found defines a handler to respond whenever a route could
	// not be found.
	virtual void set_not_found(iti::http::IHandler::handlerFunc h)     = 0;
	virtual void set_not_found(std::shared_ptr<iti::http::IHandler> h) = 0;

	// set_method_not_allowed defines a handler to respond whenever a method is
	// not allowed.
	virtual void set_method_not_allowed(iti::http::IHandler::handlerFunc h) = 0;
	virtual void
	set_method_not_allowed(std::shared_ptr<iti::http::IHandler> h) = 0;
};

} // namespace router
} // namespace http
} // namespace iti

#endif // ITI_LIB_HTTP_ROUTER_H
