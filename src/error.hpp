#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include <mstd/string_view.hpp>

namespace conftaal {

class Error : public std::runtime_error {
public:
	Error(
		std::string what,
		mstd::string_view where = mstd::string_view(nullptr, 0),
		std::vector<std::pair<std::string, mstd::string_view>> notes = {}
	) : std::runtime_error(std::move(what)), where(where), notes(std::move(notes)) {}

	mstd::string_view where;

	// ('where', message) pairs
	std::vector<std::pair<std::string, mstd::string_view>> notes;
};

}
