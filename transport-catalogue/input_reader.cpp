#include <stdexcept>

#include "input_reader.h"

using namespace std;

input::Requests input::MakeRequests(istream& input)
{
	string s_req_numbers;
	getline(input, s_req_numbers);
	size_t req_numbers = stoi(s_req_numbers);

	vector<NewStop> stop_requests;
	vector<Distance> dist_requests;
	vector<NewBus> bus_requests;

	for (size_t i = 0; i < req_numbers; ++i) {
		string request;
		getline(input, request);

		size_t type_begin = request.find_first_not_of(' ');
		
		if (request[type_begin] == 'B') {
			// "Bus" length = 3
			size_t word_length = 3;
			bus_requests.push_back(MakeBusRequest(
				move(request.substr(type_begin + word_length + 1))));
		}
		else if (request[type_begin] == 'S') {
			// "Stop" length = 4
			size_t word_length = 4;
			auto stop_dist_req = MakeStopRequest(move(request.substr(type_begin + word_length + 1)));
			stop_requests.push_back(move(stop_dist_req.first));
			dist_requests.push_back(move(stop_dist_req.second));
		}
		else {
			//throw logic_error("Invalid request.");
			std::cerr << "Invalid request.";
		}
	}
	return { stop_requests, dist_requests, bus_requests };
}

tuple<string, bool, vector<string>> input::MakeBusRequest(string&& text)
{
	size_t colon_pos = text.find(':');

	string bus_name;

	auto name_pos = FindNamePos(text, 0, colon_pos - 1);
	bus_name = move(text.substr(name_pos.first, name_pos.second - name_pos.first + 1));

	size_t token_pos = text.find_first_of("->", colon_pos + 1);
	char token = text[token_pos];

	vector<string> names_stops;
	size_t find_begin = colon_pos +1;
	for (; token_pos != string::npos; token_pos = text.find_first_of(token, find_begin)) {

		auto name_pos = FindNamePos(text, find_begin, token_pos - 1);
		string stop_name = text.substr(name_pos.first, name_pos.second - name_pos.first + 1);

		names_stops.push_back(move(stop_name));
		
		find_begin = token_pos + 1;
	}
	
	name_pos = FindNamePos(text, find_begin);
	string stop_name = text.substr(name_pos.first, name_pos.second - name_pos.first + 1);
	names_stops.push_back(move(stop_name));
	
	bool route_is_circle = true;
	if (token == '-') {
		route_is_circle = false;
		size_t old_size = names_stops.size();
		names_stops.reserve(old_size * 2 - 1);
		for (size_t n = 1; n < old_size; ++n) {
			names_stops.push_back(names_stops[old_size - 1 - n]);
		}
	}
	return make_tuple(bus_name, route_is_circle, names_stops);
}

pair<input::NewStop, input::Distance> input::MakeStopRequest(string && text)
{
	size_t colon_pos = text.find(':');

	auto name_pos = FindNamePos(text, 0, colon_pos - 1);
	string stop_name = text.substr(name_pos.first, name_pos.second - name_pos.first + 1);

	size_t comma_pos = text.find_first_of(',', colon_pos + 1);

	auto lat_pos = FindNamePos(text, colon_pos + 1, comma_pos - 1);
	double latitude = stod(text.substr(lat_pos.first, lat_pos.second - lat_pos.first + 1));

	size_t comma2_pos = text.find_first_of(',', comma_pos+1);
	auto lon_pos = FindNamePos(text, comma_pos + 1, comma2_pos - 1);
	double longitude = stod(text.substr(lon_pos.first, lon_pos.second - lon_pos.first + 1));

	vector<pair<string, int>> to_stop_dist;
	// text -> "..., distm to stop,..."
	comma_pos = comma2_pos;
	for (; comma_pos != string::npos;) {

		size_t m_pos = text.find_first_of('m', comma_pos + 1);
		auto dist_pos = FindNamePos(text, comma_pos + 1, m_pos - 1);
		double dist = stoi(text.substr(dist_pos.first, dist_pos.second - dist_pos.first + 1));

		size_t o_pos = text.find_first_of('o', m_pos + 1);
		comma_pos = text.find_first_of(',', o_pos + 1);
		auto name_pos = FindNamePos(text, o_pos + 1, comma_pos - 1);
		string stop_name = text.substr(name_pos.first, name_pos.second - name_pos.first + 1);

		to_stop_dist.push_back(move(make_pair(stop_name, dist)));
	}
	NewStop stop_req = make_tuple(stop_name, latitude, longitude);
	Distance dist_req = make_pair(stop_name, to_stop_dist);

	return make_pair(stop_req, dist_req);
}

pair<size_t, size_t> input::FindNamePos(string_view text,
					size_t begin_pos, size_t last_pos)
{
	size_t name_begin = text.find_first_not_of(' ', begin_pos);
	size_t name_end = text.find_last_not_of(" ", last_pos);
	return make_pair(name_begin, name_end);
}