#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include <string_view.hpp>

namespace conftaal {

class Error : public std::runtime_error {
public:
	Error(
		std::string what,
		string_view where = string_view(nullptr, 0),
		std::vector<std::pair<std::string, string_view>> notes = {}
	) : std::runtime_error(std::move(what)), where(where), notes(std::move(notes)) {}

	string_view where;

	// ('where', message) pairs
	std::vector<std::pair<std::string, string_view>> notes;
};

}
