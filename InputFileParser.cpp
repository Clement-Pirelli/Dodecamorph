#include "InputFileParser.h"
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <array>
#include <algorithm>
#include "Dependencies/Logger/Logger.h"


namespace 
{
	void remove_crlf(std::string& s)
	{
		for (std::string::size_type pos = s.find("\r\n"); pos != std::string::npos; pos = s.find("\r\n"))
		{
			s.erase(pos, 1);
		}
	}

	using Words = std::array<std::string, 12>;

	bool validate(const Words& words)
	{
		for(auto& word : words)
		{
			auto contains = [&word](const char* contained)
			{
				return word.find(contained) != std::string::npos;
			};

			if(std::count(words.begin(), words.end(), word) > 1)
			{
				Logger::LogErrorFormatted("Word %s is not unique! Please provide unique words", word.c_str());
				return false;
			}

			if(contains("("))
			{
				Logger::LogErrorFormatted("Word %s contains reserved character \"(\"!", word.c_str());
				return false;
			}

			if (contains(")"))
			{
				Logger::LogErrorFormatted("Word %s contains reserved character \")\"!", word.c_str());
				return false;
			}
		}

		return true;
	}


	std::optional<Words> parse_words(const std::string words)
	{
		if (words.empty())
		{
			return std::nullopt;
		}

		Words result{};
		std::string line;
		std::string token;

		std::stringstream stream(words);
		
		for(auto& word : result)
		{
			if(!(stream >> word))
			{
				Logger::LogError("Too few words specified! Please specify all words.");
				return std::nullopt;
			}
		}

		if(!validate(result))
		{
			return std::nullopt;
		}
		
		return result;
	}

	std::optional<int> get_instruction(const std::string& token, const std::optional<Words> maybe_words, int x, int y)
	{
		if (maybe_words)
		{
			const auto& words = maybe_words.value();
			auto it = std::find(words.begin(), words.end(), token);
			if(it != words.end())
			{
				return static_cast<int>(std::distance(words.begin(), it));
			}
		}
		
		try
		{
			return std::stoi(token);
		}
		catch (...)
		{
			Logger::LogErrorFormatted("Initial state parsing: could not parse cell at row %u, column %u. Instead, found '%s'", x, y, token.c_str());
			return std::nullopt;
		}
		
	}
}


namespace InputFile
{
    std::optional<Tensor<Cell>> parse(std::string contents, std::string words)
	{
		Tensor<Cell> result;

		remove_crlf(contents);
		remove_crlf(words);

		auto maybe_words = parse_words(words);

		std::vector<int> coords = { 0 };

		std::string line;
		std::string token;

		std::stringstream cellsStream(contents);
		for (int y = 0; std::getline(cellsStream, line, '\n'); y++)
		{
			std::stringstream line_stream(line);

			int x = 0;
			while (line_stream >> token)
			{
				if (token.starts_with('/')) //comments
				{
					y--;
					break;
				}

				coords[0] = x;

				if (y != 0)
				{
					if (coords.size() == 1)
					{
						coords.push_back(y);
					}
					else
					{
						coords[1] = y;
					}
				}

				//use the words here if it's possible to do so
				if (token.compare("(") == 0)
				{
					result.setAtCoordinates(coords, OpeningParens{});
				}
				else if (token.compare(")") == 0)
				{
					result.setAtCoordinates(coords, ClosingParens{});
				}
				else
				{
					if(auto maybe_instruction = get_instruction(token, maybe_words, x, y))
					{
						result.setAtCoordinates(coords, maybe_instruction.value());
					}
					else
					{
						return std::nullopt;
					}
				}

				x++;
			}
		}

		return result;
	}
}
