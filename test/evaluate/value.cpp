#define CATCH_CONFIG_MAIN
#include "../../src/parse.hpp"
#include "../../src/evaluate.hpp"

#include <catch.hpp>

namespace Catch {
	template<> struct StringMaker<conftaal::TypeInfo> {
		static std::string convert(conftaal::TypeInfo const & value) {
			return value.name();
		}
	};
}

string_pool::string_tracker tracker;

std::unique_ptr<conftaal::Expression> parse(std::string_view source) {
	conftaal::Parser parser{tracker, source};
	return parser.parse_expression();
}

TEST_CASE("Test double Value", "[value]") {
	conftaal::Value double_val{0.0};
	REQUIRE(double_val.type() == conftaal::TypeInfo::of<double>());
	REQUIRE(double_val.is<double>());
	REQUIRE(double_val.as<double>() == 0.0);
}

TEST_CASE("Test int64 Value", "[value]") {
	conftaal::Value val{std::int64_t(1)};
	REQUIRE(val.type() == conftaal::TypeInfo::of<std::int64_t>());
	REQUIRE(val.is<std::int64_t>());
	REQUIRE(val.as<std::int64_t>() == 1);
}

TEST_CASE("Test string Value", "[value]") {
	conftaal::Value val{std::string("aap")};
	REQUIRE(val.type() == conftaal::TypeInfo::of<std::string>());
	REQUIRE(val.is<std::string>());
	REQUIRE(val.as<std::string>() == "aap");
}

TEST_CASE("Test Value.swap", "[value]") {
	conftaal::Value val_a{std::string("aap")};
	conftaal::Value val_b{std::int64_t(1)};

	REQUIRE(val_a.as<std::string>() == "aap");
	REQUIRE(val_b.as<std::int64_t>() == 1);

	val_a.swap(val_b);
	REQUIRE(val_a.as<std::int64_t>() == 1);
	REQUIRE(val_b.as<std::string>() == "aap");
}
