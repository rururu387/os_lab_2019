#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <getopt.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "pthread.h"
#include "inttypes.h"
#include "common.h"

pthread_mutex_t bindMut = PTHREAD_MUTEX_INITIALIZER, fileMut = PTHREAD_MUTEX_INITIALIZER;

struct FactorialArgs {
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
};

uint64_t Factorial(const struct FactorialArgs *args) {
  uint64_t ans = 1;

  // TODO: your code here
  for (int i = args->begin; i < args->end; i++)
  {
    ans = MultModulo(ans, i, args->mod);
  }

  return ans;
}

void *ThreadFactorial(void *args) {
  struct FactorialArgs *fargs = (struct FactorialArgs *)args;
  return (void *)(uint64_t *)Factorial(fargs);
}

void parallelServerCreate()
{
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    fprintf(stderr, "Can not create server socket!");
    return;
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = 0;
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

  //Mutex may be unnecessery
  pthread_mutex_lock(&bindMut); 
  int err = bind(server_fd, (struct sockaddr *)&server, sizeof(server));
  pthread_mutex_unlock(&bindMut);

  //struct sockaddr_in server2;
  int len = sizeof(server);
  if (getsockname(server_fd, (struct sockaddr *)&server, &len))
  {
    printf("Could not bind socket");
    return;
  }

  if (err < 0) {
    fprintf(stderr, "Can not bind to socket!");
    return;
  }

  err = listen(server_fd, 128);
  if (err < 0) {
    fprintf(stderr, "Could not listen on socket\n");
    return;
  }

  printf("Server listening at %d\n", server.sin_port);

  pthread_mutex_lock(&fileMut);
  char* fileName = "./ports.txt";
  FILE* fp = fopen(fileName, "a");
  if (fp == NULL)
  {
    printf("Couldn't open file: %s\n", fileName);
    return;
  }
  fprintf(fp, "127.0.0.1 %hu\n", server.sin_port);
  fclose(fp);
  pthread_mutex_unlock(&fileMut);

  while (true) {
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    int client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);

    if (client_fd < 0) {
      fprintf(stderr, "Could not establish new connection\n");
      continue;
    }
    while (true) {
      unsigned int buffer_size = sizeof(uint64_t) * 3;
      char from_client[buffer_size];
      int read = recv(client_fd, from_client, buffer_size, 0);

      if (!read)
        break;
      if (read < 0) {
        fprintf(stderr, "Client read failed\n");
        break;
      }
      if (read < buffer_size) {
        fprintf(stderr, "Client send wrong data format\n");
        break;
      }

      int tnum = 1;
      pthread_t threads[tnum];

      uint64_t begin = 0;
      uint64_t end = 0;
      uint64_t mod = 0;
      memcpy(&begin, from_client, sizeof(uint64_t));
      memcpy(&end, from_client + sizeof(uint64_t), sizeof(uint64_t));
      memcpy(&mod, from_client + 2 * sizeof(uint64_t), sizeof(uint64_t));
      
      fprintf(stdout, "Receive: %llu %llu %llu\n", begin, end, mod);

      struct FactorialArgs args[tnum];
      for (uint32_t i = 0; i < tnum; i++) {
        uint64_t step = (end - begin) / tnum;
        args[i].begin = begin;
        if(i == tnum - 1)
        {
          args[i].end = end;
        }
        else
        {
          args[i].end = begin + step;
        }
        args[i].mod = mod;

        if (pthread_create(&(threads[i]), NULL, (void*) ThreadFactorial, (void *)&args[i]))
        {
          printf("Error: pthread_create failed!\n");
          return 1;
        }
      }

      uint64_t total = 1;
      for (uint32_t i = 0; i < tnum; i++) {
        uint64_t result = 0;
        pthread_join(threads[i], (void **)&result);
        total = MultModulo(total, result, mod);
      }

      printf("Total: %llu\n", total);

      char* buffer = (char*)malloc(sizeof(uint64_t));
      memcpy(buffer, &total, sizeof(total));

      err = send(client_fd, buffer, sizeof(total), 0);
      free(buffer);
      
      if (err < 0) {
        fprintf(stderr, "Can't send data to client\n");
        break;
      }
    }

    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);
  }
}

int main(int argc, char **argv) {
  int tnum = -1;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"tnum", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        tnum = atoi(optarg);
        if (tnum < 1)
        {
          printf("Tnum must be a positive number. Now tnum = %d", tnum);
          return 1;
        }
        // TODO: your code here
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
      break;
    }

    case '?':
      printf("Unknown argument\n");
      break;

    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (tnum == -1) {
    fprintf(stderr, "Using: %s --tnum [number]\n", argv[0]);
    return 1;
  }

  pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * tnum);
  for (int i = 0; i < tnum; i++)
  {
    if (pthread_create(&(threads[i]), NULL, parallelServerCreate, NULL))
    {
      printf("Error: pthread_create failed!\n");
      return 1;
    }
  }

  for (int i = 0; i < tnum; i++)
  {
    if (pthread_join(threads[i], NULL))
    {
      printf("Pthread didn't join\n");
      return 1;
    }
  }
  free(threads);

  char* fileName = "./ports.txt";
  if (remove(fileName))
  {
    printf("Unable to delete file: %s", fileName); 
  }

  return 0;
  /*int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    fprintf(stderr, "Can not create server socket!");
    return 1;
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons((uint16_t)port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

  int err = bind(server_fd, (struct sockaddr *)&server, sizeof(server));
  if (err < 0) {
    fprintf(stderr, "Can not bind to socket!");
    return 1;
  }

  err = listen(server_fd, 128);
  if (err < 0) {
    fprintf(stderr, "Could not listen on socket\n");
    return 1;
  }

  printf("Server listening at %d\n", port);

  while (true) {
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    int client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);

    if (client_fd < 0) {
      fprintf(stderr, "Could not establish new connection\n");
      continue;
    }

    while (true) {
      unsigned int buffer_size = sizeof(uint64_t) * 3;
      char from_client[buffer_size];
      int read = recv(client_fd, from_client, buffer_size, 0);

      if (!read)
        break;
      if (read < 0) {
        fprintf(stderr, "Client read failed\n");
        break;
      }
      if (read < buffer_size) {
        fprintf(stderr, "Client send wrong data format\n");
        break;
      }

      pthread_t threads[tnum];

      uint64_t begin = 0;
      uint64_t end = 0;
      uint64_t mod = 0;
      memcpy(&begin, from_client, sizeof(uint64_t));
      memcpy(&end, from_client + sizeof(uint64_t), sizeof(uint64_t));
      memcpy(&mod, from_client + 2 * sizeof(uint64_t), sizeof(uint64_t));

      fprintf(stdout, "Receive: %llu %llu %llu\n", begin, end, mod);

      struct FactorialArgs args[tnum];
      for (uint32_t i = 0; i < tnum; i++) {
        // TODO: parallel somehow
        args[i].begin = 1;
        args[i].end = 1;
        args[i].mod = mod;

        if (pthread_create(&threads[i], NULL, ThreadFactorial, (void *)&args[i])) {
          printf("Error: pthread_create failed!\n");
          return 1;
        }
      }

      uint64_t total = 1;
      for (uint32_t i = 0; i < tnum; i++) {
        uint64_t result = 0;
        pthread_join(threads[i], (void **)&result);
        total = MultModulo(total, result, mod);
      }

      printf("Total: %llu\n", total);

      char buffer[sizeof(total)];
      memcpy(buffer, &total, sizeof(total));
      err = send(client_fd, buffer, sizeof(total), 0);
      if (err < 0) {
        fprintf(stderr, "Can't send data to client\n");
        break;
      }
    }

    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);
  }

  return 0;*/
}