#include "config.hpp"
#include "utils.hpp"
#include "ifacestate.hpp"

#include <iostream>
#include <clickhouse/client.h>
#include <unordered_map>
#include <vector>
#include <thread>
#include <string>
#include <mutex>
#include <exception>

struct IfaceStatePoint {
	utime_t timePoint;
	IfaceStatesMap state;
};

struct SharedCtx {
	std::mutex mutex;
	std::vector<IfaceStatePoint> statesBuffer;
	AppConfig config;
};

clickhouse::Client getClickhouseConnection(const AppConfig &config) {
	auto &clickConf = config.clickhouse;

	return {
		clickhouse::ClientOptions()
			.SetHost(clickConf.host)
			.SetPort(clickConf.port)
			.SetUser(clickConf.user)
			.SetPassword(clickConf.password)
			.SetDefaultDatabase(clickConf.database)
			.SetConnectionConnectTimeout(std::chrono::milliseconds(5000))
			.SetConnectionRecvTimeout(std::chrono::milliseconds(5000))
			.SetConnectionSendTimeout(std::chrono::milliseconds(5000))
	};
}

[[noreturn]]
void readingThread(SharedCtx *ctx) {
	IfaceStatesMap lastState;
	readIfacesState(lastState);

	while (true) {
		sleepUntilNextSec();

		utime_t currentTime = currentTimeUs();
		IfaceStatesMap currentState, diffState;
		readIfacesState(currentState);

		for (auto &&[iface, cstate] : currentState) {
			if (lastState.contains(iface)) {
				auto diff = cstate - lastState[iface];
				if (diff.isValid()) {
					diffState[iface] = diff;
					continue;
				}
			}

			diffState[iface] = IfaceState{0};
		}

		{
			std::lock_guard l(ctx->mutex);

			ctx->statesBuffer.emplace_back(IfaceStatePoint{
				.timePoint = currentTime,
				.state = std::move(diffState),
			});

			if (ctx->statesBuffer.size() > ctx->config.maxBufferedStates) {
				ctx->statesBuffer.erase(ctx->statesBuffer.begin());
			}
		}

		lastState = std::move(currentState);
	}
}

void sendingThread(SharedCtx *ctx) {
	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(ctx->config.minStatesToInsert - 1));
		sleepUntilNextSec(10000); // with offset 10ms of readingThread, prevent data access races

		auto &interfaces = ctx->config.interfaces;
		if (ctx->statesBuffer.size() >= ctx->config.minStatesToInsert) {
			try {
				std::vector<IfaceStatePoint> stateToSend;
				{
					std::lock_guard guard(ctx->mutex);
					stateToSend = ctx->statesBuffer;
				}

				auto conn = getClickhouseConnection(ctx->config);

				clickhouse::Block block;
				auto timeCol = std::make_shared<clickhouse::ColumnDateTime>();
				auto groupIdCol = std::make_shared<clickhouse::ColumnString>();
				auto ifaceCol = std::make_shared<clickhouse::ColumnString>();
				auto rxBytesCol = std::make_shared<clickhouse::ColumnUInt32>();
				auto rxPacketsCol = std::make_shared<clickhouse::ColumnUInt32>();
				auto rxErrorsCol = std::make_shared<clickhouse::ColumnUInt32>();
				auto rxDropsCol = std::make_shared<clickhouse::ColumnUInt32>();
				auto txBytesCol = std::make_shared<clickhouse::ColumnUInt32>();
				auto txPacketsCol = std::make_shared<clickhouse::ColumnUInt32>();
				auto txErrorsCol = std::make_shared<clickhouse::ColumnUInt32>();
				auto txDropsCol = std::make_shared<clickhouse::ColumnUInt32>();

				for (auto &&statePoint : stateToSend) {
					for (auto &&[iface, state] : statePoint.state) {
						if (!interfaces.empty() && !interfaces.contains(iface)) {
							continue;
						}

						timeCol->AppendRaw(statePoint.timePoint / 1000 / 1000);
						groupIdCol->Append(ctx->config.groupId);
						ifaceCol->Append(iface);

						rxBytesCol->Append(state.rxBytes);
						rxPacketsCol->Append(state.rxPackets);
						rxErrorsCol->Append(state.rxErrors);
						rxDropsCol->Append(state.rxDrops);

						txBytesCol->Append(state.txBytes);
						txPacketsCol->Append(state.txPackets);
						txErrorsCol->Append(state.txErrors);
						txDropsCol->Append(state.txDrops);
					}
				}

				block.AppendColumn("time", timeCol);
				block.AppendColumn("group_id", groupIdCol);
				block.AppendColumn("iface", ifaceCol);
				block.AppendColumn("rx_bytes", rxBytesCol);
				block.AppendColumn("rx_packets", rxPacketsCol);
				block.AppendColumn("rx_errors", rxErrorsCol);
				block.AppendColumn("rx_drops", rxDropsCol);
				block.AppendColumn("tx_bytes", txBytesCol);
				block.AppendColumn("tx_packets", txPacketsCol);
				block.AppendColumn("tx_errors", txErrorsCol);
				block.AppendColumn("tx_drops", txDropsCol);

				conn.Insert("network_interfaces", block);

				if (ctx->config.debug) {
					std::cout << "Inserted " << block.GetRowCount() << " values" << std::endl;
				}

				{
					std::lock_guard guard(ctx->mutex);
					auto lastPoint = stateToSend[stateToSend.size() - 1];

					std::erase_if(ctx->statesBuffer, [&lastPoint](auto &&el) {
						return lastPoint.timePoint >= el.timePoint;
					});
				}
			} catch (std::exception &e) {
				std::cerr << "Exception while inserting values to clickhouse: " << e.what() << std::endl;
			} catch (...) {
				std::cerr << "Unknown exception occured" << std::endl;
			}
		}
	}
}

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cerr << "ifmetrica <config.json>";
		return 1;
	}

	auto configJson = json::parse(std::ifstream(argv[1]));
	AppConfig config = configJson.get<AppConfig>();

	SharedCtx ctx {
		.mutex = {},
		.statesBuffer = {},
		.config = config,
	};

	std::thread rThread(readingThread, &ctx);
	rThread.detach();

	std::cout << "Network metrics collection started" << std::endl;

	sendingThread(&ctx);

	return 0;
}