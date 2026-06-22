#include "node_interop.h"
#include <node.h>
#include <queue>
#include <string>
#include <mutex>
#include <cstdlib>
#include <cstring>

static std::queue<std::string> input_queue;
static std::mutex input_mutex;

extern "C" void push_input_event(const char *cmd) {
    std::lock_guard<std::mutex> lock(input_mutex);
    input_queue.push(cmd);
}

extern "C" char* pop_input_event(void) {
    std::lock_guard<std::mutex> lock(input_mutex);
    if (input_queue.empty()) {
        return NULL;
    }
    std::string s = input_queue.front();
    input_queue.pop();
    return strdup(s.c_str());
}

extern "C" int start_embedded_node(int argc, char **argv) {
    return node::Start(argc, argv);
}
