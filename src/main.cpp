#include "cacheforge/CommandProcessor.h"
#include "cacheforge/KVStore.h"
#include "cacheforge/TcpServer.h"

int main() {
    cacheforge::KVStore store;
    cacheforge::CommandProcessor processor(store);
    cacheforge::TcpServer server(processor);

    return server.run();
}
