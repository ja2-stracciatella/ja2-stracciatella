#pragma once

#include "Exceptions.h"

#include <cstddef>
#include <map>
#include <memory>
#include <string_theory/format>
#include <string_theory/string>
#include <type_traits>
#include <utility>
#include <vector>

namespace Containers {
	template<typename Id> class Entity {
		static_assert(std::is_integral_v<Id>, "Id must be an integral type for Containers::Entity");

		public:
			virtual ~Entity() {}
			virtual Id getId() const = 0;
	};

	template<typename Id> class NamedEntity : public Entity<Id> {
		public:
			virtual ~NamedEntity() {}
			virtual const ST::string& getInternalName() const = 0;
	};

	template<typename Id, typename Model> class Indexed {
		static_assert(std::is_base_of_v<Entity<Id>, Model>, "Model must be a subclass of Entity for Containers::Indexed or Containers::Named");
		static_assert(std::is_pointer_v<decltype(Model::ENTITY_NAME)>, "Model must have a static entityName method for Containers::Indexed or Containers::Named");

		public:
			Indexed() = default;

			const std::vector<std::unique_ptr<Model>>& all() const {
				return m_models;
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
				return m_models.at(index->second).get();
			}

			void add(std::unique_ptr<Model>&& model) {
				auto id = model->getId();
				auto index = m_models.size();
				if (m_ids.find(id) != m_ids.end()) {
					throw DataError(ST::format("duplicate numeric id `{}` for entity `{}`", id, Model::ENTITY_NAME));
				}
				m_models.push_back(std::move(model));
				m_ids.emplace(id, index);
			}
		protected:
			std::vector<std::unique_ptr<Model>> m_models;
			std::map<Id, std::size_t> m_ids;
	};

	template<typename Id, typename Model> class Named : public Indexed<Id, Model> {
		static_assert(std::is_base_of_v<NamedEntity<Id>, Model>, "Model must be a subclass of NamedEntity for Containers::Named");

		public:
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
				return this->m_models.at(index->second).get();
			}

			void add(std::unique_ptr<Model>&& model) {
				auto index = this->m_models.size();
				ST::string name = model->getInternalName();
				if (m_names.find(name) != m_names.end()) {
					throw DataError(ST::format("duplicate internal name `{}` for entity `{}`", name, Model::ENTITY_NAME));
				}
				Indexed<Id, Model>::add(std::move(model));
				m_names.emplace(name, index);
			}
		private:
			std::map<ST::string, size_t> m_names;
	};
}
