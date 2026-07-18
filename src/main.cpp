#include "cacheforge/CommandProcessor.h"
#include "cacheforge/KVStore.h"
#include "cacheforge/SnapshotPersistence.h"
#include "cacheforge/TcpServer.h"

#include <iostream>
#include <string>

int main() {
    cacheforge::KVStore store;
    const cacheforge::SnapshotPersistence snapshot;
    std::string snapshot_error;
    if (!snapshot.load(store, snapshot_error)) {
        std::cerr << "Snapshot load failed: " << snapshot_error << '\n';
    }

    cacheforge::CommandProcessor processor(store);
    cacheforge::TcpServer server(processor);

    return server.run();
}
