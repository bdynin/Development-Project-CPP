#ifndef ITI_LIB_HTTP_ROUTER_TREE_H
#define ITI_LIB_HTTP_ROUTER_TREE_H

#include "pch.h"

#include <algorithm>
#include <array>
#include <memory>
#include <numeric>
#include <regex>
#include <unordered_map>

#include "router.context.h"

#include "StrUtils.h"
#include "http.h"
#include "router.h"

namespace iti {
namespace http {
namespace router {

class Endpoint {
  public:
	// endpoint handler
	std::shared_ptr<iti::http::IHandler> handler;

	// pattern is the routing pattern for handler nodes
	std::string pattern;

	// parameter keys recorded on handler nodes
	std::vector<std::string> paramKeys;
};

// endpoints is a mapping of http method constants to handlers
// for a given route.
class Endpoints {
  public:
	std::unordered_map<iti::http::Method, std::shared_ptr<Endpoint>> collection;

	std::shared_ptr<Endpoint> operator[](const iti::http::Method method) const {
		std::shared_ptr<Endpoint> ep = nullptr;

		try {
			ep = collection.at(method);
		} catch (const std::out_of_range &) {
		}

		return ep;
	}

	std::shared_ptr<Endpoint>
	operator[](iti::http::Method::Value mValue) const {
		std::shared_ptr<Endpoint> ep = nullptr;

		try {
			ep = collection.at(iti::http::Method(mValue));
		} catch (const std::out_of_range &) {
		}

		return ep;
	}

	std::shared_ptr<Endpoint> Value(http::Method method) {
		std::shared_ptr<Endpoint> ep;

		try {
			ep = collection.at(method);
		} catch (const std::out_of_range &) {
			ep = std::make_shared<Endpoint>();

			collection[method] = ep;
		}

		return ep;
	}
};

enum class NodeType : uint8_t {
	Static,   // /home
	Regexp,   // /{id:[0-9]+}
	Param,    // {category}
	CatchAll, // /api/v1/*
};

class Node : public std::enable_shared_from_this<Node> {
  public:
	// subroutes on the leaf node
	std::shared_ptr<IRoutes> subroutes;

	// regexp matcher for regexp nodes
	std::unique_ptr<std::regex> rex = nullptr;

	// HTTP handler endpoints on the leaf node
	Endpoints endpoints;

	// prefix is the common prefix we ignore
	std::string prefix;

	// child nodes should be stored in-order for iteration,
	// in groups of the node type.
	std::array<std::vector<std::shared_ptr<Node>>, 4> children;

	// first byte of the child prefix
	char tail{};

	// node type: static, regexp, param, catchAll
	NodeType typ{};

	// first byte of the prefix
	char label{};

	std::shared_ptr<Node> insert_route(http::Method method,
	                                   const std::string &pattern,
	                                   std::shared_ptr<http::IHandler> handler);

	std::tuple<std::shared_ptr<Node>, Endpoints,
	           std::shared_ptr<http::IHandler>>
	find_route(std::shared_ptr<RoutingContext> rctx, iti::http::Method method,
	           const std::string &path);

	std::vector<iti::http::router::Route> get_routes();

	bool find_pattern(const std::string &pattern);

  private:
	// add_child appends the new `child` node to the tree using the `pattern` as
	// the trie key. For a URL router like chi's, we split the static, param,
	// regexp and wildcard segments into different nodes. In addition, add_child
	// will recursively call itself until every pattern segment is added to the
	// url pattern tree as individual nodes, depending on type.
	std::shared_ptr<Node> add_child(std::shared_ptr<Node> child,
	                                const std::string &prefix);

	void replace_child(char label, char tail, std::shared_ptr<Node> child);

	std::shared_ptr<Node> get_edge(NodeType ntyp, char label, char tail,
	                               const std::string &prefix);

	void set_endpoint(http::Method method,
	                  std::shared_ptr<http::IHandler> handler,
	                  std::string pattern);

	// Recursive edge traversal by checking all nodeTyp groups along the way.
	// It's like searching through a multi-dimensional radix trie.
	std::shared_ptr<Node>
	find_route_helper(std::shared_ptr<RoutingContext> rctx, http::Method method,
	                  const std::string &path);

	std::shared_ptr<Node> find_edge(NodeType ntyp, char label);

	bool is_leaf();

	bool walk(std::function<bool(const Endpoints &, std::shared_ptr<IRoutes>)>);
};

} // namespace router
} // namespace http
} // namespace iti

#endif ITI_LIB_HTTP_ROUTER_TREE_H
