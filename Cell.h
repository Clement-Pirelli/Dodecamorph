#pragma once

struct OpeningParens {};
struct ClosingParens {};

enum Instruction : int
{
	OutputCurrentData = 0,
	IncrementDataCursorCellIndex = 1,
	SetDataCursorTensorIndex = 2,
	IncrementDataCell = 3,
	DecrementDataCell = 4,
	SetInstructionCursorDirection = 5,
	SetDataCellUserInput = 6,
	ConditionalSetInstructionCursorCellIndex = 7,
	SetDataCellOpeningParens = 8,
	SetDataCellClosingParens = 9,
	SetInstructionCursorTensorIndex = 10,
	ShrinkTensor = 11,
	InstructionCount
};

using Cell = std::variant<int, OpeningParens, ClosingParens>;