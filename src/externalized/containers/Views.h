#pragma once

#include "Containers.h"
#include <cstddef>

namespace Containers {
	namespace Views {
		template<typename Id, typename Model> class ViewIterator {
			static_assert(std::is_base_of_v<Entity<Id>, Model>, "Model must be a subclass of Entity for Containers::Views::ViewIterator");

			public:
				using iterator_category = std::bidirectional_iterator_tag;
				using value_type = const Model*;
				using difference_type = std::ptrdiff_t;
				using pointer = const Model**;
				using reference = const Model*&;

				ViewIterator(typename std::vector<const Model*>::const_iterator it) : m_it(it) {}

				const Model* operator*() const {
					return *m_it;
				}

				const Model* operator->() const {
					return *m_it;
				}

				ViewIterator& operator++() {
					++m_it;
					return *this;
				}

				ViewIterator operator++(int) {
					ViewIterator tmp = *this;
					++m_it;
					return tmp;
				}

				ViewIterator& operator--() {
					--m_it;
					return *this;
				}

				ViewIterator operator--(int) {
					ViewIterator tmp = *this;
					--m_it;
					return tmp;
				}

				bool operator==(const ViewIterator& other) const {
					return m_it == other.m_it;
				}

				bool operator!=(const ViewIterator& other) const {
					return m_it != other.m_it;
				}

				bool operator<(const ViewIterator& other) const {
					return m_it < other.m_it;
				}

				bool operator>(const ViewIterator& other) const {
					return m_it > other.m_it;
				}

			private:
				typename std::vector<const Model*>::const_iterator m_it;
		};

		template<typename Id, typename Model, typename Base = Model> class Indexed {
			static_assert(std::is_base_of_v<NamedEntity<Id>, Base>, "Model must be a subclass of NamedEntity for Containers::View::Indexed or Containers::View::Named");
			static_assert(std::is_base_of_v<Base, Model>, "Model must be a subclass of Base for Containers::View::Indexed or Containers::View::Named");
			static_assert(std::is_pointer_v<decltype(Base::ENTITY_NAME)>, "Base must have a static constexpr ENTITY_NAME for Containers::View::Indexed or Containers::View::Named");
			static_assert(std::is_pointer_v<decltype(Model::ENTITY_NAME)>, "Model must have a static constexpr ENTITY_NAME for Containers::View::Indexed or Containers::View::Named");

			public:
				Indexed() = default;
				Indexed(
					Containers::ContainerIterator<Id, Base> begin,
					Containers::ContainerIterator<Id, Base> end,
					std::function<const Model*(const Base*)> filter
				) {
					for (auto it = begin; it != end; it++) {
						auto m = filter(*it);
						if (m) {
							auto index = m_models.size();
							auto id = m->getId();
							auto name = m->getInternalName();

							m_models.push_back(m);
							m_ids.insert(std::make_pair(id, index));
						}
					}
				}

				using iterator = ViewIterator<Id, Model>;

				size_t size() const {
					return m_models.size();
				}

				iterator begin() const {
					return iterator(m_models.begin());
				}

				iterator end() const {
					return iterator(m_models.end());
				}

				const Model* byId(Id id) const {
					auto model = optionalById(id);
					if (!model) {
						throw NotFoundError(ST::format("entity {}({}) not found", Model::ENTITY_NAME, id));
					}
					return model;
				}

				const Model* optionalById(Id id) const {
					auto index = m_ids.find(id);
					if (index == m_ids.end()) {
						return nullptr;
					}
					return m_models.at(index->second);
				}

			protected:
				std::vector<const Model*> m_models;
				std::map<Id, size_t> m_ids;
		};

		template<typename Id, typename Model, typename Base = Model> class Named : public Indexed<Id, Model, Base> {
			public:
				Named() = default;
				Named(
					Containers::ContainerIterator<Id, Base> begin,
					Containers::ContainerIterator<Id, Base> end,
					std::function<const Model*(const Base*)> filter
				) : Indexed<Id, Model, Base>(begin, end, filter) {
					size_t index = 0;
					for (const Model* model : this->m_models) {
						m_names.emplace(model->getInternalName(), index);
						index++;
					}
				}

				const Model* byName(const ST::string& internalName) const {
					auto model = optionalByName(internalName);
					if (!model) {
						throw NotFoundError(ST::format("entity {}(\"{}\") not found", Model::ENTITY_NAME, internalName));
					}
					return model;
				}

				const Model* optionalByName(const ST::string& internalName) const {
					auto index = m_names.find(internalName);
					if (index == m_names.end()) {
						return nullptr;
					}
					return this->m_models.at(index->second);
				}

			protected:
				std::map<ST::string, size_t> m_names;
		};
	}
}
