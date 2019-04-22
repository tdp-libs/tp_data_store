#include "tp_data_store/stores/RAMStore.h"

#include "tp_data/Collection.h"
#include "tp_data/CollectionFactory.h"

#include "tp_utils/MutexUtils.h"
#include "tp_utils/DebugUtils.h"

#include <atomic>
#include <unordered_map>

namespace tp_data_store
{

namespace
{
struct CollectionDetails_lt
{
  TPMutex mutex{TPM};
  tp_data::Collection collection;
  int count{0};
  std::atomic_bool remove{false};
};
}

//##################################################################################################
struct RAMStore::Private
{
  TPMutex mutex{TPM};
  std::vector<std::string> collectionNames;
  std::unordered_map<std::string, CollectionDetails_lt*> collections;

  //################################################################################################
  ~Private()
  {
    for(const auto& c : collections)
      delete c.second;
  }

  //################################################################################################
  CollectionDetails_lt* collectionDetails(const std::string& name)
  {
    TP_MUTEX_LOCKER(mutex);
    auto& collectionDetails = collections[name];

    if(!collectionDetails)
    {
      collectionDetails = new CollectionDetails_lt();
      collectionNames.push_back(name);
    }

    collectionDetails->count++;

    return collectionDetails;
  }

  //################################################################################################
  void returnCollectionDetails(CollectionDetails_lt* collectionDetails)
  {
    bool deleteCollectionDetails = false;
    {
      TP_MUTEX_LOCKER(mutex);
      collectionDetails->count--;

      if(collectionDetails->remove)
      {
        for(auto i = collections.begin(); i!=collections.end(); ++i)
        {
          if(i->second == collectionDetails)
          {
            tpRemoveOne(collectionNames, i->first);
            collections.erase(i);
            break;
          }
        }

        deleteCollectionDetails = (collectionDetails->count == 0);
      }
    }

    if(deleteCollectionDetails)
      delete collectionDetails;
  }
};

//##################################################################################################
RAMStore::RAMStore(const tp_data::CollectionFactory* collectionFactory):
  AbstractStore(collectionFactory),
  d(new Private())
{

}

//##################################################################################################
RAMStore::~RAMStore()
{
  delete d;
}

//##################################################################################################
void RAMStore::add(const std::string& name,
                   const tp_data::Collection& collection)
{
  auto collectionDetails = d->collectionDetails(name);
  TP_CLEANUP([&]{d->returnCollectionDetails(collectionDetails);});
  TP_MUTEX_LOCKER(collectionDetails->mutex);
  std::string error;
  collectionFactory()->cloneAppend(error, collection, collectionDetails->collection);
  if(!error.empty())
    tpWarning() << "RAMStore::add: " << error;
}

//##################################################################################################
void RAMStore::remove(const std::string& name)
{
  auto collectionDetails = d->collectionDetails(name);
  TP_CLEANUP([&]{d->returnCollectionDetails(collectionDetails);});
  collectionDetails->remove = true;
}

//##################################################################################################
void RAMStore::fetch(const std::string& name,
                     tp_data::Collection& collection,
                     const std::vector<std::string>& subset)
{
  auto collectionDetails = d->collectionDetails(name);
  TP_CLEANUP([&]{d->returnCollectionDetails(collectionDetails);});
  TP_MUTEX_LOCKER(collectionDetails->mutex);
  std::string error;
  collectionFactory()->cloneAppend(error, collectionDetails->collection, collection, subset);
  if(!error.empty())
    tpWarning() << "RAMStore::fetch: " << error;
}

//##################################################################################################
void RAMStore::viewNames(const std::function<void(const std::vector<std::string>&)>& closure)
{
  TP_MUTEX_LOCKER(d->mutex);
  closure(d->collectionNames);
}

}
