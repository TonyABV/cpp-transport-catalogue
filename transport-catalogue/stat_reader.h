#pragma once
#include <iomanip>
#include <iostream>
#include <istream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "transport_catalogue.h"

namespace statistic {
struct Requests
{
	std::vector<std::pair<char, std::string>> requests;
};

Requests MakeRequests(std::istream& input);

void PrinStat(std::ostream& out, std::vector<std::pair<char, domain::Stat>>&& information);
}
