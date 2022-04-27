#include <string>
#include <type_traits>
#include "ArgumentParser.h"
#include "Tensor.h"
#include "Cell.h"
#include "InputFileParser.h"
#include "Dependencies/Files.h"
#include "Dependencies/Logger/Logger.h"


enum Direction : unsigned char
{
	Neutral = 0,
	Incremental = 1,
	Decremental = 2,
	DirectionCount
};

struct Cursor
{
	Coordinates cell_index;
	Coordinates tensor_index;
};

Tensor<Tensor<Cell>> meta_tensor;

Cursor instruction_cursor = Cursor{ { 0 }, { 0 } };
Cursor data_cursor = Cursor{ { 0 }, { 1 } };

std::vector<Direction> instruction_cursor_direction = { Incremental };

Tensor<Cell>& get_instruction_tensor() 
{
	return meta_tensor.at(instruction_cursor.tensor_index);
}

Tensor<Cell>& get_data_tensor()
{
	return meta_tensor.at(data_cursor.tensor_index);
}

Cell& get_current_data_cell() 
{
	return get_data_tensor().at(data_cursor.cell_index);
}

Cell& get_current_instruction_cell()
{
	return get_instruction_tensor().at(instruction_cursor.cell_index);
}

bool initial_setup(const Arguments::ParseResult& parseResult)
{
	auto read_input = [&]()
	{
		FileReader reader = FileReader(parseResult.inputPath);
		return reader.readInto<std::string>();
	};

	auto read_words = [&]()
	{
		if(parseResult.wordsPath.empty())
		{
			return std::string();
		}
		FileReader reader = FileReader(parseResult.wordsPath);
		return reader.readInto<std::string>();
	};

	if(auto maybe_result = InputFile::parse(read_input(), read_words()))
	{
		get_instruction_tensor() = std::move(maybe_result).value();
		return true;
	}

	return false;
}

Coordinates next_index_for(Coordinates index) 
{
	const std::vector<int>& instruction_tensor_dimensions = get_instruction_tensor().getDimensions();
	const std::span<const int> dimensions(instruction_tensor_dimensions.begin(), instruction_tensor_dimensions.end());
	std::vector<int> movement_by;
	movement_by.resize(instruction_cursor_direction.size());

	for (size_t i = 0; i < instruction_cursor_direction.size(); i++)
	{
		Direction current_direction = static_cast<Direction>(instruction_cursor_direction[i] % Direction::DirectionCount);
		if (current_direction == Direction::Incremental)
		{
			movement_by[i] = 1;
		}
		else if (current_direction == Direction::Decremental)
		{
			movement_by[i] = -1;
		}
	}

	return index.increment(Coordinates(std::move(movement_by)), dimensions);
}

void cursor_tick() 
{
	instruction_cursor.cell_index = next_index_for(instruction_cursor.cell_index);
}

template<typename OnNumberOperation_t>
std::optional<Coordinates> find_closing_parens_for(const Coordinates& opening_parens_index, OnNumberOperation_t onNumber)
{
	int parens_count = 1;
	Coordinates current_index = opening_parens_index;

	while(parens_count != 0)
	{
		current_index = next_index_for(current_index);
		if (Coordinates::equal(current_index, opening_parens_index))
		{
			return std::nullopt;
		}

		const Cell &current_cell = get_instruction_tensor().at(current_index);
		if (std::holds_alternative<ClosingParens>(current_cell)) 
		{
			parens_count--;
		}
		else if (std::holds_alternative<OpeningParens>(current_cell)) 
		{
			parens_count++;
		}
		else if (std::holds_alternative<int>(current_cell) && parens_count == 1) 
		{
			onNumber(std::get<0>(current_cell));
		}
	}

	return current_index;
}

std::optional<Coordinates> find_closing_parens_for(const Coordinates& opening_parens_index)
{
	return find_closing_parens_for(opening_parens_index, [](int) {});
}

struct PairParensReturn 
{
	Coordinates closing_parens_index;
	std::vector<int> numbers;
};

std::optional<PairParensReturn> pair_parens(const Coordinates& opening_parens_index)
{
	PairParensReturn result;

	auto on_number = [&result](int n) { result.numbers.push_back(n); };

	std::optional<Coordinates> found_closing_parens = find_closing_parens_for(opening_parens_index, on_number);
	if (found_closing_parens.has_value()) 
	{
		result.closing_parens_index = std::move(found_closing_parens.value());
		return result;
	}
	else
	{
		return std::nullopt;
	}
}

template<typename Operation_t>
bool pair_parens_and_execute(const Operation_t& operation) 
{
	std::optional<PairParensReturn> found_paired_parens = pair_parens(next_index_for(instruction_cursor.cell_index));
	if (found_paired_parens.has_value())
	{
		if constexpr (std::is_invocable_r<bool, Operation_t, PairParensReturn>())
		{
			return operation(found_paired_parens.value());
		}
		else 
		{
			operation(found_paired_parens.value());
			return true;
		}
	}
	else
	{
		return false;
	}
}

bool execute_current_instruction();
bool execute_instruction(const Instruction instruction)
{
	bool succeeded = true;
	
	switch (instruction) 
	{
	case OutputCurrentData:
	{
		const Cell& cell = get_current_data_cell();
		if (std::holds_alternative<int>(cell)) 
		{
			std::cout << std::get<0>(cell) << ' ';
		}
		else
		{
			std::cout << (std::holds_alternative<OpeningParens>(cell) ? '(' : ')') << ' ';
		}
	}
	break;
	case IncrementDataCursorCellIndex:
	{
		succeeded = pair_parens_and_execute([&](const PairParensReturn& paired_parens)
		{
			data_cursor.cell_index.increment(paired_parens.numbers, get_data_tensor().getDimensions());
		});
	}
	break;
	case SetDataCursorTensorIndex:
	{
		succeeded = pair_parens_and_execute([&](PairParensReturn& paired_parens)
		{
			data_cursor.tensor_index = Coordinates(std::move(paired_parens.numbers));
		});
	}
	break;
	case IncrementDataCell:
	{
		Cell& cell = get_current_data_cell();
		if (std::holds_alternative<int>(cell))
		{
			std::get<0>(cell)++;
		}
		else
		{
			std::get<0>(cell) = 0;
		}
	}
	break;
	case DecrementDataCell:
	{
		Cell& cell = get_current_data_cell();
		if (std::holds_alternative<int>(cell))
		{
			std::get<0>(cell)--;
		}
		else
		{
			std::get<0>(cell) = 0;
		}
	}
	break;
	case SetInstructionCursorDirection:
	{
		succeeded = pair_parens_and_execute([&](const PairParensReturn& paired_parens)
		{
			const std::vector<int>& numbers = paired_parens.numbers;
			instruction_cursor_direction.resize(numbers.size());
			for (size_t i = 0; i < numbers.size(); i++) 
			{
				instruction_cursor_direction[i] = static_cast<Direction>(numbers[i] % Direction::DirectionCount);
			}
		});
	}
	break;
	case SetDataCellUserInput:
	{
		int userInput = 0;
		std::cin >> userInput;
		get_current_data_cell() = userInput;
	}
	break;
	case ConditionalSetInstructionCursorCellIndex:
	{
		succeeded = pair_parens_and_execute([&](PairParensReturn& paired_parens)
		{
			const Cell& data_cell = get_current_data_cell();
			const bool data_cell_is_zero = std::holds_alternative<int>(data_cell) && std::get<0>(data_cell) == 0;
			if (data_cell_is_zero)
			{
				instruction_cursor.cell_index = Coordinates(std::move(paired_parens.numbers));
				return execute_current_instruction();
			}
			return true;
		});
	}
	break;
	case SetDataCellOpeningParens:
	{
		get_current_data_cell() = OpeningParens{};
	}
	break;
	case SetDataCellClosingParens:
	{
		get_current_data_cell() = ClosingParens{};
	}
	break;
	case SetInstructionCursorTensorIndex:
	{
		succeeded = pair_parens_and_execute([&](PairParensReturn& paired_parens)
		{
			instruction_cursor.tensor_index = Coordinates(std::move(paired_parens.numbers));
		});
	}
	break;
	case ShrinkTensor:
	{
		succeeded = pair_parens_and_execute([&](PairParensReturn& paired_parens)
		{
			meta_tensor.at(Coordinates(std::move(paired_parens.numbers))).shrink();
		});
	}
	break;
	}

	return true;
}


bool execute_current_instruction()
{
	const Cell& current_cell = get_current_instruction_cell();

	if (std::holds_alternative<int>(current_cell))
	{
		const Instruction current_instruction = static_cast<Instruction>(std::get<0>(current_cell) % Instruction::InstructionCount);
		if (!execute_instruction(current_instruction))
		{
			return false;
		}
	}
	else if (std::holds_alternative<OpeningParens>(current_cell))
	{
		std::optional<Coordinates> found_closing_parens = find_closing_parens_for(instruction_cursor.cell_index);
		if (found_closing_parens.has_value())
		{
			instruction_cursor.cell_index = found_closing_parens.value();
		}
		else
		{
			return false;
		}
	}

	return true;
}

int main(const int argc, const char **argv) 
{
	const auto parsed = Arguments::parse(std::span<const char *>(argv, argc));
	if (!parsed.has_value() || !initial_setup(parsed.value()))
	{
		return 1;
	}

	const Arguments::ParseResult result = parsed.value();

	Cursor last_instruction_cursor;
	do 
	{
		last_instruction_cursor = instruction_cursor;
		if (!execute_current_instruction()) 
		{
			return 1;
		}
		cursor_tick();
	} while (!Coordinates::equal(last_instruction_cursor.cell_index, instruction_cursor.cell_index)
			|| !Coordinates::equal(last_instruction_cursor.tensor_index, instruction_cursor.tensor_index));

	return 0;
}