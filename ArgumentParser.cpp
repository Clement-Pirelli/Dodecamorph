#include "ArgumentParser.h"
#include <tuple>
#include <algorithm>
#include "Dependencies/Logger/Logger.h"

namespace
{
	std::optional<std::string> findString(std::span<const char *> args, const std::string_view marker)
	{
		for (size_t i = 0; i < args.size(); i++) 
		{
			if (std::string_view(args[i]) == marker) 
			{
				const size_t string_index = i + 1;
				if (string_index < args.size()) 
				{
					return args[string_index];
				}
			}
		}
		return std::nullopt;
	}

	bool findMarker(std::span<const char *> args, const std::string_view marker)
	{
		return std::find_if(args.begin(), args.end(), [marker](const char* currentArg) 
		{
			return std::string_view(currentArg) == marker;
		}) != args.end();
	}

	void outputHelp()
	{
		Logger::LogMessage("-i: required, the path of the input file");
		Logger::LogMessage("-o: optional, the path of the output file. If not supplied, output will be to stdout");
		Logger::LogMessage("-w: optional, the path of the words file, which maps instructions to words.");
	}
}
namespace Arguments 
{
	std::optional<ParseResult> parse(std::span<const char *> args)
	{
		if (const bool help = findMarker(args, "-h")) 
		{
			outputHelp();
			return std::nullopt;
		}


		const auto inputPath = findString(args, "-i");
		const auto outputPath = findString(args, "-o");
		const auto wordsPath = findString(args, "-w");
		if (inputPath.has_value())
		{
			const ParseResult result
			{
				.inputPath = inputPath.value(),
				.outputPath = outputPath.value_or(""),
				.wordsPath = wordsPath.value_or("")
			};

			return result;
		}
		else
		{
			Logger::LogError("Please specify an input path using -i. Use -h for help");
			return std::nullopt;
		}
	}
}
