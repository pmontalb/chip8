
#include <gtest/gtest.h>
#include "Emulator/RingBuffer.h"

#include "IgnoreWarning.h"

class RingBufferTests: public ::testing::Test
{
public:
	static constexpr auto ringBufferSize = 32;
	using RingBuffer = utils::RingBuffer<size_t, ringBufferSize>;
};

TEST_F(RingBufferTests, PushBackUntilCapacity)
{
	RingBuffer rb{};
	ASSERT_EQ(rb.MaxSize(), ringBufferSize);
	for (size_t i = 0; i < rb.MaxSize(); ++i)
	{
		ASSERT_EQ(rb.Size(), i);
		rb.PushBack(i);
		ASSERT_EQ(rb.Size(), i + 1);
		ASSERT_EQ(rb.Back(), i);
		ASSERT_EQ(rb.Front(), 0);
	}

	ASSERT_EQ(rb.Back(), rb.Size() - 1);
	ASSERT_EQ(rb.Front(), 0);
	ASSERT_EQ(rb.Size(), rb.MaxSize());
	for (size_t i = 0; i < rb.Size(); ++i)
		ASSERT_EQ(i, rb[i]);

	__START_IGNORING_WARNINGS__
#ifdef __clang__
	__IGNORE_WARNING__("-Wused-but-marked-unused")
#endif
	ASSERT_DEATH({ rb.PushBack(1ul); }, "");
	__STOP_IGNORING_WARNINGS__
}

TEST_F(RingBufferTests, PopFrontUntilEmpty)
{
	RingBuffer rb{};
	for (size_t i = 0; i < rb.MaxSize(); ++i)
		rb.PushBack(i);

	for (size_t i = 0; i < rb.MaxSize(); ++i)
	{
		ASSERT_EQ(rb.Back(), rb.MaxSize() - 1);
		ASSERT_EQ(rb.Front(), i);

		ASSERT_EQ(rb.Size(), rb.MaxSize() - i);
		ASSERT_EQ(i, rb.PopFront());
		ASSERT_EQ(rb.Size(), rb.MaxSize() - i - 1);
	}
	ASSERT_EQ(rb.Size(), 0);

	__START_IGNORING_WARNINGS__
#ifdef __clang__
	__IGNORE_WARNING__("-Wused-but-marked-unused")
#endif
	ASSERT_DEATH({ rb.PopFront(); }, "");
	ASSERT_DEATH({ auto x = rb.Front(); std::cout << x; }, "");
	ASSERT_DEATH({ auto x = rb.Back(); std::cout << x; }, "");
	__STOP_IGNORING_WARNINGS__
}

TEST_F(RingBufferTests, AccessWhilePopFrontping)
{
	RingBuffer rb{};
	for (size_t i = 0; i < rb.MaxSize(); ++i)
		rb.PushBack(i);

	for (size_t i = 0; i < rb.MaxSize(); ++i)
	{
		ASSERT_EQ(rb.Back(), rb.MaxSize() - 1);
		ASSERT_EQ(rb.Front(), i);

		ASSERT_EQ(rb.Size(), rb.MaxSize() - i);
		ASSERT_EQ(i, rb.PopFront());
		for (size_t j = 0; j < rb.Size(); ++j)
			ASSERT_EQ(j + i + 1, rb[j]);
		ASSERT_EQ(rb.Size(), rb.MaxSize() - i - 1);
	}
	ASSERT_EQ(rb.Size(), 0);
}

TEST_F(RingBufferTests, AddMoreThanCapacity)
{
	RingBuffer rb{};

	std::vector<size_t> results;
	for (size_t i = 0; i < 10 * rb.MaxSize(); ++i)
	{
		rb.Add(i);
		results.push_back(rb.Back());
	}
	ASSERT_EQ(rb.Size(), rb.MaxSize());

	for (size_t i = results.size() - rb.Size(); i < results.size(); ++i)
		ASSERT_EQ(results[i], rb[i - (results.size() - rb.Size())]);

	rb.Clear();
	results.clear();
	ASSERT_EQ(rb.Size(), 0);

	for (size_t i = 0; i < 10 * rb.MaxSize(); ++i)
	{
		rb.Add(i);
		results.push_back(rb.Back());
	}
	ASSERT_EQ(rb.Size(), rb.MaxSize());

	for (size_t i = results.size() - rb.Size(); i < results.size(); ++i)
		ASSERT_EQ(results[i], rb[i - (results.size() - rb.Size())]);
}
