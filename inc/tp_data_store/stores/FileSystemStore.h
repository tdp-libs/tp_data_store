#ifndef tp_data_store_FileSystemStore_h
#define tp_data_store_FileSystemStore_h

#include "tp_data_store/AbstractStore.h"

namespace tp_data_store
{

//##################################################################################################
//! Stores collections in RAM.
class FileSystemStore : public AbstractStore
{
public:
  //################################################################################################
  FileSystemStore(const tp_data::CollectionFactory* collectionFactory,
                  const std::string& path);

  //################################################################################################
  ~FileSystemStore() override;

  //################################################################################################
  void add(const std::string& name,
           const tp_data::Collection& collection) override;

  //################################################################################################
  void remove(const std::string& name) override;

  //################################################################################################
  void fetch(const std::string& name,
             tp_data::Collection& collection,
             const std::vector<std::string>& subset=std::vector<std::string>()) override;

  //################################################################################################
  void viewNames(const std::function<void(const std::vector<std::string>&)>& closure) override;

private:
  struct Private;
  friend struct Private;
  Private* d;
};

}

#endif
