#include "tp_data_store/stores/FileSystemStore.h"

#include "tp_data/CollectionFactory.h"

#include "tp_utils/MutexUtils.h"
#include "tp_utils/FileUtils.h"
#include "tp_utils/DebugUtils.h"

#include <unordered_map>

namespace tp_data_store
{

//##################################################################################################
struct FileSystemStore::Private
{
  TPMutex mutex{TPM};
  std::unordered_map<std::string, std::shared_ptr<TPMutex>> mutexes;
  std::string path;
  std::vector<std::string> names;

  //################################################################################################
  Private(const std::string& path_):
    path(path_),
    names(tp_utils::listDirectories(path))
  {
    for(auto& name : names)
    {
      std::vector<std::string> parts;
      tpSplit(parts, name, '/', tp_utils::SplitBehavior::SkipEmptyParts);
      if(!parts.empty())
        name = parts.back();
    }
  }

  //################################################################################################
  std::string getPath(const std::string& name)
  {
    return path + "/" + name;
  }

  //################################################################################################
  TPMutex& getMutex(const std::string& name, NameAction nameAction)
  {
    TP_MUTEX_LOCKER(mutex);

    if(nameAction!=NameAction::None)
    {
      bool add = (nameAction==NameAction::Add);
      for(size_t n=0; n<names.size(); n++)
      {
        if(names.at(n) == name)
        {
          add=false;
          if(nameAction==NameAction::Remove)
            tpRemoveAt(names, n);
          break;
        }
      }

      if(add)
        names.push_back(name);
    }

    auto& m = mutexes[name];
    if(!m)
      m.reset(new TPMutex(TPM));
    return *m;
  }
};

//##################################################################################################
FileSystemStore::FileSystemStore(const tp_data::CollectionFactory* collectionFactory,
                                 const std::string& path):
  AbstractStore(collectionFactory),
  d(new Private(path))
{

}

//##################################################################################################
FileSystemStore::~FileSystemStore()
{
  delete d;
}

//##################################################################################################
void FileSystemStore::add(const std::string& name,
                          const tp_data::Collection& collection)
{
  TP_MUTEX_LOCKER(d->getMutex(name, NameAction::Add));
  std::string error;
  collectionFactory()->saveToPath(error, collection, d->getPath(name), true);
  if(!error.empty())
    tpWarning() << "FileSystemStore::add Error: " << error;
}

//##################################################################################################
void FileSystemStore::remove(const std::string& name)
{
  if(name.empty())
    return;

  TP_MUTEX_LOCKER(d->getMutex(name, NameAction::Remove));
  tp_utils::rm(d->getPath(name), true);
}

//##################################################################################################
void FileSystemStore::fetch(const std::string& name,
                            tp_data::Collection& collection,
                            const std::vector<std::string>& subset)
{
  TP_MUTEX_LOCKER(d->getMutex(name, NameAction::None));
  std::string error;
  collectionFactory()->loadFromPath(error, d->getPath(name), collection, subset);
  if(!error.empty())
    tpWarning() << "FileSystemStore::fetch Error: " << error;
}

//##################################################################################################
void FileSystemStore::viewNames(const std::function<void(const std::vector<std::string>&)>& closure)
{
  TP_MUTEX_LOCKER(d->mutex);
  closure(d->names);
}

}
