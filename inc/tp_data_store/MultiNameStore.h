#ifndef tp_data_store_MultiNameStore_h
#define tp_data_store_MultiNameStore_h

#include "tp_data_store/Globals.h"

#include "tp_data/Collection.h"

#include <memory>

namespace tp_data
{
class CollectionFactory;
}

namespace tp_data_store
{
class AbstractStore;

//##################################################################################################
struct MultiName
{
  std::string name;
  std::vector<std::string> names;
};

//##################################################################################################
struct CollectionFetchResults
{
  MultiName multiName;
  tp_data::Collection collection;
};

//##################################################################################################
//! The base class for data stores.
class MultiNameStore
{
public:
  //################################################################################################
  MultiNameStore(AbstractStore* store);

  //################################################################################################
  ~MultiNameStore();

  //################################################################################################
  const tp_data::CollectionFactory* collectionFactory() const;

  //################################################################################################
  //! Add members to a new or existing collection.
  void add(const std::vector<std::string>& names,
           const tp_data::Collection& collection);

  //################################################################################################
  //! Remove a collection.
  void remove(const std::vector<std::string>& names);

  //################################################################################################
  //! Fetch a collection that matches all names in order.
  void fetch(const std::vector<std::string>& names,
             tp_data::Collection& collection,
             const std::vector<std::string>& subset=std::vector<std::string>());

  //################################################################################################
  //! Fetch all collections that match all of the names in andNames.
  void fetch(const std::vector<std::string>& andNames,
             std::vector<std::shared_ptr<CollectionFetchResults>>& collections,
             const std::vector<std::string>& subset=std::vector<std::string>());

  //################################################################################################
  //! View the list of collection names that are currently in this store.
  void viewNames(const std::function<void(const std::vector<MultiName>&)>& closure);

  //################################################################################################
  //! Fetch all names that match all of the names in andNames
  std::vector<MultiName> fetchNames(const std::vector<std::string>& andNames);

private:
  struct Private;
  friend struct Private;
  Private* d;
};

}

#endif
