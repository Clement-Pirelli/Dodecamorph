#include "InputFileParser.h"
#include <iostream>
#include <sstream>
#include "Dependencies/Logger/Logger.h"

namespace InputFile
{
    std::optional<Tensor<Cell>> parse(std::string contents)
	{
		Tensor<Cell> result;

		//crlf annoyance
		for (std::string::size_type carriage_return_pos = contents.find("\r\n"); carriage_return_pos != std::string::npos; carriage_return_pos = contents.find("\r\n"))
		{
			contents.erase(carriage_return_pos, 1);
		}

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

				//todo: allow custom strings for instructions on top of the numbers?
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
					try
					{
						result.setAtCoordinates(coords, std::stoi(token));
					}
					catch (...)
					{
						Logger::LogErrorFormatted("Initial state parsing: could not parse cell at row %u, column %u. Instead, found '%s'", x, y, token.c_str());
						return std::nullopt;
					}
				}

				x++;
			}
		}

		return result;
	}
}
