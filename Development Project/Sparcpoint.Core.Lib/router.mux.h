#ifndef ITI_LIB_HTTP_ROUTER_MUX_H
#define ITI_LIB_HTTP_ROUTER_MUX_H

#include "pch.h"

#include <memory>
#include <string>
#include <vector>

#include "http.h"
#include "router.tree.h"

namespace iti {
namespace http {
namespace router {
class Mux : public IRouter, public std::enable_shared_from_this<Mux> {
  public:
	void handle_request(const Request &req, Response &resp);

	// Routes returns the routing tree in an easily traversable structure.
	std::vector<Route> get_routes();

	// Middlewares returns the list of middlewares in use by the router.
	Middlewares get_middlewares();

	// Match searches the routing tree for a handler that matches
	// the method/path - similar to routing a http request, but without
	// executing the handler thereafter.
	bool match(std::shared_ptr<RoutingContext> rctx, const std::string &method,
	           const std::string &path);

	void use(middleware middleware) override;
	void use(const std::vector<middleware> &middlewares) override;

	std::shared_ptr<IRouter> with(middleware middleware);
	std::shared_ptr<IRouter> with(const std::vector<middleware> &middlewares);

	std::shared_ptr<IRouter>
	group(std::function<void(std::shared_ptr<IRouter> r)> fn);

	std::shared_ptr<IRouter>
	route(const std::string &pattern,
	      std::function<void(std::shared_ptr<IRouter> r)> fn);

	void mount(const std::string &pattern, std::shared_ptr<IRouter> r);

	void handle(const std::string &pattern,
	            std::shared_ptr<iti::http::IHandler> h) override;
	void handle_func(const std::string &pattern,
	                 iti::http::IHandler::handlerFunc h) override;

	void method(iti::http::Method method, const std::string &pattern,
	            std::shared_ptr<iti::http::IHandler> h) override;
	void method_func(iti::http::Method method, const std::string &pattern,
	                 iti::http::IHandler::handlerFunc h) override;

	void connect(const std::string &pattern,
	             iti::http::IHandler::handlerFunc h) override;
	void del(const std::string &pattern,
	         iti::http::IHandler::handlerFunc h) override;
	void get(const std::string &pattern,
	         iti::http::IHandler::handlerFunc h) override;
	void head(const std::string &pattern,
	          iti::http::IHandler::handlerFunc h) override;
	void options(const std::string &pattern,
	             iti::http::IHandler::handlerFunc h) override;
	void patch(const std::string &pattern,
	           iti::http::IHandler::handlerFunc h) override;
	void post(const std::string &pattern,
	          iti::http::IHandler::handlerFunc h) override;
	void put(const std::string &pattern,
	         iti::http::IHandler::handlerFunc h) override;
	void trace(const std::string &pattern,
	           iti::http::IHandler::handlerFunc h) override;

	void set_not_found(iti::http::IHandler::handlerFunc h) override;
	void set_not_found(std::shared_ptr<iti::http::IHandler> h) override;
	void set_method_not_allowed(iti::http::IHandler::handlerFunc h) override;
	void
	set_method_not_allowed(std::shared_ptr<iti::http::IHandler> h) override;

	void method_not_allowed_handler(const Request &req, Response &resp);
	void not_found_handler(const Request &req, Response &resp);

  private:
	std::shared_ptr<Node> handle_impl(iti::http::Method method,
	                                  const std::string &pattern,
	                                  std::shared_ptr<iti::http::IHandler> h);

	std::shared_ptr<IHandler> route_http();

	void update_route_handler();

	void update_subroutes(std::function<void(Mux &subMux)> fn);

	std::string next_route_path(std::shared_ptr<RoutingContext> rctx);

	// The computed mux handler made of the chained middleware stack and
	// the tree router
	std::shared_ptr<iti::http::IHandler> handler = nullptr;

	// The radix trie router
	std::shared_ptr<Node> tree = std::make_shared<Node>();

	// Custom method not allowed handler
	std::shared_ptr<iti::http::IHandler> methodNotAllowedHandler = nullptr;

	// Custom route not found handler
	std::shared_ptr<iti::http::IHandler> notFoundHandler = nullptr;

	// Controls the behaviour of middleware chain generation when a mux
	// is registered as an inline group inside another mux.
	std::shared_ptr<Mux> parent = nullptr;

	// The middleware stack
	std::vector<middleware> middlewares;

	bool isInline = false;
};

} // namespace router
} // namespace http
} // namespace iti

#endif // ITI_LIB_HTTP_ROUTER_MUX_H
