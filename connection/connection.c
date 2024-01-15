#include "connection.h"
#include "logger.h"
#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct message_header {
  int32_t content_length;
};

static bool receive_all(int socket, void *data, size_t data_size) {
  size_t data_read = 0;
  while (data_read < data_size) {
    const ssize_t receive_result =
        recv(socket, data + data_read, data_size - data_read, 0);
    if (receive_result < 0) {
      warn("Receive data error");
      return false;
    }

    data_read += receive_result;
  }

  return true;
}

static bool send_all(int socket, const void *data, size_t data_size) {
  size_t data_send = 0;
  while (data_send < data_size) {
    const ssize_t send_result =
        send(socket, data + data_send, data_size - data_send, 0);
    if (send_result < 0) {
      warn("Send data error");
      return false;
    }

    data_send += send_result;
  }

  return true;
}

char *client_request(const char *host, int port, const char *message) {
  const int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    warn("Create socket error");
    return NULL;
  }

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = port;
  address.sin_addr.s_addr = inet_addr(host);

  const int connect_result =
      connect(sock, (struct sockaddr *)&address, sizeof(address));
  if (connect_result != 0) {
    close(sock);
    warn("Connect socket error: %d", errno);
    return NULL;
  }

  const struct message_header s_header = {.content_length =
                                              (int32_t)strlen(message)};
  if (!send_all(sock, &s_header, sizeof(s_header))) {
    close(sock);
    warn("Send header socket error");
    return NULL;
  }

  if (!send_all(sock, message, strlen(message))) {
    close(sock);
    warn("Send socket error");
    return NULL;
  }

  struct message_header header;
  if (!receive_all(sock, &header, sizeof(header))) {
    close(sock);
    warn("Receive header error");
    return NULL;
  }

  char *receive_message = malloc(header.content_length);
  if (receive_message == NULL) {
    close(sock);
    warn("Receive message allocation error");
    return NULL;
  }

  debug("Received header");

  if (!receive_all(sock, receive_message, header.content_length)) {
    close(sock);
    warn("Receive message error");
    return NULL;
  }

  debug("Received: %s", receive_message);

  close(sock);
  return receive_message;
}

struct server {
  int socket;
  int client_socket;
};

struct server *server_init(int port) {
  struct server *server = malloc(sizeof(struct server));
  if (server == NULL) {
    warn("Server allocation error");
    return NULL;
  }

  server->client_socket = -1;
  server->socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server->socket < 0) {
    warn("Create socket error");
    return NULL;
  }

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = port;
  address.sin_addr.s_addr = htonl(INADDR_ANY);

  const int bind_result =
      bind(server->socket, (struct sockaddr *)&address, sizeof(address));
  if (bind_result != 0) {
    close(server->socket);
    warn("Bind socket error");
    return NULL;
  }

  const int backlog = 128;
  const int listen_result = listen(server->socket, backlog);
  if (listen_result < 0) {
    close(server->socket);
    warn("Listen socket error");
    return NULL;
  }

  return server;
}

char *server_accept(struct server *server) {
  struct sockaddr_in client_address;
  socklen_t client_address_len = sizeof(client_address);

  server->client_socket = accept(
      server->socket, (struct sockaddr *)&client_address, &client_address_len);
  if (server->client_socket < 0) {
    warn("Accept socket error");
    return NULL;
  }

  struct message_header header;
  if (!receive_all(server->client_socket, &header, sizeof(header))) {
    close(server->client_socket);
    warn("Receive header error");
    return NULL;
  }

  char *receive_message = malloc(header.content_length);
  if (receive_message == NULL) {
    close(server->client_socket);
    warn("Receive message allocation error");
    return NULL;
  }

  if (!receive_all(server->client_socket, receive_message,
                   header.content_length)) {
    close(server->client_socket);
    warn("Receive message error");
    return NULL;
  }

  return receive_message;
}

bool server_send(struct server *server, char *message) {
  const struct message_header s_header = {.content_length =
                                              (int32_t)strlen(message)};
  if (!send_all(server->client_socket, &s_header, sizeof(s_header))) {
    close(server->client_socket);
    warn("Send header socket error");
    return NULL;
  }

  if (!send_all(server->client_socket, message, strlen(message))) {
    close(server->client_socket);
    warn("Send socket error");
    return false;
  }

  close(server->client_socket);
  server->client_socket = -1;
  return true;
}

void server_free(struct server *server) {
  if (server->client_socket > 0) {
    close(server->client_socket);
  }
  close(server->socket);
  free(server);
}