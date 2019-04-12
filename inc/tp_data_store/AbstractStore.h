#ifndef tp_data_store_AbstractStore_h
#define tp_data_store_AbstractStore_h

#include "tp_data_store/Globals.h"

#include "tp_data/AbstractMember.h"

namespace tp_data
{
class Collection;
class CollectionFactory;
class AbstractMember;
}

namespace tp_data_store
{

//##################################################################################################
//! The base class for data stores.
class AbstractStore
{
public:
  //################################################################################################
  AbstractStore(const tp_data::CollectionFactory* collectionFactory);

  //################################################################################################
  virtual ~AbstractStore();

  //################################################################################################
  const tp_data::CollectionFactory* collectionFactory() const;

  //################################################################################################
  //! Add members to a new or existing collection.
  virtual void add(const std::string& name,
                   const tp_data::Collection& collection) = 0;

  //################################################################################################
  //! Add member to a new or existing collection.
  /*!
  This will add a member to a collection, it will create the collection if required.

  \param name - The name of the collection to add to.
  \param member - The member to add to the collection, this will take ownership.
  */
  void add(const std::string& name,
           tp_data::AbstractMember* member);

  //################################################################################################
  //! Remove a collection.
  virtual void remove(const std::string& name) = 0;

  //################################################################################################
  //! Fetch a collection.
  virtual void fetch(const std::string& name,
                     tp_data::Collection& collection,
                     const std::vector<std::string>& subset=std::vector<std::string>()) = 0;

  //################################################################################################
  //! View the list of collection names that are currently in this store.
  virtual void viewNames(const std::function<void(const std::vector<std::string>&)>& closure) = 0;

private:
  const tp_data::CollectionFactory* m_collectionFactory;
};

}

#endif
