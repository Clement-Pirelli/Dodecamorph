#pragma once
#include <optional>
#include <string>
#include <span>

namespace Arguments
{
	struct ParseResult
	{
		std::string inputPath;
		std::string outputPath;
	};

	std::optional<ParseResult> parse(std::span<const char*> args);
}

