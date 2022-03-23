
#include <gtest/gtest.h>
#include "Emulator/RingBuffer.h"

class RingBufferTests: public ::testing::Test
{
public:
	static constexpr auto ringBufferSize = 32;
	using RingBuffer = utils::RingBuffer<double, ringBufferSize>;
};

TEST_F(RingBufferTests, PushBackUntilCapacity)
{
	RingBuffer rb{};
	ASSERT_EQ(rb.MaxSize(), ringBufferSize);
	for (size_t i = 0; i < rb.MaxSize(); ++i)
	{
		ASSERT_EQ(rb.Size(), i);
		rb.PushBack(static_cast<double>(i));
		ASSERT_EQ(rb.Size(), i + 1);
		ASSERT_DOUBLE_EQ(rb.Back(), i);
		ASSERT_DOUBLE_EQ(rb.Front(), 0);
	}

	ASSERT_DOUBLE_EQ(rb.Back(), rb.Size() - 1);
	ASSERT_DOUBLE_EQ(rb.Front(), 0);
	ASSERT_EQ(rb.Size(), rb.MaxSize());
	for (size_t i = 0; i < rb.Size(); ++i)
		ASSERT_DOUBLE_EQ(i, rb[i]);

	ASSERT_DEATH({ rb.PushBack(1.0); }, "");
}

TEST_F(RingBufferTests, PopFrontUntilEmpty)
{
	RingBuffer rb{};
	for (size_t i = 0; i < rb.MaxSize(); ++i)
		rb.PushBack(static_cast<double>(i));

	for (size_t i = 0; i < rb.MaxSize(); ++i)
	{
		ASSERT_DOUBLE_EQ(rb.Back(), rb.MaxSize() - 1);
		ASSERT_DOUBLE_EQ(rb.Front(), i);

		ASSERT_EQ(rb.Size(), rb.MaxSize() - i);
		ASSERT_DOUBLE_EQ(i, rb.PopFront());
		ASSERT_EQ(rb.Size(), rb.MaxSize() - i - 1);
	}
	ASSERT_EQ(rb.Size(), 0);

	ASSERT_DEATH({ rb.PopFront(); }, "");
	ASSERT_DEATH({ rb.Front(); }, "");
	ASSERT_DEATH({ rb.Back(); }, "");
}

TEST_F(RingBufferTests, AccessWhilePopFrontping)
{
	RingBuffer rb{};
	for (size_t i = 0; i < rb.MaxSize(); ++i)
		rb.PushBack(static_cast<double>(i));

	for (size_t i = 0; i < rb.MaxSize(); ++i)
	{
		ASSERT_DOUBLE_EQ(rb.Back(), rb.MaxSize() - 1);
		ASSERT_DOUBLE_EQ(rb.Front(), i);

		ASSERT_EQ(rb.Size(), rb.MaxSize() - i);
		ASSERT_DOUBLE_EQ(i, rb.PopFront());
		for (size_t j = 0; j < rb.Size(); ++j)
			ASSERT_DOUBLE_EQ(j + i + 1, rb[j]);
		ASSERT_EQ(rb.Size(), rb.MaxSize() - i - 1);
	}
	ASSERT_EQ(rb.Size(), 0);
}

TEST_F(RingBufferTests, AddMoreThanCapacity)
{
	RingBuffer rb{};

	std::vector<double> results;
	for (size_t i = 0; i < 10 * rb.MaxSize(); ++i)
	{
		rb.Add(static_cast<double>(i));
		results.push_back(rb.Back());
	}
	ASSERT_EQ(rb.Size(), rb.MaxSize());

	for (size_t i = results.size() - rb.Size(); i < results.size(); ++i)
		ASSERT_DOUBLE_EQ(results[i], rb[i - (results.size() - rb.Size())]);

	rb.Clear();
	results.clear();
	ASSERT_EQ(rb.Size(), 0);

	for (size_t i = 0; i < 10 * rb.MaxSize(); ++i)
	{
		rb.Add(static_cast<double>(i));
		results.push_back(rb.Back());
	}
	ASSERT_EQ(rb.Size(), rb.MaxSize());

	for (size_t i = results.size() - rb.Size(); i < results.size(); ++i)
		ASSERT_DOUBLE_EQ(results[i], rb[i - (results.size() - rb.Size())]);
}
