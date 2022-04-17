
#include <gtest/gtest.h>
#include "Emulator/Span.h"

#include "IgnoreWarning.h"

class SpanTests: public ::testing::Test
{
};

TEST_F(SpanTests, CreationFromStdContainer)
{
	{
		std::vector<double> v(10);
		for (size_t i = 0; i < v.size(); ++i)
			v[i] = static_cast<double>(i * (i + 1)) + 1.0;
		utils::Span span { v };

		size_t counter = 0;
		for (const auto& element : span)
			ASSERT_DOUBLE_EQ(element, v[counter++]);
	}

	{
		std::array<int, 5> a = { { 1, 5, 42, 7, 6 } };

		utils::Span span { a };

		size_t counter = 0;
		for (const auto& element : span)
			ASSERT_DOUBLE_EQ(element, a[counter++]);
	}
}

TEST_F(SpanTests, RandomAccess)
{
	std::vector<double> v(10);
	for (size_t i = 0; i < v.size(); ++i)
		v[i] = static_cast<double>(i * (i + 1)) + 1.0;
	utils::Span span { v };
	for (size_t i = 0; i < v.size(); ++i)
		ASSERT_DOUBLE_EQ(span[i], v[i]);
}

TEST_F(SpanTests, SubView)
{
	std::vector<double> v(10);
	for (size_t i = 0; i < v.size(); ++i)
		v[i] = static_cast<double>(i * (i + 1)) + 1.0;
	utils::Span<double> span { v.begin() + 2, v.begin() + 4 };

	size_t counter = 0;
	for (const auto& element : span)
		ASSERT_DOUBLE_EQ(element, v[2 + counter++]);
}

TEST_F(SpanTests, MultipleSpans)
{
	std::vector<double> v(10);
	for (size_t i = 0; i < v.size(); ++i)
		v[i] = static_cast<double>(i * (i + 1)) + 1.0;
	utils::Span<double> span1 { v.begin() + 2, v.begin() + 4 };
	utils::Span<double> span2 { v.begin() + 1, v.begin() + 7 };

	size_t counter1 = 0;
	for (const auto& element1 : span1)
	{
		ASSERT_DOUBLE_EQ(element1, v[2 + counter1++]);

		size_t counter2 = 0;
		for (const auto& element2 : span2)
			ASSERT_DOUBLE_EQ(element2, v[1 + counter2++]);
	}
}

TEST_F(SpanTests, CopySpan)
{
	std::vector<double> v(10);
	for (size_t i = 0; i < v.size(); ++i)
		v[i] = static_cast<double>(i * (i + 1)) + 1.0;
	utils::Span<double> span1 { v.begin() + 2, v.begin() + 4 };
	utils::Span<double> span2(span1);

	size_t counter = 0;
	for (const auto& element : span1)
		ASSERT_DOUBLE_EQ(element, span2[counter++]);
}

TEST_F(SpanTests, AssignSpan)
{
	std::vector<double> v(10);
	for (size_t i = 0; i < v.size(); ++i)
		v[i] = static_cast<double>(i * (i + 1)) + 1.0;
	utils::Span<double> span1 { v.begin() + 2, v.begin() + 4 };
	utils::Span<double> span2;
	span2 = span1;

	size_t counter = 0;
	for (const auto& element : span1)
		ASSERT_DOUBLE_EQ(element, span2[counter++]);
}

TEST_F(SpanTests, MoveSpan)
{
	std::vector<double> v(10);
	for (size_t i = 0; i < v.size(); ++i)
		v[i] = static_cast<double>(i * (i + 1)) + 1.0;
	utils::Span<double> span1 { v.begin() + 2, v.begin() + 4 };
	utils::Span<double> span2(utils::Span<double> { v.begin() + 2, v.begin() + 4 });

	size_t counter = 0;
	for (const auto& element : span1)
		ASSERT_DOUBLE_EQ(element, span2[counter++]);

	span2 = std::move(span1);
	counter = 0;
	for (const auto& element : span2)
		ASSERT_DOUBLE_EQ(element, v[2 + counter++]);
}

TEST_F(SpanTests, MoveAssignSpan)
{
	std::vector<double> v(10);
	for (size_t i = 0; i < v.size(); ++i)
		v[i] = static_cast<double>(i * (i + 1)) + 1.0;
	utils::Span<double> span1 { v.begin() + 2, v.begin() + 4 };
	utils::Span<double> span2;
	span2 = utils::Span<double> { v.begin() + 2, v.begin() + 4 };

	size_t counter = 0;
	for (const auto& element : span1)
		ASSERT_DOUBLE_EQ(element, span2[counter++]);
}
