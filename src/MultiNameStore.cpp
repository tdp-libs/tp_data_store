#include "tp_data_store/MultiNameStore.h"
#include "tp_data_store/AbstractStore.h"

#include "tp_data/Collection.h"
#include "tp_data/CollectionFactory.h"

#include "tp_utils/MutexUtils.h"

namespace tp_data_store
{
namespace
{

//##################################################################################################
std::string escapeName(const std::string& name)
{
  std::string n;
  for(const auto& c : name)
  {
    if(c=='_')
      n+="_0";
    else if(c=='.')
      n+="_1";
    else
      n+=c;
  }
  return n;
}

//##################################################################################################
std::string unEscapeName(const std::string& name)
{
  std::string n;

  auto c = name.data();
  auto cMax = c+name.size();
  bool decode=false;
  for(; c<cMax; c++)
  {
    if(decode)
    {
      if((*c)=='0')
        n+='_';
      else if((*c)=='1')
        n+='.';
      decode=false;
    }
    else
    {
      if((*c)=='_')
        decode = true;
      else
        n+=(*c);
    }
  }

  return n;
}

}

//##################################################################################################
struct MultiNameStore::Private
{
  AbstractStore* store;

  TPMutex mutex{TPM};
  std::unordered_map<std::string, std::shared_ptr<TPMutex>> mutexes;
  std::vector<MultiName> multiNames;

  //################################################################################################
  Private(AbstractStore* store_):
    store(store_)
  {
    store->viewNames([&](const std::vector<std::string>& names)
    {
      multiNames.resize(names.size());
      for(size_t c=0; c<names.size(); c++)
      {
        const auto& name = names.at(c);
        auto& multiName = multiNames.at(c);

        std::vector<std::string> parts;
        tpSplit(parts, name, '.');
        for(auto& p : parts)
          p = unEscapeName(p);
        multiName = compileNames(parts);
      }
    });
  }

  //################################################################################################
  TPMutex& getMutex(const MultiName& multiName, NameAction nameAction)
  {
    TP_MUTEX_LOCKER(mutex);

    if(nameAction!=NameAction::None)
    {
      bool add = (nameAction==NameAction::Add);
      for(size_t n=0; n<multiNames.size(); n++)
      {
        if(multiNames.at(n).name == multiName.name)
        {
          add=false;
          if(nameAction==NameAction::Remove)
            tpRemoveAt(multiNames, n);
          break;
        }
      }

      if(add)
        multiNames.push_back(multiName);
    }

    auto& m = mutexes[multiName.name];
    if(!m)
      m.reset(new TPMutex(TPM));
    return *m;
  }

  //################################################################################################
  static MultiName compileNames(const std::vector<std::string>& names)
  {
    MultiName multiName;
    multiName.names.reserve(names.size());

    for(const auto& n : names)
    {
      auto esc = escapeName(n);
      multiName.names.push_back(n);
      multiName.name += (multiName.name.empty()?"":".") + esc;
    }

    return multiName;
  }
};

//##################################################################################################
MultiNameStore::MultiNameStore(AbstractStore* store):
  d(new Private(store))
{

}

//##################################################################################################
MultiNameStore::~MultiNameStore()
{
  delete d;
}

//##################################################################################################
const tp_data::CollectionFactory* MultiNameStore::collectionFactory() const
{
  return d->store->collectionFactory();
}

//##################################################################################################
void MultiNameStore::add(const std::vector<std::string>& names,
                         const tp_data::Collection& collection)
{
  auto multiName = d->compileNames(names);
  TP_MUTEX_LOCKER(d->getMutex(multiName, NameAction::Add));
  d->store->add(multiName.name, collection);
}

//##################################################################################################
void MultiNameStore::remove(const std::vector<std::string>& names)
{
  auto multiName = d->compileNames(names);
  d->store->remove(multiName.name);
}

//##################################################################################################
void MultiNameStore::fetch(const std::vector<std::string>& names,
                           tp_data::Collection& collection,
                           const std::vector<std::string>& subset)
{
  auto multiName = d->compileNames(names);
  d->store->fetch(multiName.name, collection, subset);
}

//##################################################################################################
void MultiNameStore::fetch(const std::vector<std::string>& andNames,
                           std::vector<std::shared_ptr<CollectionFetchResults>>& collections,
                           const std::vector<std::string>& subset)
{
  std::vector<MultiName> collectionNames = fetchNames(andNames);

  collections.resize(collectionNames.size());
  for(size_t i=0; i<collectionNames.size(); i++)
  {
    auto& collection = collections.at(i);
    const auto& multiName = collectionNames.at(i);
    collection.reset(new CollectionFetchResults());
    collection->multiName = multiName;
    d->store->fetch(multiName.name, collection->collection, subset);
  }
}

//##################################################################################################
void MultiNameStore::viewNames(const std::function<void(const std::vector<MultiName>&)>& closure)
{
  TP_MUTEX_LOCKER(d->mutex);
  closure(d->multiNames);
}

//##################################################################################################
std::vector<MultiName> MultiNameStore::fetchNames(const std::vector<std::string>& andNames)
{
  std::vector<MultiName> collectionNames;
  TP_MUTEX_LOCKER(d->mutex);
  for(const auto& multiName : d->multiNames)
  {
    bool add=true;
    for(const auto& n : andNames)
    {
      if(!tpContains(multiName.names, n))
      {
        add = false;
        break;
      }
    }

    if(add)
      collectionNames.push_back(multiName);
  }
  return collectionNames;
}


}
