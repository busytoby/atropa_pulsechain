#ifndef NODE_INTEROP_H
#define NODE_INTEROP_H

#ifdef __cplusplus
extern "C" {
#endif

int start_embedded_node(int argc, char **argv);
void push_input_event(const char *cmd);
char* pop_input_event(void);

#ifdef __cplusplus
}
#endif

#endif /* NODE_INTEROP_H */
