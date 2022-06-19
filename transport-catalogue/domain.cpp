#include "domain.h"

namespace domain {
	bool Stop::operator==(const Stop& other) const {
		return name == other.name;
	}

	bool Bus::operator==(const Bus& other) const {
		return name == other.name;
	}
} // namespace domain
