#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace conftaal {

class Error : public std::runtime_error {
public:
	Error(
		std::string what,
		std::string_view where = std::string_view(nullptr, 0),
		std::vector<std::pair<std::string, std::string_view>> notes = {}
	) : std::runtime_error(std::move(what)), where(where), notes(std::move(notes)) {}

	std::string_view where;

	// ('where', message) pairs
	std::vector<std::pair<std::string, std::string_view>> notes;
};

}
