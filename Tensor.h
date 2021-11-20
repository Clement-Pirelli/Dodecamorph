#pragma once
#include <vector>
#include <utility>
#include <algorithm>
#include <span>
#include <variant>


struct Coordinates 
{
	Coordinates(std::vector<int>&& v) : value(std::move(v)) {}
	Coordinates(const std::vector<int>& v) : value(v) {}
	Coordinates(const int sole) : value({sole}) {}
	Coordinates(const int first, const int second) : value({first, second}) {}
	Coordinates() = default;

	int& operator[](size_t index) { return value[index]; }
	int operator[](size_t index) const { return value[index]; }

	typename std::vector<int>::iterator begin() { return value.begin(); }
	typename std::vector<int>::iterator end() { return value.end(); }
	typename std::vector<int>::const_iterator begin() const { return value.begin(); }
	typename std::vector<int>::const_iterator end() const { return value.end(); }

	size_t size() const { return value.size(); }

	static bool equal(const Coordinates& lhs, const Coordinates& rhs)
	{
		const size_t lhs_size = lhs.size();
		const size_t rhs_size = rhs.size();
		if (lhs_size < rhs_size)
		{
			for (size_t i = 0; i < rhs_size; i++)
			{
				const int other_coordinate = i >= lhs_size ? 0 : lhs[i];
				if (rhs[i] != other_coordinate)
				{
					return false;
				}
			}
		}
		else if (lhs_size > rhs_size)
		{
			for (size_t i = 0; i < lhs_size; i++)
			{
				const int other_coordinate = i >= rhs_size ? 0 : rhs[i];
				if (lhs[i] != other_coordinate)
				{
					return false;
				}
			}
		}
		else
		{
			return std::equal(lhs.begin(), lhs.end(), rhs.begin());
		}

		return true;
	}

	Coordinates& increment(const Coordinates& by, const std::span<const int> dimensions)
	{
		const size_t coords_size = size();
		const size_t by_size = by.size();	

		const auto get_dimension_for_index = [&dimensions](size_t index)
		{
			return dimensions.size() > index ? dimensions[index] : 1;
		};

		if (coords_size < by_size)
		{
			for (size_t i = 0; i < by_size; i++)
			{
				const int dimension = get_dimension_for_index(i);
				if (coords_size > i)
				{
					value[i] = (value[i] + by[i]) % dimension;
				}
				else
				{
					value.push_back(by[i] % dimension);
				}
			}
		}
		else if (coords_size > by_size)
		{
			for (size_t i = 0; i < coords_size; i++)
			{
				const int by_value = by_size > i ? by[i] : 0;
				const int dimension = get_dimension_for_index(i);
				value[i] = (value[i] + by_value) % dimension;
			}
		}
		else 
		{
			for (size_t i = 0; i < coords_size; i++)
			{
				const int dimension = get_dimension_for_index(i);
				value[i] = (value[i] + by[i]) % dimension;
			}
		}

		return *this;
	}

private:

	std::vector<int> value;
};


template<typename T>
class Tensor;

template<typename T>
class TensorHandle 
{
	std::variant<size_t, Coordinates> index_or_coordinate;
	
	bool invalid() const
	{
		return !std::holds_alternative<size_t>(index_or_coordinate);
	}
	
	TensorHandle(size_t givenIndex) : index_or_coordinate(givenIndex) {}
	TensorHandle(const Coordinates& coords) : index_or_coordinate(coords) {}
	TensorHandle(Coordinates&& coords) : index_or_coordinate(std::move(coords)) {}

	friend class Tensor<T>;
};


template<typename T>
class Tensor
{
private:
	struct Element
	{
		T value;
		Coordinates coordinates;
	};

	std::vector<int> dimensions;
	std::vector<Element> elements;

	size_t indexAt(const Coordinates& coordinates)
	{
		if (dimensions.size() < coordinates.size())
		{
			dimensions.resize(coordinates.size());
		}

		for (size_t i = 0; i < coordinates.size(); i++)
		{
			dimensions[i] = std::max(dimensions[i], coordinates[i]+1);
		}

		auto all_coordinates_equal = [&coordinates](const Tensor<T>::Element &element)
		{
			return Coordinates::equal(element.coordinates, coordinates);
		};

		const auto pos = std::find_if(elements.begin(), elements.end(), all_coordinates_equal);

		return std::distance(elements.begin(), pos);
	}
	
public:

	const std::vector<int>& getDimensions() { return dimensions; }

	TensorHandle<T> handleAtCoordinates(const Coordinates& coordinates)
	{
		const size_t index = indexAt(coordinates);
		if (index == elements.size()) 
		{
			return TensorHandle<T>(coordinates);
		}
		else 
		{
			return TensorHandle<T>(index);
		}
	}

	T& at(TensorHandle<T>& handle) 
	{
		if (handle.invalid()) 
		{
			const size_t new_index = elements.size();
			elements.emplace_back(T(), std::get<1>(handle.index_or_coordinate)); //todo: what happens if we call at with an invalid handle on a tensor which has shrunk?
			handle = TensorHandle<T>(new_index);
			return elements.back().value;
		}
		else 
		{
			return elements[std::get<0>(handle.index_or_coordinate)].value;
		}
	}

	T& at(const Coordinates& coordinates) 
	{
		TensorHandle<T> temp_handle = handleAtCoordinates(coordinates);
		return at(temp_handle);
	}

	void setAtCoordinates(const Coordinates& coordinates, const T& t)
	{
		TensorHandle<T> handle = handleAtCoordinates(coordinates);
		at(handle) = t;
	}

	void shrink() 
	{
		dimensions.clear();
		dimensions.push_back(1u);
		elements.clear();
	}

	Tensor() : elements({ Element{ T(), Coordinates(0) } }), dimensions(1u) {}
};

