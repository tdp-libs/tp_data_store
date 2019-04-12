TARGET = tp_data_store
TEMPLATE = lib

DEFINES += TDP_DATA_STORE_LIBRARY

SOURCES += src/Globals.cpp
HEADERS += inc/tp_data_store/Globals.h

SOURCES += src/AbstractStore.cpp
HEADERS += inc/tp_data_store/AbstractStore.h

SOURCES += src/MultiNameStore.cpp
HEADERS += inc/tp_data_store/MultiNameStore.h

#-- Stores -----------------------------------------------------------------------------------------
SOURCES += src/stores/RAMStore.cpp
HEADERS += inc/tp_data_store/stores/RAMStore.h

SOURCES += src/stores/FileSystemStore.cpp
HEADERS += inc/tp_data_store/stores/FileSystemStore.h

