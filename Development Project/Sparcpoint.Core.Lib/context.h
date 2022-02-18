#ifndef ITI_LIB_CONTEXT_H
#define ITI_LIB_CONTEXT_H

#include <any>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

namespace iti {
class Context {
  public:
	template <typename T>
	bool try_get_value(const std::string_view key, T &data) const {
		try {
			data = std::any_cast<T>(ctxData.at(std::string(key)));
		} catch (const std::out_of_range &) {
			// not found in map
			return false;
		} catch (const std::bad_any_cast &) {
			// bad cast
			return false;
		}
		return true;
	}

	template <typename T>
	void set_value(const std::string_view key, const T &data) {
		ctxData[std::string(key)] = std::any(data);
	}

  private:
	std::unordered_map<std::string, std::any> ctxData;
};

} // namespace iti

#endif // ITI_LIB_CONTEXT_H
