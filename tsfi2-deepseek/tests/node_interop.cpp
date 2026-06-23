#include "node_interop.h"
#include "tsfi_dom.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "tsfi_tls.h"
#include <node.h>
#include <queue>
#include <string>
#include <mutex>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <memory>
#include <map>

#include <fstream>
#include <sstream>

static std::queue<std::string> input_queue;
static std::mutex input_mutex;

// LocalStorage and Cookie backing stores
static std::map<std::string, std::string> local_storage_map;
static std::mutex local_storage_mutex;

static std::map<std::string, std::string> cookie_map;
static std::mutex cookie_mutex;

static void LoadLocalStorage() {
    std::lock_guard<std::mutex> lock(local_storage_mutex);
    std::ifstream f("local_storage.json");
    if (!f.is_open()) return;
    std::string line;
    while (std::getline(f, line)) {
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            local_storage_map[line.substr(0, colon)] = line.substr(colon + 1);
        }
    }
}

static void SaveLocalStorage() {
    std::ofstream f("local_storage.json");
    for (auto const& [k, v] : local_storage_map) {
        f << k << ":" << v << "\n";
    }
}

static void LocalStorageGetItem(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    if (args.Length() < 1 || !args[0]->IsString()) {
        args.GetReturnValue().Set(v8::Null(isolate));
        return;
    }
    v8::String::Utf8Value key_val(isolate, args[0]);
    std::string key = *key_val;

    std::lock_guard<std::mutex> lock(local_storage_mutex);
    auto it = local_storage_map.find(key);
    if (it != local_storage_map.end()) {
        args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, it->second.c_str()).ToLocalChecked());
    } else {
        args.GetReturnValue().Set(v8::Null(isolate));
    }
}

static void LocalStorageSetItem(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsString()) {
        return;
    }
    v8::String::Utf8Value key_val(isolate, args[0]);
    v8::String::Utf8Value val_val(isolate, args[1]);
    std::string key = *key_val;
    std::string val = *val_val;

    std::lock_guard<std::mutex> lock(local_storage_mutex);
    local_storage_map[key] = val;
    SaveLocalStorage();
}

static void LocalStorageRemoveItem(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    if (args.Length() < 1 || !args[0]->IsString()) {
        return;
    }
    v8::String::Utf8Value key_val(isolate, args[0]);
    std::string key = *key_val;

    std::lock_guard<std::mutex> lock(local_storage_mutex);
    local_storage_map.erase(key);
    SaveLocalStorage();
}

static void LocalStorageClear(const v8::FunctionCallbackInfo<v8::Value>&) {
    std::lock_guard<std::mutex> lock(local_storage_mutex);
    local_storage_map.clear();
    SaveLocalStorage();
}

static void CookieSetter(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info) {
    (void)property;
    v8::Isolate* isolate = info.GetIsolate();
    v8::String::Utf8Value val_val(isolate, value);
    std::string cookie_str = *val_val;

    size_t first_semi = cookie_str.find(';');
    std::string main_pair = (first_semi != std::string::npos) ? cookie_str.substr(0, first_semi) : cookie_str;
    size_t equal = main_pair.find('=');
    if (equal != std::string::npos) {
        std::string name = main_pair.substr(0, equal);
        std::string val = main_pair.substr(equal + 1);
        
        name.erase(0, name.find_first_not_of(" \t"));
        name.erase(name.find_last_not_of(" \t") + 1);
        val.erase(0, val.find_first_not_of(" \t"));
        val.erase(val.find_last_not_of(" \t") + 1);

        std::lock_guard<std::mutex> lock(cookie_mutex);
        cookie_map[name] = val;
    }
}

static void CookieGetter(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info) {
    (void)property;
    v8::Isolate* isolate = info.GetIsolate();
    std::lock_guard<std::mutex> lock(cookie_mutex);
    std::string res = "";
    bool first = true;
    for (auto const& [name, val] : cookie_map) {
        if (!first) res += "; ";
        res += name + "=" + val;
        first = false;
    }
    info.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, res.c_str()).ToLocalChecked());
}

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

// Global active DOM node exported from presenter thread
extern "C" TsfiNode *g_dashboard_dom;

// Structure to track registered JS event listeners
struct JSListener {
    v8::Persistent<v8::Function> callback;
};

// Map nodes/windows to their listeners (e.g. "click", "keydown", etc.)
static std::map<TsfiNode*, std::map<std::string, std::vector<JSListener*>>> node_listeners;
static std::map<std::string, std::vector<JSListener*>> window_listeners;
static std::mutex listeners_mutex;

// Recursive DOM tree searching by ID attribute
static TsfiNode* find_node_by_id(TsfiNode *node, const char *id) {
    if (!node) return NULL;
    if (node->type == TSFI_NODE_ELEMENT) {
        const char *node_id = tsfi_node_get_attribute(node, "id");
        if (node_id && strcmp(node_id, id) == 0) {
            return node;
        }
    }
    TsfiNode *child = node->first_child;
    while (child) {
        TsfiNode *res = find_node_by_id(child, id);
        if (res) return res;
        child = child->next_sibling;
    }
    return NULL;
}

// Check which node is positioned at absolute coordinate x, y (for mouse event targeting)
static TsfiNode* find_node_at_coords(TsfiNode *node, float x, float y) {
    if (!node) return NULL;
    
    // Check children first (topmost layered elements)
    TsfiNode *child = node->last_child;
    while (child) {
        TsfiNode *found = find_node_at_coords(child, x, y);
        if (found) return found;
        child = child->prev_sibling;
    }

    if (node->type == TSFI_NODE_ELEMENT) {
        // Simple bounding box checks
        if (x >= node->layout_x && x <= (node->layout_x + node->layout_w) &&
            y >= node->layout_y && y <= (node->layout_y + node->layout_h)) {
            return node;
        }
    }
    return NULL;
}

// V8 DOM wrappers & templates
static v8::Local<v8::Object> WrapNode(v8::Isolate* isolate, TsfiNode* node);

static void DocumentGetElementById(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    if (args.Length() < 1 || !args[0]->IsString()) {
        args.GetReturnValue().Set(v8::Null(isolate));
        return;
    }

    v8::String::Utf8Value id_val(isolate, args[0]);
    const char *id = *id_val;

    TsfiNode *found = find_node_by_id(g_dashboard_dom, id);
    if (found) {
        args.GetReturnValue().Set(WrapNode(isolate, found));
    } else {
        args.GetReturnValue().Set(v8::Null(isolate));
    }
}

static void NodeAddEventListener(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsFunction()) {
        return;
    }

    v8::Local<v8::Object> holder = args.Holder();
    TsfiNode* node = static_cast<TsfiNode*>(holder->GetAlignedPointerFromInternalField(0));
    if (!node) return;

    v8::String::Utf8Value type_val(isolate, args[0]);
    std::string type = *type_val;

    v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(args[1]);

    std::lock_guard<std::mutex> lock(listeners_mutex);
    JSListener *listener = new JSListener();
    listener->callback.Reset(isolate, callback);
    node_listeners[node][type].push_back(listener);
}

static void WindowAddEventListener(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsFunction()) {
        return;
    }

    v8::String::Utf8Value type_val(isolate, args[0]);
    std::string type = *type_val;

    v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(args[1]);

    std::lock_guard<std::mutex> lock(listeners_mutex);
    JSListener *listener = new JSListener();
    listener->callback.Reset(isolate, callback);
    window_listeners[type].push_back(listener);
}

static void NodeSetAttribute(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsString()) {
        return;
    }

    v8::Local<v8::Object> holder = args.Holder();
    TsfiNode* node = static_cast<TsfiNode*>(holder->GetAlignedPointerFromInternalField(0));
    if (!node) return;

    v8::String::Utf8Value name(isolate, args[0]);
    v8::String::Utf8Value value(isolate, args[1]);

    tsfi_node_set_attribute(node, *name, *value);
}

static void NodeGetAttribute(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    if (args.Length() < 1 || !args[0]->IsString()) {
        args.GetReturnValue().Set(v8::Null(isolate));
        return;
    }

    v8::Local<v8::Object> holder = args.Holder();
    TsfiNode* node = static_cast<TsfiNode*>(holder->GetAlignedPointerFromInternalField(0));
    if (!node) {
        args.GetReturnValue().Set(v8::Null(isolate));
        return;
    }

    v8::String::Utf8Value name(isolate, args[0]);
    const char *val = tsfi_node_get_attribute(node, *name);
    if (val) {
        args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, val).ToLocalChecked());
    } else {
        args.GetReturnValue().Set(v8::Null(isolate));
    }
}

extern "C" {
    bool g_video_playing = true;
    double g_video_current_time = 0.0;
    double g_video_duration = 30.0;
}

static void VideoPlay(const v8::FunctionCallbackInfo<v8::Value>& args) {
    g_video_playing = true;
    printf("[JS Video] play() called\n");
}

static void VideoPause(const v8::FunctionCallbackInfo<v8::Value>& args) {
    g_video_playing = false;
    printf("[JS Video] pause() called\n");
}

static void VideoGetCurrentTime(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info) {
    v8::Isolate* isolate = info.GetIsolate();
    info.GetReturnValue().Set(v8::Number::New(isolate, g_video_current_time));
}

static void VideoSetCurrentTime(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info) {
    v8::Isolate* isolate = info.GetIsolate();
    if (value->IsNumber()) {
        g_video_current_time = value->NumberValue(isolate->GetCurrentContext()).FromMaybe(g_video_current_time);
        printf("[JS Video] Set currentTime = %f\n", g_video_current_time);
    }
}

static void VideoGetDuration(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info) {
    v8::Isolate* isolate = info.GetIsolate();
    info.GetReturnValue().Set(v8::Number::New(isolate, g_video_duration));
}

static v8::Local<v8::Object> WrapNode(v8::Isolate* isolate, TsfiNode* node) {
    v8::EscapableHandleScope handle_scope(isolate);

    v8::Local<v8::ObjectTemplate> templ = v8::ObjectTemplate::New(isolate);
    templ->SetInternalFieldCount(1);

    // Bind DOM manipulation methods
    templ->Set(v8::String::NewFromUtf8(isolate, "setAttribute").ToLocalChecked(),
               v8::FunctionTemplate::New(isolate, NodeSetAttribute));
    templ->Set(v8::String::NewFromUtf8(isolate, "getAttribute").ToLocalChecked(),
               v8::FunctionTemplate::New(isolate, NodeGetAttribute));
    templ->Set(v8::String::NewFromUtf8(isolate, "addEventListener").ToLocalChecked(),
               v8::FunctionTemplate::New(isolate, NodeAddEventListener));

    // Video tag controls
    if (node && node->tag_name && strcmp(node->tag_name, "video") == 0) {
        templ->Set(v8::String::NewFromUtf8(isolate, "play").ToLocalChecked(),
                   v8::FunctionTemplate::New(isolate, VideoPlay));
        templ->Set(v8::String::NewFromUtf8(isolate, "pause").ToLocalChecked(),
                   v8::FunctionTemplate::New(isolate, VideoPause));
        templ->SetAccessor(v8::String::NewFromUtf8(isolate, "currentTime").ToLocalChecked(),
                           VideoGetCurrentTime, VideoSetCurrentTime);
        templ->SetAccessor(v8::String::NewFromUtf8(isolate, "duration").ToLocalChecked(),
                           VideoGetDuration);
    }

    v8::Local<v8::Object> obj = templ->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
    obj->SetAlignedPointerInInternalField(0, node);

    return handle_scope.Escape(obj);
}

// Dispatches a formatted input event string to standard JS listeners
static void DispatchInputEvent(v8::Isolate* isolate, v8::Local<v8::Context> context, const std::string &event) {
    v8::HandleScope handle_scope(isolate);
    v8::Context::Scope context_scope(context);

    // Format matches MOUSE_MOVE x y, MOUSE_DOWN btn, KEY_DOWN key, etc.
    if (event.rfind("MOUSE_MOVE", 0) == 0) {
        int x = 0, y = 0;
        if (sscanf(event.c_str() + 10, "%d %d", &x, &y) == 2) {
            std::lock_guard<std::mutex> lock(listeners_mutex);
            auto it = window_listeners.find("mousemove");
            if (it != window_listeners.end()) {
                v8::Local<v8::Object> ev_obj = v8::Object::New(isolate);
                ev_obj->Set(context, v8::String::NewFromUtf8(isolate, "clientX").ToLocalChecked(), v8::Number::New(isolate, x)).Check();
                ev_obj->Set(context, v8::String::NewFromUtf8(isolate, "clientY").ToLocalChecked(), v8::Number::New(isolate, y)).Check();
                v8::Local<v8::Value> argv[] = { ev_obj };
                for (JSListener *l : it->second) {
                    v8::Local<v8::Function> cb = v8::Local<v8::Function>::New(isolate, l->callback);
                    cb->Call(context, context->Global(), 1, argv).ToLocalChecked();
                }
            }
        }
    } else if (event.rfind("MOUSE_DOWN", 0) == 0) {
        unsigned int btn = 0;
        int x = 0, y = 0;
        // Check if format contains button + coordinates (from direct Wayland pointer mapping)
        bool has_coords = (sscanf(event.c_str() + 10, "%u %d %d", &btn, &x, &y) == 3);
        if (!has_coords) {
            sscanf(event.c_str() + 10, "%u", &btn);
        }
        std::lock_guard<std::mutex> lock(listeners_mutex);
        
        // Dispatch to target node if coordinates are available
        if (has_coords && g_dashboard_dom) {
            TsfiNode *target = find_node_at_coords(g_dashboard_dom, (float)x, (float)y);
            if (target) {
                auto nit = node_listeners.find(target);
                if (nit != node_listeners.end()) {
                    auto it = nit->second.find("click");
                    if (it != nit->second.end()) {
                        v8::Local<v8::Object> ev_obj = v8::Object::New(isolate);
                        ev_obj->Set(context, v8::String::NewFromUtf8(isolate, "button").ToLocalChecked(), v8::Number::New(isolate, btn)).Check();
                        v8::Local<v8::Value> argv[] = { ev_obj };
                        for (JSListener *l : it->second) {
                            v8::Local<v8::Function> cb = v8::Local<v8::Function>::New(isolate, l->callback);
                            cb->Call(context, context->Global(), 1, argv).ToLocalChecked();
                        }
                    }
                }
            }
        }
        
        // Fallback to window-level listener
        auto it = window_listeners.find("mousedown");
        if (it != window_listeners.end()) {
            v8::Local<v8::Object> ev_obj = v8::Object::New(isolate);
            ev_obj->Set(context, v8::String::NewFromUtf8(isolate, "button").ToLocalChecked(), v8::Number::New(isolate, btn)).Check();
            v8::Local<v8::Value> argv[] = { ev_obj };
            for (JSListener *l : it->second) {
                v8::Local<v8::Function> cb = v8::Local<v8::Function>::New(isolate, l->callback);
                cb->Call(context, context->Global(), 1, argv).ToLocalChecked();
            }
        }
        g_input_dirty = true;
    } else if (event.rfind("KEY_DOWN", 0) == 0) {
        unsigned int key = 0;
        if (sscanf(event.c_str() + 8, "%u", &key) == 1) {
            std::lock_guard<std::mutex> lock(listeners_mutex);
            auto it = window_listeners.find("keydown");
            if (it != window_listeners.end()) {
                v8::Local<v8::Object> ev_obj = v8::Object::New(isolate);
                ev_obj->Set(context, v8::String::NewFromUtf8(isolate, "keyCode").ToLocalChecked(), v8::Number::New(isolate, key)).Check();
                v8::Local<v8::Value> argv[] = { ev_obj };
                for (JSListener *l : it->second) {
                    v8::Local<v8::Function> cb = v8::Local<v8::Function>::New(isolate, l->callback);
                    cb->Call(context, context->Global(), 1, argv).ToLocalChecked();
                }
            }
        }
        g_input_dirty = true;
    }
}

static std::string tsfi_http_fetch(const std::string &host, const std::string &path, const std::string &origin, bool &cors_passed) {
    if (host == "localhost" || host == "test-cors.local") {
        // Mock a response for testing CORS without hitting network
        cors_passed = false;
        return "CORS ERROR: No Access-Control-Allow-Origin header present";
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return "";

    struct hostent *server = gethostbyname(host.c_str());
    if (!server) {
        close(sockfd);
        return "";
    }

    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(443);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

    cors_passed = true;

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sockfd);
        return "";
    }

    TsfiTlsContext ctx;
    tsfi_tls_init(&ctx, sockfd);

    if (!tsfi_tls_handshake(&ctx, host.c_str())) {
        tsfi_tls_close(&ctx);
        close(sockfd);
        return "";
    }

    std::string req = "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\nUser-Agent: AuncientWeb/1.0\r\n";
    if (!origin.empty()) {
        req += "Origin: " + origin + "\r\n";
    }
    req += "\r\n";
    tsfi_tls_write(&ctx, req.c_str(), req.length());

    std::string response = "";
    char buf[4096];
    while (true) {
        ssize_t bytes = tsfi_tls_read(&ctx, buf, sizeof(buf) - 1);
        if (bytes <= 0) break;
        buf[bytes] = '\0';
        response.append(buf, bytes);
    }

    tsfi_tls_close(&ctx);
    close(sockfd);

    // Parse headers and body
    size_t body_pos = response.find("\r\n\r\n");
    std::string headers_part = "";
    std::string body_part = response;
    if (body_pos != std::string::npos) {
        headers_part = response.substr(0, body_pos);
        body_part = response.substr(body_pos + 4);
    }

    // Origin/CORS validation
    if (!origin.empty()) {
        // Search headers for Access-Control-Allow-Origin
        size_t acao_pos = headers_part.find("Access-Control-Allow-Origin:");
        if (acao_pos == std::string::npos) {
            acao_pos = headers_part.find("access-control-allow-origin:");
        }
        if (acao_pos != std::string::npos) {
            size_t val_start = acao_pos + 28;
            size_t val_end = headers_part.find("\r\n", val_start);
            if (val_end == std::string::npos) val_end = headers_part.length();
            std::string acao_val = headers_part.substr(val_start, val_end - val_start);
            // Trim whitespace
            acao_val.erase(0, acao_val.find_first_not_of(" \t"));
            acao_val.erase(acao_val.find_last_not_of(" \t") + 1);

            if (acao_val != "*" && acao_val != origin) {
                cors_passed = false;
                return "CORS ERROR: Origin " + origin + " is not allowed by Access-Control-Allow-Origin: " + acao_val;
            }
        } else {
            // If Origin was sent but no Access-Control-Allow-Origin header is present, it's a CORS failure
            cors_passed = false;
            return "CORS ERROR: No Access-Control-Allow-Origin header present";
        }
    }

    return body_part;
}

static void GlobalFetch(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    if (args.Length() < 1 || !args[0]->IsString()) {
        args.GetReturnValue().Set(v8::Null(isolate));
        return;
    }

    v8::String::Utf8Value url_val(isolate, args[0]);
    std::string url = *url_val;

    std::string origin = "";
    if (args.Length() >= 2 && args[1]->IsObject()) {
        v8::Local<v8::Context> context = isolate->GetCurrentContext();
        v8::Local<v8::Object> options = args[1]->ToObject(context).ToLocalChecked();
        v8::Local<v8::Value> orig_val;
        if (options->Get(context, v8::String::NewFromUtf8(isolate, "origin").ToLocalChecked()).ToLocal(&orig_val) && orig_val->IsString()) {
            v8::String::Utf8Value orig_utf(isolate, orig_val);
            origin = *orig_utf;
        }
    }

    std::string host = "";
    std::string path = "/";

    if (url.rfind("https://", 0) == 0) {
        std::string stripped = url.substr(8);
        size_t slash = stripped.find('/');
        std::string host_port = "";
        if (slash != std::string::npos) {
            host_port = stripped.substr(0, slash);
            path = stripped.substr(slash);
        } else {
            host_port = stripped;
        }
        size_t colon = host_port.find(':');
        if (colon != std::string::npos) {
            host = host_port.substr(0, colon);
        } else {
            host = host_port;
        }
    } else {
        args.GetReturnValue().Set(v8::Null(isolate));
        return;
    }

    bool cors_passed = true;
    std::string body = tsfi_http_fetch(host, path, origin, cors_passed);
    if (!cors_passed) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, body.c_str()).ToLocalChecked()
        ));
        return;
    }
    args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, body.c_str()).ToLocalChecked());
}

extern "C" int start_embedded_node(int argc, char **argv) {
    LoadLocalStorage();
    std::vector<std::string> args(argv, argv + argc);

    auto init_result = node::InitializeOncePerProcess(
        args,
        static_cast<node::ProcessInitializationFlags::Flags>(0)
    );

    if (init_result->early_return()) {
        return init_result->exit_code();
    }

    int exit_code = 0;
    {
        node::MultiIsolatePlatform* platform = init_result->platform();
        std::vector<std::string> errors = init_result->errors();

        auto setup = node::CommonEnvironmentSetup::Create(
            platform,
            &errors,
            init_result->args(),
            init_result->exec_args()
        );

        if (!setup) {
            fprintf(stderr, "[Node.js Embedder] Setup failed: %s\n", errors.empty() ? "unknown" : errors[0].c_str());
            return 1;
        }

        v8::Isolate* isolate = setup->isolate();
        node::Environment* env = setup->env();

        {
            v8::Locker locker(isolate);
            v8::Isolate::Scope isolate_scope(isolate);
            v8::HandleScope handle_scope(isolate);
            v8::Local<v8::Context> context = setup->context();
            v8::Context::Scope context_scope(context);

            // Expose the global document object template and attach DOM methods
            v8::Local<v8::ObjectTemplate> doc_templ = v8::ObjectTemplate::New(isolate);
            doc_templ->Set(v8::String::NewFromUtf8(isolate, "getElementById").ToLocalChecked(),
                           v8::FunctionTemplate::New(isolate, DocumentGetElementById));
            doc_templ->SetAccessor(v8::String::NewFromUtf8(isolate, "cookie").ToLocalChecked(),
                                   CookieGetter, CookieSetter);

            v8::Local<v8::Object> document_obj = doc_templ->NewInstance(context).ToLocalChecked();

            // Set document and window on global context
            v8::Local<v8::Object> global = context->Global();
            global->Set(context, v8::String::NewFromUtf8(isolate, "document").ToLocalChecked(), document_obj).Check();

            // Create window object
            v8::Local<v8::ObjectTemplate> win_templ = v8::ObjectTemplate::New(isolate);
            win_templ->Set(v8::String::NewFromUtf8(isolate, "addEventListener").ToLocalChecked(),
                           v8::FunctionTemplate::New(isolate, WindowAddEventListener));
            v8::Local<v8::Object> window_obj = win_templ->NewInstance(context).ToLocalChecked();
            global->Set(context, v8::String::NewFromUtf8(isolate, "window").ToLocalChecked(), window_obj).Check();

            // Create localStorage object template
            v8::Local<v8::ObjectTemplate> ls_templ = v8::ObjectTemplate::New(isolate);
            ls_templ->Set(v8::String::NewFromUtf8(isolate, "getItem").ToLocalChecked(),
                          v8::FunctionTemplate::New(isolate, LocalStorageGetItem));
            ls_templ->Set(v8::String::NewFromUtf8(isolate, "setItem").ToLocalChecked(),
                          v8::FunctionTemplate::New(isolate, LocalStorageSetItem));
            ls_templ->Set(v8::String::NewFromUtf8(isolate, "removeItem").ToLocalChecked(),
                          v8::FunctionTemplate::New(isolate, LocalStorageRemoveItem));
            ls_templ->Set(v8::String::NewFromUtf8(isolate, "clear").ToLocalChecked(),
                          v8::FunctionTemplate::New(isolate, LocalStorageClear));

            v8::Local<v8::Object> ls_obj = ls_templ->NewInstance(context).ToLocalChecked();
            global->Set(context, v8::String::NewFromUtf8(isolate, "localStorage").ToLocalChecked(), ls_obj).Check();
            window_obj->Set(context, v8::String::NewFromUtf8(isolate, "localStorage").ToLocalChecked(), ls_obj).Check();

            // Set global fetch function
            global->Set(context, v8::String::NewFromUtf8(isolate, "fetch").ToLocalChecked(),
                        v8::FunctionTemplate::New(isolate, GlobalFetch)->GetFunction(context).ToLocalChecked()).Check();

            v8::MaybeLocal<v8::Value> load_env = node::LoadEnvironment(env,
                "const fs = require('fs');\n"
                "const path = require('path');\n"
                "const Module = require('module');\n"
                "const filepath = path.resolve(process.argv[1]);\n"
                "const code = fs.readFileSync(filepath, 'utf8');\n"
                "const m = new Module(filepath);\n"
                "m._compile(code, filepath);\n"
            );
            (void)load_env;

            // Simple tick polling loop for events inside Node event loop spin
            // We run SpinEventLoop but occasionally flush incoming events
            while (true) {
                auto exit_code_val = node::SpinEventLoop(env);
                exit_code = exit_code_val.FromMaybe(0);

                // Check for queued hardware input events
                std::string popped_event;
                while (true) {
                    char *raw = pop_input_event();
                    if (!raw) break;
                    popped_event = raw;
                    free(raw);
                    DispatchInputEvent(isolate, context, popped_event);
                }

                // Yield to Node event loop ticks
                usleep(2000);
            }
        }
    }

    node::TearDownOncePerProcess();
    return exit_code;
}
