#ifndef IFMETRICA_CONFIG_HPP
#define IFMETRICA_CONFIG_HPP

#include <nlohmann/json.hpp>
#include <unordered_set>

using nlohmann::json;

struct ClickhouseDbConfig {
	std::string host;
	int32_t port;
	std::string database;
	std::string user;
	std::string password;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(ClickhouseDbConfig, host, port, database, user, password)
};

struct AppConfig {
	bool debug;
	ClickhouseDbConfig clickhouse;
	std::string groupId;
	uint32_t maxBufferedStates;
	uint32_t minStatesToInsert;
	std::unordered_set<std::string> interfaces;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(AppConfig, debug, clickhouse, groupId, maxBufferedStates, minStatesToInsert, interfaces)
};



#endif //IFMETRICA_CONFIG_HPP
