#pragma once

#include <shared_mutex>
#include <string>

class CfgService {
  public:
	static CfgService &GetInstance() {
		static CfgService instance;
		return instance;
	}

	std::string GetConnectionString() const;
	unsigned int GetServerPort() const;
	unsigned int GetPageSize() const;

  private:
	CfgService();
	~CfgService()                  = default;
	CfgService(const CfgService &) = delete;
	CfgService &operator=(const CfgService &) = delete;

	mutable std::shared_mutex mtx;
	std::string connectionStr;
	uint16_t serverPort = 8080;
	unsigned int pageSize = 50;
	void init();
};
