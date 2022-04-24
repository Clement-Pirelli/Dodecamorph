#pragma once
#include <string>
#include <optional>
#include "Tensor.h"
#include "Cell.h"

namespace InputFile
{
	std::optional<Tensor<Cell>> parse(std::string contents);
}

