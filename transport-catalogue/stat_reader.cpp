#include "stat_reader.h"

using namespace std;

statistic::Requests statistic::MakeRequests(istream& input)
{
	string s_req_numbers;
	getline(input, s_req_numbers);
	size_t req_numbers = stoi(s_req_numbers);
	
	Requests result;
	result.requests.reserve(req_numbers);
	pair<char, string> request;
	for (size_t i = 0; i < req_numbers; ++i) {
		string request;
		getline(input, request);

		size_t type_begin = request.find_first_not_of(' ');

		if (request[type_begin] == 'B') {
			// "Bus" length = 3
			size_t word_length = 3;
			char type_marker = 'B';
			result.requests.push_back(make_pair(type_marker,
				move(request.substr(type_begin + word_length + 1))));
		}
		else if(request[type_begin] == 'S'){
			// "Stop" length = 4
			size_t word_length = 4;
			char type_marker = 'S';
			result.requests.push_back(make_pair(type_marker,
				move(request.substr(type_begin + word_length + 1))));
		}
		else {
			//throw logic_error("Invalid request type.");
			std::cerr << "Invalid request.";
		}
	}
	return result;
}

void statistic::PrinStat(ostream& out, vector<pair<char, domain::Stat>>&& information){
	for (auto stat : information) {
		if (stat.first == 'B') {
			domain::BusStat busstat = get<domain::BusStat>(stat.second);
			out << "Bus " << busstat.name_ << ": ";
			if (!busstat.existing_) {
				out << "not found" << endl;
				continue;
			}
				out << busstat.stops_count_ << " stops on route, " <<
					busstat.unique_stops_count_ << " unique stops, "<<
					busstat.route_length_ << " route length, " <<
					setprecision(6) << busstat.curvature_<< " curvature" << endl;
		}
		else {
			domain::StopStat stopstat = get<domain::StopStat>(stat.second);
			out << "Stop " << stopstat.name_ << ": ";
			if (!stopstat.existing_) {
				out << "not found" << endl;
			}
			else if (stopstat.busses_.empty()) {
				out << "no buses" << endl;
			}
			else
			{
				out << "buses ";
				for (domain::Bus* bus : stopstat.busses_) {
					out << bus->name_ << ' ';
				}
				out << endl;
			}
		}
	}
}
