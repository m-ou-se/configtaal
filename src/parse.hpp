#pragma once

#include <memory>
#include <stdexcept>
#include <vector>

#include <optional.hpp>
#include <string_view.hpp>
#include <string_tracker.hpp>

#include "expression.hpp"

namespace conftaal {

class parse_error : public std::runtime_error {

private:
	string_view where_;

	// ('where', message) pairs
	std::vector<std::pair<std::string, string_view>> notes_;

public:
	parse_error(
		std::string what,
		string_view where,
		std::vector<std::pair<std::string, string_view>> notes = {}
	) : std::runtime_error(std::move(what)), where_(where), notes_(std::move(notes)) {}

	string_view where() const { return where_; }

	std::vector<std::pair<std::string, string_view>> const & notes() const { return notes_; }

};

class matcher;

extern matcher match_end_of_file;

class parser {

public:
	explicit parser(string_tracker & tracker, string_view source)
		: string_tracker_(tracker), source_(source) {}

	std::unique_ptr<expression> parse_expression(matcher const & end = match_end_of_file);
	std::unique_ptr<list_literal_expression> parse_list(matcher const & end = match_end_of_file);
	std::unique_ptr<object_literal_expression> parse_object(matcher const & end = match_end_of_file);

	static bool is_identifier_start(char c);
	static string_view parse_identifier(string_view & source);
	static std::unique_ptr<identifier_expression> parse_identifier_expression(string_view & source);

private:
	std::unique_ptr<expression> parse_expression_atom(matcher const & end);
	bool parse_more_expression(std::unique_ptr<expression> & expr, matcher const & end);

	std::unique_ptr<string_literal_expression> parse_string_literal();

	optional<string_view> parse_end(matcher const &, bool consume = true);

private:
	string_tracker & string_tracker_;
	string_view source_;

};

}
