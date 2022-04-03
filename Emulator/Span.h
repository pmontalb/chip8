
#pragma once

#include <cassert>
#include <cstddef>
#include <array>

namespace utils
{
	// use std::span in c++20
	template<typename T>
	class Span
	{
	public:
		template<typename Container>
		constexpr Span(Container&& container) : _data(&(*std::begin(container))), _length(std::size(container))
		{
			// we can only support lvalue references!
			static_assert(std::is_rvalue_reference_v<decltype(container)> == false);
		}
		template<template<typename, typename ...> typename Container, typename... Args>
		constexpr Span(const Container<T, Args...>& container) : _data(&(*std::begin(container))), _length(std::size(container))
		{
		}
		template<template<typename, std::size_t> typename Container, std::size_t N>
		constexpr Span(const Container<T, N>& container) : _data(&(*std::begin(container))), _length(std::size(container))
		{
		}

		Span() = default;
		~Span() = default;
		Span(const Span& rhs) = default;
		Span(Span&& rhs) noexcept = default;
		Span& operator=(const Span& rhs) = default;
		Span& operator=(Span&& rhs) noexcept = default;

		template<typename Iter>
		constexpr Span(const Iter& begin, const Iter& end)
			: _data(&(*begin)), _length(static_cast<std::size_t>(end - begin))
		{
			assert(end >= begin);
		}

		[[nodiscard]] constexpr auto Size() const { return _length; }

		[[nodiscard]] constexpr T* begin() const { return _data; }
		[[nodiscard]] constexpr T* begin() { return _data; }

		[[nodiscard]] constexpr T* end() const { return _data + _length; }
		[[nodiscard]] constexpr T* end() { return _data + _length; }
		[[nodiscard]] constexpr auto size() const noexcept { return _length; }

		[[nodiscard]] constexpr const T& operator[](const std::size_t i) const
		{
			assert(i < _length);
			return _data[i];
		}
		[[nodiscard]] constexpr T& operator[](const std::size_t i)
		{
			assert(i < _length);
			return _data[i];
		}

	private:
		T* _data = nullptr;
		std::size_t _length = 0;
	};

	template<typename T, std::size_t N>
	Span(std::array<T, N>) -> Span<T>;
}	 // namespace utils
