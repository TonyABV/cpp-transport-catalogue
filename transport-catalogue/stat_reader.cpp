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

void statistic::PrinStat(vector<pair<char, Info>>&& information){
	for (auto info : information) {
		if (info.first == 'B') {
			cout << "Bus " << info.second.name_ << ": ";
			if (!info.second.existing_) {
				cout << "not found" << endl;
				continue;
			}
				cout << info.second.stops_on_route_ << " stops on route, " <<
				info.second.unique_stops_ << " unique stops, "<<
				info.second.route_length_ << " route length, " <<
					setprecision(6)<< info.second.curvature_<< " curvature" << endl;
		}
		else {
			cout << "Stop " << info.second.name_ << ": ";
			if (!info.second.existing_) {
				cout << "not found" << endl;
			}
			else if (info.second.busses_to_stop_.empty()) {
				cout << "no buses" << endl;
			}
			else
			{
				cout << "buses ";
				for (auto bus_name : info.second.busses_to_stop_) {
					cout << bus_name << ' ';
				}
				cout << endl;
			}
		}
	}
}
