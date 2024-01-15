#ifndef LOW_LEVEL_PROGRAMMING_LAB3_CONNECTION_H
#define LOW_LEVEL_PROGRAMMING_LAB3_CONNECTION_H

#include <stdbool.h>

char *client_request(const char *host, int port, const char *message);

struct server;

struct server *server_init(int port);

char *server_accept(struct server *server);

bool server_send(struct server *server, char *message);

void server_free(struct server *server);

#endif // LOW_LEVEL_PROGRAMMING_LAB3_CONNECTION_H
