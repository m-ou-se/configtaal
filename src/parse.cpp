#include <cassert>
#include <cctype>
#include <stdexcept>

#include <string_view.hpp>
#include <string_tracker.hpp>
#include <optional.hpp>

#include "expression.hpp"
#include "parse.hpp"

namespace conftaal {

namespace {

void skip_whitespace(string_view & source, bool skip_newlines) {
	while (true) {
		while (!source.empty() && isspace(source[0])) {
			if (!skip_newlines && source[0] == '\n') return;
			source.remove_prefix(1);
		}
		if (!source.empty() && source[0] == '#') {
			while (!source.empty() && source[0] != '\n') source.remove_prefix(1);
		} else {
			return;
		}
	}
}

}

enum class match_mode {
	end_of_file,
	specific,
	matching_bracket,
	object_element,
};

class matcher {
private:
	match_mode mode_;
	string_view expected_;
	string_view matching_bracket_;
	matcher const * or_before_ = nullptr;

public:
	explicit matcher(string_view expected)
		: mode_(match_mode::specific), expected_(expected), matching_bracket_() {}

	matcher(match_mode mode, string_view expected = {}, string_view matching_bracket = {}, matcher const * or_before = nullptr)
		: mode_(mode), expected_(expected), matching_bracket_(matching_bracket), or_before_(or_before) {}

	optional<string_view> try_parse(string_view & source, bool consume = true, bool eat_whitespace = true) const {
		if (eat_whitespace) {
			bool skip_newlines = mode_ != match_mode::object_element;
			skip_whitespace(source, skip_newlines);
		}
		switch (mode_) {
			case match_mode::end_of_file:
				if (source.empty()) return source;
				break;
			case match_mode::specific:
			case match_mode::matching_bracket: {
				auto s = source.substr(0, expected_.size());
				if (s == expected_) {
					if (consume) source.remove_prefix(expected_.size());
					return s;
				}
				break;
			}
			case match_mode::object_element:
				if (!source.empty()) switch (source[0]) {
					case ',': case ';': case '\n': {
						auto match = source.substr(0, 1);
						if (consume) source.remove_prefix(1);
						return match;
					}
				}
				break;
		}
		if (or_before_) {
			auto m = or_before_->try_parse(source, false, false);
			if (m) return m->substr(0, 0);
		}
		return nullopt;
	}

	string_view parse(string_view & source, bool consume = true) const {
		auto m = try_parse(source, consume);
		if (!m) throw error(source);
		return *m;
	}

	// Life time of the 'or before' matcher must be at least as long as
	// the life time of this matcher.
	matcher or_before(matcher const & alt) {
		return matcher(mode_, expected_, matching_bracket_, &alt);
	}

	// Prevent taking temporaries.
	matcher or_before(matcher const &&) = delete;

	std::string description() const {
		std::string desc;
		switch (mode_) {
			case match_mode::end_of_file:
				desc = "end of file";
				break;
			case match_mode::specific:
			case match_mode::matching_bracket:
				desc = "`" + std::string(expected_) + "'";
				break;
			case match_mode::object_element:
				desc = "newline or `,' or `;'";
				break;
			default:
				throw std::logic_error("match_mode");
		}
		if (or_before_) desc += " or " + or_before_->description();
		return desc;
	}

	parse_error error(string_view source) const {
		std::vector<std::pair<std::string, string_view>> notes;
		if (mode_ == match_mode::matching_bracket && !or_before_) {
			notes = {{"... to match this `" + std::string(matching_bracket_) + "'", matching_bracket_}};
		}
		return parse_error(
			"expected " + description(),
			source.substr(0, 0),
			std::move(notes)
		);
	}

};

matcher match_end_of_file = conftaal::match_mode::end_of_file;

optional<string_view> parser::parse_end(matcher const & end, bool consume) {
	auto m = end.try_parse(source_, consume);
	if (!m && source_.empty()) {
		throw end.error(source_);
	}
	return m;
}

bool parser::is_identifier_start(char c) {
	return isalpha(c) || c == '_';
}

string_view parser::parse_identifier(string_view & source) {
	auto const original_source = source;
	while (!source.empty() && (isalpha(source[0]) || source[0] == '_' || isdigit(source[0]))) {
		source.remove_prefix(1);
	}
	return original_source.substr(0, original_source.size() - source.size());
}

namespace {

optional<int> hex_digit(char c) {
	switch (c) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			return c - '0';
		case 'a': case 'A': return 10;
		case 'b': case 'B': return 11;
		case 'c': case 'C': return 12;
		case 'd': case 'D': return 13;
		case 'e': case 'E': return 14;
		case 'f': case 'F': return 15;
		default:
			return nullopt;
	}
}

int parse_hex_digit(string_view & s) {
	if (!s.empty()) {
		if (auto value = hex_digit(s[0])) {
			s.remove_prefix(1);
			return *value;
		}
	}
	throw parse_error("expected hexadecimal digit (0-9, a-f, A-F)", s.substr(0, 0));
}

size_t encode_utf8(char32_t codepoint, char (& buffer)[4]) {
	if (codepoint < 0x7F) {
		buffer[0] = codepoint;
		return 1;
	} else if (codepoint < 0x7FF) {
		buffer[0] = 0xC0 | codepoint >> 6;
		buffer[1] = 0x80 | (codepoint & 0x3F);
		return 2;
	} else if (codepoint < 0xFFFF) {
		buffer[0] = 0xE0 | codepoint >> 12;
		buffer[1] = 0x80 | (codepoint >> 6 & 0x3F);
		buffer[2] = 0x80 | (codepoint & 0x3F);
		return 3;
	} else if (codepoint < 0x1FFFFF) {
		buffer[0] = 0xF0 | codepoint >> 18;
		buffer[1] = 0x80 | (codepoint >> 12 & 0x3F);
		buffer[2] = 0x80 | (codepoint >> 6 & 0x3F);
		buffer[3] = 0x80 | (codepoint & 0x3F);
		return 4;
	} else {
		return 0;
	}
}

}

std::unique_ptr<string_literal_expression> parser::parse_string_literal() {
	auto const original_source = source_;

	char const quote = source_[0];
	source_.remove_prefix(1);

	auto string_builder = string_tracker_.builder();
	string_view value;

	while (true) {
		value = source_.substr(0, 0);
		while (!source_.empty() && source_[0] != quote && source_[0] != '\\') {
			source_.remove_prefix(1);
		}
		value = string_view(value.data(), source_.data() - value.data());
		if (source_.empty()) {
			throw parse_error("unterminated string literal", original_source);
		} else if (source_[0] == quote) {
			source_.remove_prefix(1);
			break;
		} else {
			assert(source_[0] == '\\');
			if (!value.empty()) string_builder.append(value, value);

			if (source_.size() < 2) throw parse_error("incomplete escape sequence", source_);
			switch (source_[1]) {
				case '\\': case '"':
				case 't': case 'n': case 'r':
				case 'b': case 'a': case 'e':
				case 'f': case 'v': {
					string_view replacement;
					switch (source_[1]) {
						case '\\': replacement = "\\"; break;
						case '"': replacement = "\""; break;
						case 't': replacement = "\t"; break;
						case 'n': replacement = "\n"; break;
						case 'r': replacement = "\r"; break;
						case 'b': replacement = "\b"; break;
						case 'a': replacement = "\a"; break;
						case 'e': replacement = "\033"; break;
						case 'f': replacement = "\f"; break;
						case 'v': replacement = "\v"; break;
					}
					string_builder.append(replacement, source_.substr(0, 2));
					source_.remove_prefix(2);
					break;
				}
				case '\n':
					source_.remove_prefix(2);
					break;
				case 'x': {
					// 8-bit byte in hex
					auto escape_sequence = source_.substr(0, 4);
					source_.remove_prefix(2);
					int a = parse_hex_digit(source_);
					int b = parse_hex_digit(source_);
					char ch = a << 4 | b;
					string_builder.append(string_view(&ch, 1), escape_sequence);
					break;
				}
				case 'u':
				case 'U': {
					// 16-bit ('u') or 32-bit ('U') unicode codepoint in hex
					int n_digits = source_[1] == 'u' ? 4 : 8;
					auto escape_sequence = source_.substr(0, 2 + n_digits);
					source_.remove_prefix(2);
					char32_t codepoint = 0;
					for (int i = 0; i < n_digits; ++i) {
						codepoint <<= 4;
						codepoint |= parse_hex_digit(source_);
					}
					char buffer[4];
					if (size_t n_bytes = encode_utf8(codepoint, buffer)) {
						string_builder.append(string_view(buffer, n_bytes), escape_sequence);
					} else {
						throw parse_error(
							"invalid unicode codepoint",
							escape_sequence
						);
					}
					break;
				}
				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7': {
					// 8-bit byte in octal (1-3 digits)
					auto escape_sequence_start = source_.data();
					int value = source_[1] - '0';
					source_.remove_prefix(2);
					int n_digits = 1;
					while (n_digits < 3 && !source_.empty() && source_[0] >= '0' && source_[0] <= '7') {
						value <<= 3;
						value |= source_[0] - '0';
						++n_digits;
						source_.remove_prefix(1);
					}
					string_view escape_sequence(escape_sequence_start, source_.data() - escape_sequence_start);
					if (value > 255) throw parse_error("octal escape sequence out of range", escape_sequence);
					char ch = value;
					string_builder.append(string_view(&ch, 1), escape_sequence);
					break;
				}
				default:
					throw parse_error("invalid escape sequence", source_.substr(0, 2));
			}
		}
	}

	if (!string_builder.empty()) {
		// String literal contained escape sequences,
		// so the literal value is not a substring of the source.
		string_builder.append(value, value);
		value = string_builder.build();
	}

	return std::make_unique<string_literal_expression>(value);
}

std::unique_ptr<identifier_expression> parser::parse_identifier_expression(string_view & source) {
	auto identifier = parse_identifier(source);
	if (!identifier.empty()) {
		return std::make_unique<identifier_expression>(identifier);
	} else {
		return nullptr;
	}
}

namespace {

int get_precedence(string_view op, bool unary) {
	if (unary) {
		return 3;
	} else {
		if (!op.empty()) switch (op[0]) {
			case '.': case '(': case '[':
				return 1; // . ( [
			case '*':
				if (op.size() > 1 && op[1] == '*') return 4; // **
			case '/': case '%':
				return 5; // * / %
			case '+': case '-':
				return 6; // + -
			case '<': case '>':
				if (op.size() > 1 && op[1] == op[0]) return 7; // >> <<
				return 8; // > < >= <=
			case '=': case '!':
				return 9; // != ==
			case '&':
				if (op.size() > 1 && op[1] == '&') return 13; // &&
				return 10; // &
			case '^':
				return 11; // ^
			case '|':
				if (op.size() > 1 && op[1] == '|') return 14; // ||
				return 12; // |
		}
	}
	throw std::logic_error("get_precedence");
}

}

std::unique_ptr<expression> parser::parse_expression_atom(matcher const & end) {
	if (parse_end(end, false)) return nullptr;

	if (source_[0] == '(') {
		auto open = source_.substr(0, 1);
		source_.remove_prefix(1);
		auto expr = parse_expression(matcher(match_mode::matching_bracket, ")", open));
		if (!expr) throw parse_error(
			"missing expression between `(' and `)'",
			string_view(open.data(), source_.data() - open.data() + 1)
		);
		if (auto e = dynamic_cast<operator_expression *>(expr.get())) {
			e->parenthesized = true;
		}
		return expr;

	} else if (source_[0] == '!' || source_[0] == '~' || source_[0] == '-' || source_[0] == '+') {
		auto op = source_.substr(0, 1);
		source_.remove_prefix(1);
		auto subexpr = parse_expression_atom(end);
		if (!subexpr) throw parse_error(
			"missing expression after unary `" + std::string(op) + "' operator",
			string_view(op.data(), source_.data() - op.data() + 1)
		);
		return std::make_unique<operator_expression>(op, nullptr, std::move(subexpr));

	} else if (is_identifier_start(source_[0])) {
		return parse_identifier_expression(source_);

	} else if (source_[0] == '{') {
		auto open = source_.substr(0, 1);
		source_.remove_prefix(1);
		return parse_object(matcher(match_mode::matching_bracket, "}", open));

	} else if (source_[0] == '[') {
		auto open = source_.substr(0, 1);
		source_.remove_prefix(1);
		return parse_list(matcher(match_mode::matching_bracket, "]", open));

	} else if (source_[0] == '"') {
		return parse_string_literal();

	} else if (source_[0] == '\\') {
		throw parse_error("lambdas are not yet implemented", source_.substr(0, 1));

	} else {
		throw parse_error("expected expression", source_.substr(0, 0));

	}

}

bool parser::parse_more_expression(std::unique_ptr<expression> & expr, matcher const & end) {
	if (parse_end(end)) return false;

	switch (source_[0]) {
		case ':':
		case '+': case '-':
		case '*': case '/': case '%':
		case '=': case '!': case '>': case '<':
		case '^': case '&': case '|':
		case '[': case '(': case '.':
		case '~': {

			auto op = source_.substr(0, 1);

			// Multi-character operators.
			switch (source_[0]) {
				case '!': // !=
				case '=': // ==
					if (source_.size() > 1 && source_[1] == '=') {
						op = source_.substr(0, 2);
					} else {
						// Just '!' and '=' aren't binary operators.
						if (source_[0] == '!') {
							throw parse_error("`!' can only be used as unary operator", op);
						} else {
							throw parse_error("assignment (`=') cannot be used in expressions (did you mean `=='?)", op);
						}
					}
					break;
				case '*': // **
				case '&': // &&
				case '|': // ||
					if (source_.size() > 1 && source_[1] == source_[0]) {
						op = source_.substr(0, 2);
					}
					break;
				case '>': // >> >=
				case '<': // << <=
					if (source_.size() > 1 && (source_[1] == source_[0] || source_[1] == '=')) {
						op = source_.substr(0, 2);
					}
					break;
				case '~':
					throw parse_error("`~' can only be used as unary operator", op);
					break;
			}

			source_.remove_prefix(op.size());

			std::unique_ptr<expression> rhs;

			if (op == "[" || op == "(") {
				rhs = parse_list(matcher(match_mode::matching_bracket, op == "[" ? "]" : ")", op));
			} else if (op == ".") {
				rhs = parse_identifier_expression(source_);
				if (!rhs) throw parse_error(
					"expected identifier after `.'",
					string_view(op.data(), source_.data() - op.data() + 1)
				);
			} else {
				rhs = parse_expression_atom(end);
				if (!rhs) throw parse_error(
					"missing expression after `" + std::string(op) + "' operator",
					string_view(op.data(), source_.data() - op.data() + 1)
				);
			}

			// Find the expression to use as left hand side.
			// Often it is the entire expression expr,
			// but depending on the precedence of operators,
			// it might be just a subexpresssion of expr.
			std::unique_ptr<expression> * lhs = &expr;
			while (true) {
				auto e = dynamic_cast<operator_expression *>(lhs->get());
				if (!e) break;
				if (e->parenthesized) break;
				if (get_precedence(op, false) >= get_precedence(e->op, e->is_unary())) break;
				lhs = &e->rhs;
			}

			// Replace the expression by an operator_expression that uses it as the left hand side.
			*lhs = std::make_unique<operator_expression>(op, std::move(*lhs), std::move(rhs));

			return true;
		}
	}

	throw parse_error(
		"expected binary operator or " + end.description(),
		source_.substr(0, 0)
	);
}

std::unique_ptr<expression> parser::parse_expression(matcher const & end) {
	auto expr = parse_expression_atom(end);
	if (expr) while (parse_more_expression(expr, end));
	return expr;
}

std::unique_ptr<object_literal_expression> parser::parse_object(matcher const & end) {
	std::vector<std::pair<std::unique_ptr<expression> , std::unique_ptr<expression>>> values;
	while (true) {
		if (parse_end(end)) break;
		auto name = parse_identifier(source_);
		if (name.empty()) throw parse_error("expected identifier or " + end.description(), source_.substr(0, 0));
		auto eq = matcher("=").parse(source_);
		auto value = parse_expression(matcher(match_mode::object_element).or_before(end));
		if (!value) throw parse_error(
			"missing expression after `='",
			string_view(eq.data(), source_.data() - eq.data() + 1)
		);
		values.emplace_back(std::make_unique<string_literal_expression>(name), std::move(value));
	}
	return std::make_unique<object_literal_expression>(std::move(values));
}

std::unique_ptr<list_literal_expression> parser::parse_list(matcher const & end) {
	std::vector<std::unique_ptr<expression>> values;
	while (true) {
		char const * expression_begin = source_.data();
		if (parse_end(end)) break;
		auto value = parse_expression(matcher(",").or_before(end));
		if (!value) throw parse_error(
			"missing expression",
			string_view(expression_begin, source_.data() - expression_begin + 1)
		);
		values.push_back(std::move(value));
	}
	return std::make_unique<list_literal_expression>(std::move(values));
}

}
