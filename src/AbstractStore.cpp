#include "tp_data_store/AbstractStore.h"

#include "tp_data/Collection.h"

namespace tp_data_store
{

//##################################################################################################
AbstractStore::AbstractStore(const tp_data::CollectionFactory* collectionFactory):
  m_collectionFactory(collectionFactory)
{

}

//##################################################################################################
AbstractStore::~AbstractStore() = default;

//##################################################################################################
const tp_data::CollectionFactory* AbstractStore::collectionFactory() const
{
  return m_collectionFactory;
}

//##################################################################################################
void AbstractStore::add(const std::string& name,
                        tp_data::AbstractMember* member)
{
  tp_data::Collection collection;
  collection.addMember(member);
  add(name, collection);
}

}
