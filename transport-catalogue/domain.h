#pragma once

#include "geo.h"
#include <string_view>
#include <vector>
#include <string>
#include <set>

namespace domain {
	struct Stop {
		std::string_view name = {}; 
		geo::Coordinates coordinates = { {}, {} };
		std::set<std::string_view> buses; 
		bool operator==(const Stop& other) const;
	};

	struct Bus {
		std::string_view name;  
		std::vector<std::string_view> stops; 
		bool is_round_trip = false;
		std::string_view end_stop;
		bool operator==(const Bus& other) const;
	};
}// namespace domain
