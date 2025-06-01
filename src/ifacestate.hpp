#ifndef IFMETRICA_IFACESTATE_HPP
#define IFMETRICA_IFACESTATE_HPP

#include "utils.hpp"
#include <unordered_map>
#include <fstream>

constexpr auto DEV_INFO_PATH = "/proc/net/dev";

struct IfaceState {
	uint64_t rxBytes;
	uint64_t rxPackets;
	uint64_t rxErrors;
	uint64_t rxDrops;
	uint64_t rxFifoErrors;
	uint64_t rxFrame;
	uint64_t rxCompressed;
	uint64_t rxMulticast;

	uint64_t txBytes;
	uint64_t txPackets;
	uint64_t txErrors;
	uint64_t txDrops;
	uint64_t txFifoErrors;
	uint64_t txCollisions;
	uint64_t txCarrierLosses;
	uint64_t txCompressed;

	IfaceState operator -(const IfaceState &state) const {
		return IfaceState {
			.rxBytes = rxBytes - state.rxBytes,
			.rxPackets = rxPackets - state.rxPackets,
			.rxErrors = rxErrors - state.rxErrors,
			.rxDrops = rxDrops - state.rxDrops,
			.rxFifoErrors = rxFifoErrors - state.rxFifoErrors,
			.rxFrame = rxFrame - state.rxFrame,
			.rxCompressed = rxCompressed - state.rxCompressed,
			.rxMulticast = rxMulticast - state.rxMulticast,
			.txBytes = txBytes - state.txBytes,
			.txPackets = txPackets - state.txPackets,
			.txErrors = txErrors - state.txErrors,
			.txDrops = txDrops - state.txDrops,
			.txFifoErrors = txFifoErrors - state.txFifoErrors,
			.txCollisions = txCollisions - state.txCollisions,
			.txCarrierLosses = txCarrierLosses - state.txCarrierLosses,
			.txCompressed = txCompressed - state.txCompressed,
		};
	}

	bool isValid() {
		return
			rxBytes >= 0
			&& rxPackets >= 0
			&& rxErrors >= 0
			&& rxDrops >= 0
			&& rxFifoErrors >= 0
			&& rxFrame >= 0
			&& rxCompressed >= 0
			&& rxMulticast >= 0
			&& txBytes >= 0
			&& txPackets >= 0
			&& txErrors >= 0
			&& txDrops >= 0
			&& txFifoErrors >= 0
			&& txCollisions >= 0
			&& txCarrierLosses >= 0
			&& txCompressed >= 0
		;
	}
};

using IfaceStatesMap = std::unordered_map<std::string, IfaceState>;

inline void readIfacesState(IfaceStatesMap &currentStates) {
	currentStates.clear();

	std::ifstream devInfoFile(DEV_INFO_PATH);
	std::string line;
	std::string ifaceName;

	while (std::getline(devInfoFile, line)) {
		std::istringstream iss(line);

		iss >> ifaceName;
		if (ifaceName.ends_with(':')) {
			ifaceName.resize(ifaceName.size() - 1);

			IfaceState state{};
			iss
				>> state.rxBytes
				>> state.rxPackets
				>> state.rxErrors
				>> state.rxDrops
				>> state.rxFifoErrors
				>> state.rxFrame
				>> state.rxCompressed
				>> state.rxMulticast

				>> state.txBytes
				>> state.txPackets
				>> state.txErrors
				>> state.txDrops
				>> state.txFifoErrors
				>> state.txCollisions
				>> state.txCarrierLosses
				>> state.txCompressed
			;

			currentStates.emplace(ifaceName, state);
		}
	}
}

#endif //IFMETRICA_IFACESTATE_HPP
