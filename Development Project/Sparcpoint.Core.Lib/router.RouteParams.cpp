#ifndef ITI_LIB_HTTP_ROUTER_ROUTEPARAMS_CPP
#define ITI_LIB_HTTP_ROUTER_ROUTEPARAMS_CPP

#include "pch.h"

#include "router.RouteParams.h"

// route params
// ----------------------------------------------------------------------------
void iti::http::router::RouteParams::add(const std::string &key,
                                         const std::string &value) {
	keys.emplace_back(key);
	values.emplace_back(value);
}

bool iti::http::router::RouteParams::try_get(const std::string &key,
                                             std::string &value) {

	auto result = std::find(keys.begin(), keys.end(), key);
	if (result != keys.end()) {
		size_t idx = result - keys.begin();
		if (idx < values.size()) {
			value = values[idx];
			return true;
		}
	}

	return false;
}

std::string iti::http::router::RouteParams::get(const std::string &key) {
	std::string value;
	try_get(key, value);
	return value;
}
#endif // ITI_LIB_HTTP_ROUTER_ROUTEPARAMS_CPP
