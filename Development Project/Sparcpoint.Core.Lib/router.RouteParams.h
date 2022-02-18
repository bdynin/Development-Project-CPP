#ifndef ITI_LIB_HTTP_ROUTER_ROUTEPARAMS_H
#define ITI_LIB_HTTP_ROUTER_ROUTEPARAMS_H

#include <string>
#include <vector>

namespace iti {
namespace http {
namespace router {

// RouteParams is a structure to track URL routing parameters efficiently.
class RouteParams {
  public:
	std::vector<std::string> keys;
	std::vector<std::string> values;

	void add(const std::string &key, const std::string &value);
	bool try_get(const std::string &key, std::string &value);
	std::string get(const std::string &key);
};

} // namespace router
} // namespace http
} // namespace iti

#endif // ITI_LIB_HTTP_ROUTER_ROUTEPARAMS_H
