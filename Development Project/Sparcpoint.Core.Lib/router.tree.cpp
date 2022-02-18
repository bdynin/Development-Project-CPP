#ifndef ITI_LIB_HTTP_ROUTER_TREE_CPP
#define ITI_LIB_HTTP_ROUTER_TREE_CPP

#include "pch.h"

#include "router.tree.h"

#include <algorithm>

using namespace iti;
using iti::http::IHandler;
using iti::http::router::Endpoints;
using iti::http::router::IRoutes;
using iti::http::router::Node;
using iti::http::router::NodeType;
using iti::http::router::Route;
using iti::http::router::RouteParams;
using iti::http::router::RoutingContext;

// helpers
// ----------------------------------------------------------------------------

static const constexpr std::array<http::Method, 9> methodsList = {
    http::Method::CONNECT, http::Method::DEL,     http::Method::GET,
    http::Method::HEAD,    http::Method::OPTIONS, http::Method::PATCH,
    http::Method::POST,    http::Method::PUT,     http::Method::TRACE,
};

// pat_next_segment returns the next segment details from a pattern:
struct NextSegmentResult {
  public:
	NodeType nodeType{};
	std::string regexPattern;
	std::string paramKey;
	char paramTail{};
	size_t paramStartingIdx{};
	size_t paramEndingIdx{};

	NextSegmentResult()  = default;
	~NextSegmentResult() = default;

	NextSegmentResult(NodeType nt, const std::string &pKey,
	                  const std::string &regex, char pTail, size_t pStartIdx,
	                  size_t pEndIdx)
	    : nodeType(nt), paramKey(pKey), regexPattern(regex), paramTail(pTail),
	      paramStartingIdx(pStartIdx), paramEndingIdx(pEndIdx) {}
};

NextSegmentResult pat_next_segment(const std::string &pattern) {
	auto ps = pattern.find('{');
	auto ws = pattern.find('*');

	if (ps == std::string::npos && ws == std::string::npos) {
		return NextSegmentResult(NodeType::Static, "", "", 0, 0,
		                         pattern.size()); // we return the entire thing
	}

	// Sanity check
	if (ps != std::string::npos && ws != std::string::npos && ws < ps) {
		throw std::logic_error(
		    "router: wildcard '*' must be the last pattern in a "
		    "route, otherwise use a '{param}'");
	}

	char tail = '/'; // Default endpoint tail to '/'

	if (ps != std::string::npos) {
		// Param/Regexp pattern is next
		NodeType nt = NodeType::Param;

		// Read to closing } taking into account opens and closes in curl count
		// (cc)
		size_t cc = 0;
		size_t pe = ps;

		std::string_view subPattern{pattern};
		subPattern = subPattern.substr(ps);
		for (size_t i = 0; i < subPattern.size(); i++) {
			char c = subPattern[i];
			if (c == '{') {
				cc++;
			} else if (c == '}') {
				cc--;
				if (cc == 0) {
					pe = ps + i;
					break;
				}
			}
		}
		if (pe == ps) {
			throw std::logic_error(
			    "router: route param closing delimiter '}' is missing");
		}

		std::string key = pattern.substr(ps + 1, pe - ps - 1);
		pe++; // set end to next position

		if (pe < pattern.size()) {
			tail = pattern[pe];
		}

		std::string rexpat;
		if (size_t idx = key.find(':'); idx != std::string::npos) {
			nt     = NodeType::Regexp;
			rexpat = key.substr(idx + 1);
			key    = key.substr(0, idx);
		}

		if (!rexpat.empty()) {
			if (rexpat[0] != '^') {
				rexpat = "^" + rexpat;
			}
			if (rexpat[rexpat.size() - 1] != '$') {
				rexpat += "$";
			}
		}

		return NextSegmentResult(nt, key, rexpat, tail, ps, pe);
	}

	// Wildcard pattern as finale
	if (ws < pattern.size() - 1) {
		throw std::logic_error(
		    "router: wildcard '*' must be the last value in a route. trim "
		    "trailing text or use a '{param}' instead");
	}
	return NextSegmentResult(NodeType::CatchAll, "*", "", 0, ws,
	                         pattern.size());
}

std::vector<std::string> pat_param_keys(const std::string &pattern) {
	std::string pat(pattern);
	std::vector<std::string> paramKeys;

	while (true) {
		NextSegmentResult result = pat_next_segment(pat);
		if (result.nodeType == NodeType::Static) {
			return paramKeys;
		}

		auto findResult =
		    std::find(paramKeys.begin(), paramKeys.end(), result.paramKey);

		if (findResult != paramKeys.end()) {
			throw std::logic_error(fmt::format(
			    "chi: routing pattern '{}' contains duplicate param key, '{}'",
			    pattern, result.paramKey));
		}

		paramKeys.emplace_back(std::move(result.paramKey));
		pat = pat.substr(result.paramEndingIdx);
	}
}

// longest_prefix finds the length of the shared prefix
// of two strings
size_t longest_prefix(const std::string &k1, const std::string &k2) {
	size_t max = k1.size();
	size_t i   = k2.size();
	if (i < max) {
		max = i;
	}

	i = 0;
	for (; i < max; i++) {
		if (k1[i] != k2[i]) {
			break;
		}
	}
	return i;
}

std::shared_ptr<Node>
nodes_find_edge(char label, const std::vector<std::shared_ptr<Node>> &nodes) {
	size_t num    = nodes.size();
	long long idx = 0;

	long long i = 0;
	long long j = (long long)num - 1;
	while (i <= j) {
		idx = i + (j - i) / 2;
		if (label > nodes[idx]->label) {
			i = idx + 1;
		} else if (label < nodes[idx]->label) {
			j = idx - 1;
		} else {
			i = (long long)num; // breaks cond
		}
	}

	if (nodes[idx]->label != label) {
		return nullptr;
	}

	return nodes[idx];
}

void nodes_sort(std::vector<std::shared_ptr<Node>> &nodes) {
	// remove any `nullptr`s
	nodes.erase(
	    std::remove_if(nodes.begin(), nodes.end(),
	                   [](std::shared_ptr<Node> n) { return n == nullptr; }),
	    nodes.end());

	// sort the nodes
	std::sort(nodes.begin(), nodes.end(),
	          [](std::shared_ptr<Node> a, std::shared_ptr<Node> b) {
		          return a->label < b->label;
	          });

	// tail sort
	for (auto it = nodes.rbegin(); it != nodes.rend(); it++) {
		if (nodes.size() > 1 && (*it)->typ > NodeType::Static &&
		    (*it)->tail == '/') {
			std::iter_swap(it, nodes.end() - 1);
		}
	}
}

// router tree
// ----------------------------------------------------------------------------

std::shared_ptr<Node>
iti::http::router::Node::insert_route(http::Method method,
                                      const std::string &pattern,
                                      std::shared_ptr<http::IHandler> handler) {

	auto n = shared_from_this();

	std::shared_ptr<Node> parent = nullptr;
	std::string search{pattern};

	while (true) {
		// Handle key exhaustion
		if (search.empty()) { // Insert or update the node's leaf handler
			n->set_endpoint(method, handler, pattern);
			return n;
		}

		// We're going to be searching for a wild node next,
		// in this case, we need to get the tail
		char label = search[0];
		NextSegmentResult result;
		if (label == '{' || label == '*') {
			result = pat_next_segment(search);
		}

		std::string prefix;
		if (result.nodeType == NodeType::Regexp) {
			prefix = result.regexPattern;
		}

		// Look for the edge to attach to
		parent = n;
		n      = n->get_edge(result.nodeType, label, result.paramTail, prefix);

		// No edge, create one
		if (n == nullptr) {
			auto child = std::make_shared<Node>();

			child->label  = label;
			child->tail   = result.paramTail;
			child->prefix = search;

			auto hn = parent->add_child(child, search);
			hn->set_endpoint(method, handler, pattern);

			return hn;
		}

		// Found an edge to match the pattern
		if (n->typ > NodeType::Static) {
			// We found a param node, trim the param from the search path
			// and continue. This param/wild pattern segment would already
			// be on the tree from a previous call to add_child when
			// creating a new node.
			search = search.substr(result.paramEndingIdx);
			continue;
		}

		// Static nodes fall below here.
		// Determine longest prefix of the search key on match.
		size_t commonPrefix = longest_prefix(search, n->prefix);
		if (commonPrefix ==
		    n->prefix.size()) { // the common prefix is as long as the
			// current node's prefix we're attempting
			// to insert. keep the search going.
			search = search.substr(commonPrefix);
			continue;
		}

		// Split the node
		auto child    = std::make_shared<Node>();
		child->typ    = NodeType::Static;
		child->prefix = search.substr(0, commonPrefix);
		parent->replace_child(search[0], result.paramTail, child);

		// Restore the existing node
		n->label  = n->prefix[commonPrefix];
		n->prefix = n->prefix.substr(commonPrefix);
		child->add_child(n, n->prefix);

		// If the new key is a subset, set the method/handler on this node
		// and finish.
		search = search.substr(commonPrefix);
		if (search.empty()) {
			child->set_endpoint(method, handler, pattern);
			return child;
		}

		// Create a new edge for the node
		auto subchild    = std::make_shared<Node>();
		subchild->typ    = NodeType::Static;
		subchild->label  = search[0];
		subchild->prefix = search;

		auto hn = child->add_child(subchild, search);
		hn->set_endpoint(method, handler, pattern);
		return hn;
	}
}

std::tuple<std::shared_ptr<Node>, Endpoints, std::shared_ptr<http::IHandler>>
iti::http::router::Node::find_route(std::shared_ptr<RoutingContext> rctx,
                                    iti::http::Method method,
                                    const std::string &path) {

	// Reset the context routing pattern and params
	rctx->routePattern.clear();
	rctx->routeParams.keys.clear();
	rctx->routeParams.values.clear();

	// Find the routing handlers for the path
	auto rn = find_route_helper(rctx, method, path);
	if (rn == nullptr) {
		return std::make_tuple(nullptr, Endpoints(), nullptr);
	}

	// Record the routing params in the request lifecycle
	rctx->urlParams.keys.insert(rctx->urlParams.keys.end(),
	                            rctx->routeParams.keys.begin(),
	                            rctx->routeParams.keys.end());

	rctx->urlParams.values.insert(rctx->urlParams.values.end(),
	                              rctx->routeParams.values.begin(),
	                              rctx->routeParams.values.end());

	// Record the routing pattern in the request lifecycle
	auto &eps = rn->endpoints.collection[method()];
	if (!(eps->pattern.empty())) {
		rctx->routePattern = eps->pattern;

		rctx->routePatterns.emplace_back(rctx->routePattern);
	}

	return std::make_tuple(rn, rn->endpoints, eps->handler);
}

std::shared_ptr<Node>
iti::http::router::Node::add_child(std::shared_ptr<Node> child,
                                   const std::string &prefix) {
	std::string search = prefix;

	// handler leaf node added to the tree is the child.
	// this may be overridden later down the flow
	auto hn = child;

	// Parse next segment
	auto result = pat_next_segment(search);

	// Add child depending on next up segment
	switch (result.nodeType) {

	case NodeType::Static:
		// Search prefix is all static (that is, has no params in path)
		// noop
		break;

	default:
		// Search prefix contains a param, regexp or wildcard
		if (result.nodeType == NodeType::Regexp) {
			std::regex rex(result.regexPattern);
			child->prefix = result.regexPattern;
			child->rex    = std::make_unique<std::regex>(rex);
		}

		if (result.paramStartingIdx == 0) {
			// Route starts with a param
			child->typ = result.nodeType;

			if (result.nodeType == NodeType::CatchAll) {
				result.paramStartingIdx = std::string::npos;
			} else {
				result.paramStartingIdx = result.paramEndingIdx;
			}
			if (result.paramStartingIdx == std::string::npos) {
				result.paramStartingIdx = search.size();
			}
			child->tail = result.paramTail; // for params, we set the tail

			if (result.paramStartingIdx != search.size()) {
				// add static edge for the remaining part, split the
				// end. its not possible to have adjacent param
				// nodes, so its certainly going to be a static node
				// next.

				search = search.substr(
				    result.paramStartingIdx); // advance search position

				auto nn    = std::make_shared<Node>();
				nn->typ    = NodeType::Static;
				nn->label  = search[0];
				nn->prefix = search;
				hn         = child->add_child(nn, search);
			}
		} else if (result.paramStartingIdx != std::string::npos) {
			// Route has some param

			// starts with a static segment
			child->typ    = NodeType::Static;
			child->prefix = search.substr(0, result.paramStartingIdx);
			child->rex    = nullptr;

			// add the param edge node
			search = search.substr(result.paramStartingIdx);

			auto nn   = std::make_shared<Node>();
			nn->typ   = result.nodeType;
			nn->label = search[0];
			nn->tail  = result.paramTail;
			hn        = child->add_child(nn, search);
		}
	}

	children[(int)(child->typ)].emplace_back(child);
	nodes_sort(children[(int)(child->typ)]);
	return hn;
}

void iti::http::router::Node::replace_child(char label, char tail,
                                            std::shared_ptr<Node> child) {
	auto nds = children[(int)(child->typ)];

	size_t childSize = nds.size();
	for (size_t i = 0; i < childSize; i++) {
		if (nds[i]->label == label && nds[i]->tail == tail) {
			children[(int)(child->typ)][i]        = child;
			children[(int)(child->typ)][i]->label = label;
			children[(int)(child->typ)][i]->tail  = tail;
			return;
		}
	}

	throw std::logic_error("router: replacing missing child");
}

std::shared_ptr<Node>
iti::http::router::Node::get_edge(NodeType ntyp, char label, char tail,
                                  const std::string &prefix) {

	auto nds = children[(int)ntyp];

	size_t childSize = nds.size();
	for (size_t i = 0; i < childSize; i++) {
		if (nds[i]->label == label && nds[i]->tail == tail) {
			if (ntyp == NodeType::Regexp && nds[i]->prefix != prefix) {
				continue;
			}
			return nds[i];
		}
	}
	return nullptr;
}

void iti::http::router::Node::set_endpoint(
    http::Method method, std::shared_ptr<http::IHandler> handler,
    std::string pattern) {

	// Set the handler for the method type on the node
	auto paramKeys = pat_param_keys(pattern);

	if ((method() & http::Method::unknown) == http::Method::unknown) {
		endpoints.Value(http::Method::unknown)->handler = handler;
	}
	if ((method() & Method::ALL) == Method::ALL) {
		auto h       = endpoints.Value(Method::ALL);
		h->handler   = handler;
		h->pattern   = pattern;
		h->paramKeys = paramKeys;

		for (const auto &m : methodsList) {
			auto h       = endpoints.Value(m);
			h->handler   = handler;
			h->pattern   = pattern;
			h->paramKeys = paramKeys;
		}
	} else {
		auto h       = endpoints.Value(method);
		h->handler   = handler;
		h->pattern   = pattern;
		h->paramKeys = paramKeys;
	}
}

std::shared_ptr<Node>
iti::http::router::Node::find_route_helper(std::shared_ptr<RoutingContext> rctx,
                                           http::Method method,
                                           const std::string &path) {
	auto nn = shared_from_this();

	std::string search = path;

	for (size_t i = 0; i < nn->children.size(); i++) {
		NodeType ntyp = (NodeType)i;
		auto nds      = nn->children[i];

		if (nds.empty()) {
			continue;
		}

		std::shared_ptr<Node> xn = nullptr;
		std::string_view xsearch(search);

		char label{};
		if (!search.empty()) {
			label = search[0];
		}

		switch (ntyp) {
		case NodeType::Static:
			xn = nodes_find_edge(label, nds);
			if (xn == nullptr || !strutils::starts_with(xsearch, xn->prefix)) {
				continue;
			}

			xsearch = xsearch.substr(xn->prefix.size());
			break;
		case NodeType::Param:
		case NodeType::Regexp:
			// short-circuit and return no matching route for empty
			// param values
			if (xsearch.empty()) {
				continue;
			}

			// serially loop through each node grouped by the tail
			// delimiter
			for (size_t i = 0; i < nds.size(); i++) {
				xn = nds[i];

				// label for param nodes is the delimiter byte
				auto p = xsearch.find(xn->tail);

				if (p == std::string_view::npos) {
					if (xn->tail == '/') {
						p = xsearch.size();
					} else {
						continue;
					}
				} else if (ntyp == NodeType::Regexp && p == 0) {
					continue;
				}

				if (ntyp == NodeType::Regexp && xn->rex != nullptr) {
					std::string s{xsearch.substr(0, p)};
					if (!std::regex_search(s, *(xn->rex.get()))) {
						continue;
					}
				} else if (xsearch.substr(0, p).find('/') !=
				           std::string_view::npos) {
					// avoid a match across path segments
					continue;
				}

				size_t prevlen = rctx->routeParams.values.size();
				rctx->routeParams.values.emplace_back(xsearch.substr(0, p));
				xsearch = xsearch.substr(p);

				if (xsearch.empty()) {
					if (xn->is_leaf()) {
						auto h = xn->endpoints[method];
						if (h != nullptr && h->handler != nullptr) {
							rctx->routeParams.keys.insert(
							    rctx->routeParams.keys.end(),
							    h->paramKeys.begin(), h->paramKeys.end());
							return xn;
						}

						// flag that the routing context found a route, but not
						// a corresponding supported method
						rctx->methodNotAllowed = true;
					}
				}

				// recursively find the next node on this branch
				auto fin =
				    xn->find_route_helper(rctx, method, std::string{xsearch});
				if (fin != nullptr) {
					return fin;
				}

				// not found on this branch, reset vars
				rctx->routeParams.values.resize(prevlen);
				xsearch = search;
			}

			rctx->routeParams.values.emplace_back("");
			break;

		default:
			// catch-all nodes
			rctx->routeParams.values.emplace_back(search);
			xn      = nds[0];
			xsearch = std::string_view();
		}

		if (xn == nullptr) {
			continue;
		}

		// did we find it yet?
		if (xsearch.empty()) {
			if (xn->is_leaf()) {
				auto h = xn->endpoints[method];
				if (h != nullptr && h->handler != nullptr) {
					rctx->routeParams.keys.insert(rctx->routeParams.keys.end(),
					                              h->paramKeys.begin(),
					                              h->paramKeys.end());
					return xn;
				}

				// flag that the routing context found a route, but not a
				// corresponding supported method
				rctx->methodNotAllowed = true;
			}
		}

		// recursively find the next node..
		auto fin = xn->find_route_helper(rctx, method, std::string{xsearch});
		if (fin != nullptr) {
			return fin;
		}

		// Did not find final handler, let's remove the param here if it was
		// set
		if (xn->typ > NodeType::Static) {
			if (!rctx->routeParams.values.empty()) {
				rctx->routeParams.values.resize(
				    rctx->routeParams.values.size() - 1);
			}
		}
	}

	return nullptr;
}

std::shared_ptr<Node> iti::http::router::Node::find_edge(NodeType ntyp,
                                                         char label) {
	auto nds      = children[(int)ntyp];
	size_t num    = nds.size();
	long long idx = 0;
	long long i   = 0;
	long long j   = num - 1;

	switch (ntyp) {
	case NodeType::Static:
	case NodeType::Param:
	case NodeType::Regexp:
		while (i <= j) {
			idx = i + (j - i) / 2;
			if (label > nds[idx]->label) {
				i = idx + 1;
			} else if (label < nds[idx]->label) {
				j = idx - 1;
			} else {
				i = num; // breaks cond
			}
		}
		if (nds[idx]->label != label) {
			return nullptr;
		}
		return nds[idx];

	default: // catch all
		return nds[idx];
	}
}

bool iti::http::router::Node::is_leaf() {
	return !endpoints.collection.empty();
}

bool iti::http::router::Node::find_pattern(const std::string &pattern) {
	auto n  = shared_from_this();
	auto nn = shared_from_this();

	for (const auto &nds : nn->children) {

		if (nds.empty()) {
			continue;
		}

		n = nn->find_edge(nds[0]->typ, pattern[0]);
		if (n == nullptr) {
			continue;
		}

		size_t idx{};
		std::string xpattern;

		switch (n->typ) {
		case NodeType::Static:
			idx = longest_prefix(pattern, n->prefix);
			if (idx < n->prefix.size()) {
				continue;
			}
			break;
		case NodeType::Param:
		case NodeType::Regexp:
			idx = pattern.find('}') + 1;
			break;
		case NodeType::CatchAll:
			idx = longest_prefix(pattern, "*");
			break;
		default:
			throw std::logic_error("find_pattern: unknown node type");
		}

		xpattern = pattern.substr(idx);
		if (xpattern.empty()) {
			return true;
		}

		return n->find_pattern(xpattern);
	}

	return false;
}

std::vector<iti::http::router::Route> iti::http::router::Node::get_routes() {
	std::vector<Route> rts;

	std::function<bool(const Endpoints &, std::shared_ptr<IRoutes>)>
	    routes_walker = [&rts](const Endpoints &eps,
	                           std::shared_ptr<IRoutes> additionalRoutes) {
		    if (eps[Method::unknown] != nullptr &&
		        eps[Method::unknown]->handler != nullptr &&
		        additionalRoutes == nullptr) {
			    return false;
		    }

		    // Group methodHandlers by unique patterns
		    std::unordered_map<std::string, Endpoints> pats;

		    for (auto &[mt, h] : eps.collection) {
			    if (h->pattern.empty()) {
				    continue;
			    }

			    pats[h->pattern][mt] = h;
		    }

		    for (auto &[p, mh] : pats) {
			    std::unordered_map<std::string, std::shared_ptr<http::IHandler>>
			        hs;

			    if (mh[Method::ALL] != nullptr &&
			        mh[Method::ALL]->handler != nullptr) {
				    hs["*"] = mh[Method::ALL]->handler;
			    }

			    for (auto &[mt, h] : mh.collection) {
				    if (h->handler == nullptr) {
					    continue;
				    }

				    try {
					    std::string m{mt.str()};
					    hs[m] = h->handler;
				    } catch (const std::logic_error) {
					    continue;
				    }
			    }

			    Route rt{};
			    rt.subroutes = additionalRoutes;
			    rt.handlers  = hs;
			    rt.pattern   = p;
			    rts.emplace_back(std::move(rt));
		    }

		    return false;
	    };

	walk(routes_walker);

	return rts;
}

bool iti::http::router::Node::walk(
    std::function<bool(const Endpoints &rps,
                       std::shared_ptr<IRoutes> additionalRoutes)>) {
	return false;
}

#endif // ITI_LIB_HTTP_ROUTER_TREE_CPP
