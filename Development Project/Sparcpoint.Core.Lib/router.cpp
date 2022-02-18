#ifndef ITI_LIB_HTTP_ROUTER_CPP
#define ITI_LIB_HTTP_ROUTER_CPP

#include "pch.h"

#include "router.h"

using namespace iti;

using iti::http::IHandler;
using iti::http::router::ChainHandler;
using iti::http::router::IRoutes;
using iti::http::router::middleware;
using iti::http::router::walkFunc;

// helpers
// ----------------------------------------------------------------------------

std::shared_ptr<IHandler> chain_helper(std::vector<middleware> middlewares,
                                       std::shared_ptr<IHandler> endpoint) {
	// Return ahead of time if there aren't any middlewares for the chain
	if (middlewares.empty()) {
		return endpoint;
	}

	// Wrap the end handler with the middleware chain
	auto h = middlewares[middlewares.size() - 1](endpoint);
	for (long long i = (long long)middlewares.size() - 2; i >= 0; i--) {
		h = middlewares[i](h);
	}

	return h;
}

void walk_impl(std::shared_ptr<IRoutes> r, walkFunc walkFn,
               const std::string &parentRoute,
               const iti::http::router::Middlewares &parentMw =
                   iti::http::router::Middlewares()) {

	auto currentRoutes = r->get_routes();
	for (const auto &route : currentRoutes) {
		iti::http::router::Middlewares mws{parentMw};
		{
			auto rMiddlewares = r->get_middlewares();
			mws.collection.insert(mws.collection.end(),
			                      rMiddlewares.collection.begin(),
			                      rMiddlewares.collection.end());
		}

		if (route.subroutes != nullptr) {
			walk_impl(route.subroutes, walkFn, parentRoute + route.pattern,
			          mws);
			continue;
		}

		for (auto &[method, handler] : route.handlers) {

			if (method == "*") {
				// Ignore a "catchAll" method, since we pass down
				// all the specific methods for each route.
				continue;
			}

			std::string fullRoute =
			    strutils::replace_copy(parentRoute + route.pattern, "/*/", "/");

			ChainHandler *chain = dynamic_cast<ChainHandler *>(handler.get());

			if (chain != nullptr) {
				auto chainMiddlewares = chain->middlewares;

				mws.collection.insert(mws.collection.end(),
				                      chainMiddlewares.collection.begin(),
				                      chainMiddlewares.collection.end());

				walkFn(method, fullRoute, chain->endpoint, mws);
			} else {
				walkFn(method, fullRoute, handler, mws);
			}
		}
	}
}

void iti::http::router::walk_routes(std::shared_ptr<IRoutes> routes,
                                    walkFunc walkFn) {

	return walk_impl(routes, walkFn, "");
}

// middlewares
// ----------------------------------------------------------------------------
std::shared_ptr<iti::http::IHandler>
iti::http::router::Middlewares::make_handler(
    std::shared_ptr<iti::http::IHandler> h) {
	ChainHandler *c = new ChainHandler();
	c->endpoint     = h;
	c->chain        = chain_helper(collection, h);
	c->middlewares  = iti::http::router::Middlewares(*this);

	return IHandler::make_handler(c);
}

// chain handler
// ----------------------------------------------------------------------------
void iti::http::router::ChainHandler::handle_request(const Request &req,
                                                     Response &resp) {
	if (chain != nullptr) {
		return chain->handle_request(req, resp);
	}
}

#endif ITI_LIB_HTTP_ROUTER_CPP
