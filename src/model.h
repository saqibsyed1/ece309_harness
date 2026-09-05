#ifndef MODEL_H
#define MODEL_H

/* Simulates asking an LLM. Doesn't call any real API -- just decides
   whether to use the calculator tool or give a canned echo reply.
   Writes its answer into response_buffer (must be big enough). */
void mock_model_generate(const char *input, char *response_buffer, int buffer_size);

#endif
