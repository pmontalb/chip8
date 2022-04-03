
#pragma once

#include <array>
#include <cassert>

namespace utils
{
	template<typename T, std::size_t N>
	class RingBuffer
	{
	public:
		[[nodiscard]] constexpr std::size_t MaxSize() const { return N; }

		[[nodiscard]] std::size_t Size() const { return _size; }

		template<typename S>
		void PushBack(S&& item)
		{
			assert(Size() < MaxSize());

			_data[_tail] = std::forward<S>(item);
			++_tail;
			if (_tail >= MaxSize())
				_tail = 0;
			++_size;
		}
		T PopFront()
		{
			assert(Size() > 0);

			auto item = std::exchange(_data[_head], T {});

			++_head;
			if (_head >= MaxSize())
				_head = 0;
			--_size;

			return item;
		}

		template<typename S>
		void Add(S&& item)
		{
			if (Size() == MaxSize())
				PopFront();
			PushBack(std::forward<S>(item));
		}

		void Clear()
		{
			_size = 0;
			_head = 0;
			_tail = 0;
			std::fill(_data.begin(), _data.end(), T {});
		}

		[[nodiscard]] T& operator[](const std::size_t i)
		{
			assert(Size() > 0);
			if (_tail > _head)
			{
				assert(_head + i <= _tail);
				return _data[_head + i];
			}
			if (_head + i < MaxSize())
				return _data[_head + i];
			assert(_head + i - MaxSize() <= _tail);
			return _data[_head + i - MaxSize()];
		}
		const T& operator[](const std::size_t i) const { return this->operator[](i); }

		[[nodiscard]] T& Front() { return this->operator[](0); }
		[[nodiscard]] T& Back() { return this->operator[](Size() - 1); }

	private:
		std::array<T, N> _data {};
		std::size_t _head = 0;
		std::size_t _tail = 0;
		std::size_t _size = 0;
	};
}	 // namespace utils
