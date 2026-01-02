#include <utility>
#ifdef WITH_UNITTESTS

#include "Containers.h"

#include "gtest/gtest.h"
#include <cstdint>
#include <memory>

class TestEntity : public Containers::NamedEntity<uint8_t> {
	public:
		TestEntity(uint8_t id, ST::string&& internalName) : m_id(id), m_internalName(std::move(internalName)) {}

		static constexpr const char* ENTITY_NAME = "TestEntity";
		uint8_t getId() const override { return m_id; }
		const ST::string& getInternalName() const override { return m_internalName; }
	private:
		uint8_t m_id;
		ST::string m_internalName;
};

TEST(Containers, Indexed)
{
	auto container = Containers::Indexed<uint8_t, TestEntity>();

	container.add(std::make_unique<TestEntity>(1, "e1"));
	container.add(std::make_unique<TestEntity>(2, "e2"));
	container.add(std::make_unique<TestEntity>(3, "e3"));

	ASSERT_EQ(container.size(), 3);

	ASSERT_EQ(container.byId(2)->getId(), 2);
	ASSERT_EQ(container.byId(2)->getInternalName(), "e2");
	EXPECT_THROW({ container.byId(5); }, NotFoundError);

	ASSERT_EQ(container.optionalById(2)->getId(), 2);
	ASSERT_EQ(container.optionalById(2)->getInternalName(), "e2");
	ASSERT_EQ(container.optionalById(5), nullptr);
}

TEST(Containers, Named)
{
	auto container = Containers::Named<uint8_t, TestEntity>();

	container.add(std::make_unique<TestEntity>(1, "e1"));
	container.add(std::make_unique<TestEntity>(2, "e2"));
	container.add(std::make_unique<TestEntity>(3, "e3"));

	ASSERT_EQ(container.size(), 3);

	ASSERT_EQ(container.byName("e2")->getId(), 2);
	ASSERT_EQ(container.byName("e2")->getInternalName(), "e2");
	EXPECT_THROW({ container.byName("nothing"); }, NotFoundError);

	ASSERT_EQ(container.optionalByName("e2")->getId(), 2);
	ASSERT_EQ(container.optionalByName("e2")->getInternalName(), "e2");
	ASSERT_EQ(container.optionalByName("nothing"), nullptr);
}

TEST(Containers, ContainerIterator)
{
	auto container = Containers::Indexed<uint8_t, TestEntity>();

	container.add(std::make_unique<TestEntity>(1, "e1"));
	container.add(std::make_unique<TestEntity>(2, "e2"));
	container.add(std::make_unique<TestEntity>(3, "e3"));

	// Test basic iteration with iterators
	auto it = container.begin();
	ASSERT_NE(it, container.end());

	// Test dereferencing - should return const Model* not unique_ptr
	const TestEntity* first = *it;
	ASSERT_EQ(first->getId(), 1);
	ASSERT_EQ(first->getInternalName(), "e1");

	// Test arrow operator
	ASSERT_EQ(it->getId(), 1);
	ASSERT_EQ(it->getInternalName(), "e1");

	// Test pre-increment
	++it;
	ASSERT_EQ(it->getId(), 2);
	ASSERT_EQ(it->getInternalName(), "e2");

	// Test post-increment
	auto old_it = it++;
	ASSERT_EQ(old_it->getId(), 2);
	ASSERT_EQ(it->getId(), 3);

	// Test increment to end
	++it;
	ASSERT_EQ(it, container.end());
}

TEST(Containers, ContainerIteratorRange)
{
	auto container = Containers::Indexed<uint8_t, TestEntity>();

	container.add(std::make_unique<TestEntity>(10, "test1"));
	container.add(std::make_unique<TestEntity>(20, "test2"));
	container.add(std::make_unique<TestEntity>(30, "test3"));

	// Test range-based for loop
	std::vector<std::pair<uint8_t, ST::string>> results;

	for (const TestEntity* entity : container) {
		results.push_back(std::make_pair(entity->getId(), entity->getInternalName()));
	}

	ASSERT_EQ(results.size(), 3);
	ASSERT_EQ(results[0].first, 10);
	ASSERT_EQ(results[0].second, "test1");
	ASSERT_EQ(results[1].first, 20);
	ASSERT_EQ(results[1].second, "test2");
	ASSERT_EQ(results[2].first, 30);
	ASSERT_EQ(results[2].second, "test3");
}

#endif
