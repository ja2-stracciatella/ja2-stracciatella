#ifdef WITH_UNITTESTS

#include "Views.h"

#include "gtest/gtest.h"
#include <cstdint>
#include <memory>
#include <utility>

class TestEntity : public Containers::NamedEntity<uint8_t> {
	public:
		TestEntity(uint8_t id, ST::string&& internalName, bool inView) : m_id(id), m_internalName(std::move(internalName)), m_inView(inView) {}
		virtual ~TestEntity() {}

		static constexpr const char* ENTITY_NAME = "TestEntity";
		uint8_t getId() const override { return m_id; }
		const ST::string& getInternalName() const override { return m_internalName; }
		bool inView() const { return m_inView; }
	private:
		uint8_t m_id;
		ST::string m_internalName;
		bool m_inView;
};

TEST(Views, Indexed)
{
	auto container = Containers::Indexed<uint8_t, TestEntity>();
	container.add(std::make_unique<TestEntity>(1, "e1", false));
	container.add(std::make_unique<TestEntity>(2, "e2", true));
	container.add(std::make_unique<TestEntity>(3, "e3", true));
	container.add(std::make_unique<TestEntity>(4, "e4", false));
	container.add(std::make_unique<TestEntity>(5, "e5", true));
	auto view = Containers::Views::Indexed<uint8_t, TestEntity>(
		container.begin(),
		container.end(),
		[](const TestEntity* entity) { return entity->inView() ? entity : nullptr; }
	);

	ASSERT_EQ(view.size(), 3);

	ASSERT_EQ(view.byId(2)->getId(), 2);
	ASSERT_EQ(view.byId(2)->getInternalName(), "e2");
	EXPECT_THROW({ view.byId(1); }, NotFoundError);

	ASSERT_EQ(view.optionalById(2)->getId(), 2);
	ASSERT_EQ(view.optionalById(2)->getInternalName(), "e2");
	ASSERT_EQ(view.optionalById(1), nullptr);
}

TEST(Views, Named)
{
	auto container = Containers::Named<uint8_t, TestEntity>();
	container.add(std::make_unique<TestEntity>(1, "e1", false));
	container.add(std::make_unique<TestEntity>(2, "e2", true));
	container.add(std::make_unique<TestEntity>(3, "e3", true));
	container.add(std::make_unique<TestEntity>(4, "e4", false));
	container.add(std::make_unique<TestEntity>(5, "e5", true));
	auto view = Containers::Views::Named<uint8_t, TestEntity>(\
		container.begin(),
		container.end(),
		[](const TestEntity* entity) { return entity->inView() ? entity : nullptr; }
	);

	ASSERT_EQ(view.size(), 3);

	ASSERT_EQ(view.byName("e2")->getId(), 2);
	ASSERT_EQ(view.byName("e2")->getInternalName(), "e2");
	EXPECT_THROW({ view.byName("e1"); }, NotFoundError);

	ASSERT_EQ(view.optionalByName("e2")->getId(), 2);
	ASSERT_EQ(view.optionalByName("e2")->getInternalName(), "e2");
	ASSERT_EQ(view.optionalByName("e1"), nullptr);
}

TEST(Views, Iterator)
{
	auto container = Containers::Indexed<uint8_t, TestEntity>();
	container.add(std::make_unique<TestEntity>(1, "e1", false));
	container.add(std::make_unique<TestEntity>(2, "e2", true));
	container.add(std::make_unique<TestEntity>(3, "e3", true));
	container.add(std::make_unique<TestEntity>(4, "e4", false));
	container.add(std::make_unique<TestEntity>(5, "e5", true));
	auto view = Containers::Views::Indexed<uint8_t, TestEntity>(
		container.begin(),
		container.end(),
		[](const TestEntity* entity) { return entity->inView() ? entity : nullptr; }
	);

	// Test basic iteration with iterators
	auto it = view.begin();
	ASSERT_NE(it, view.end());

	// Test dereferencing - should return const Model* not unique_ptr
	const TestEntity* first = *it;
	ASSERT_EQ(first->getId(), 2);
	ASSERT_EQ(first->getInternalName(), "e2");

	// Test arrow operator
	ASSERT_EQ(it->getId(), 2);
	ASSERT_EQ(it->getInternalName(), "e2");

	// Test pre-increment
	++it;
	ASSERT_EQ(it->getId(), 3);
	ASSERT_EQ(it->getInternalName(), "e3");

	// Test post-increment
	auto old_it = it++;
	ASSERT_EQ(old_it->getId(), 3);
	ASSERT_EQ(it->getId(), 5);

	// Test increment to end
	++it;
	ASSERT_EQ(it, view.end());
}

TEST(Views, IteratorRange)
{
	auto container = Containers::Indexed<uint8_t, TestEntity>();
	container.add(std::make_unique<TestEntity>(1, "e1", false));
	container.add(std::make_unique<TestEntity>(2, "e2", true));
	container.add(std::make_unique<TestEntity>(3, "e3", true));
	container.add(std::make_unique<TestEntity>(4, "e4", false));
	container.add(std::make_unique<TestEntity>(5, "e5", true));
	auto view = Containers::Views::Indexed<uint8_t, TestEntity>(
		container.begin(),
		container.end(),
		[](const TestEntity* entity) { return entity->inView() ? entity : nullptr; }
	);

	// Test range-based for loop
	std::vector<std::pair<uint8_t, ST::string>> results;

	for (const TestEntity* entity : view) {
		results.push_back(std::make_pair(entity->getId(), entity->getInternalName()));
	}

	ASSERT_EQ(results.size(), 3);
	ASSERT_EQ(results[0].first, 2);
	ASSERT_EQ(results[0].second, "e2");
	ASSERT_EQ(results[1].first, 3);
	ASSERT_EQ(results[1].second, "e3");
	ASSERT_EQ(results[2].first, 5);
	ASSERT_EQ(results[2].second, "e5");
}

#endif
