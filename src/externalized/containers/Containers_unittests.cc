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

#endif
