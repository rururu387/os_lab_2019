#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "pthread.h"
#include "inttypes.h"
#include "common.h"

pthread_mutex_t sockMutex = PTHREAD_MUTEX_INITIALIZER;

struct Server {
  char ip[255];
  int port;
};

struct ServInteractData
{
  struct Server* serv;
  char str[sizeof(uint64_t) * 3];
};


/*uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
  uint64_t result = 0;
  a = a % mod;
  while (b > 0) {
    if (b % 2 == 1)
      result = (result + a) % mod;
    a = (a * 2) % mod;
    b /= 2;
  }

  return result % mod;
}*/

bool ConvertStringToUI64(const char *str, uint64_t *val) {
  unsigned long long i = strtoull(str, NULL, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "Out of uint64_t range: %s\n", str);
    return false;
  }

  if (errno != 0)
    return false;

  *val = i;
  return true;
}

char* serverInteract(void* _servData)
{
  struct ServInteractData* servData = (struct ServInteractData*)_servData;

  printf("Getting host by name...\n");
  fflush(stdout);
  struct hostent *hostname = gethostbyname(servData->serv->ip);
  if (hostname == NULL) {
    fprintf(stderr, "gethostbyname failed with %s\n", servData->serv->ip);
    exit(1);
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  //Why no htons below?
  server.sin_port = servData->serv->port;
  server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr_list[0]);

  printf("Creating socket...\n");
  fflush(stdout);
  pthread_mutex_lock(&sockMutex);
  int sck = socket(AF_INET, SOCK_STREAM, 0);
  if (sck < 0) {
    fprintf(stderr, "Socket creation failed!\n");
    exit(1);
  }
  pthread_mutex_unlock(&sockMutex);

  //printf("Port: %d\n", server.sin_port);
  printf("Connecting to server...\n");
  fflush(stdout);
  if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
    fprintf(stderr, "Connection failed\n");
    exit(1);
  }
  printf("Connection successful!\n");
  fflush(stdout);

  printf("Sending data to server size to transfer: %d...\n", sizeof(servData->str));
  fflush(stdout);  
  if (send(sck, servData->str, sizeof(servData->str), 0) < 0) {
    fprintf(stderr, "Send failed\n");
    exit(1);
  }

  printf("Recieving data from server...\n");
  fflush(stdout);
  char* response = (char*)malloc(sizeof(uint64_t));
  if (recv(sck, response, sizeof(response), 0) < 0) {
    fprintf(stderr, "Recieve failed\n");
    exit(1);
  }
  close(sck);
  printf("Transaction complete!\n");
  fflush(stdout);

  return response;
}

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
  char servers[255] = {'\0'}; // TODO: explain why 255

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        // TODO: your code here
        if(!ConvertStringToUI64(optarg, &k) || k <= 0)
        {
          printf("Auchtung! K is invalid: %d", k);
          return 1;
        }
        
        break;
      case 1:
        //ConvertStringToUI64(optarg, &mod);
        // TODO: your code here
        if(!ConvertStringToUI64(optarg, &mod) || mod <= 0)
        {
          printf("Auchtung! Mod is invalid: %d", mod);
          return 1;
        }
        break;
      case 2:
        // TODO: your code here
        memcpy(servers, optarg, strlen(optarg));
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == -1 || mod == -1 || !strlen(servers)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
            argv[0]);
    return 1;
  }

  // TODO: for one server here, rewrite with servers from file

  char* fileName = "./ports.txt";
  FILE* fptr = fopen(fileName, "r");
  if (fptr == NULL)
  {
    fprintf(stderr, "Error opening file %s", fileName);
    return 1;
  }

  unsigned int serversCapacity = 1;
  unsigned int servers_num = 0;
  struct Server *to = (struct Server*)malloc(sizeof(struct Server) * serversCapacity);
  char a = 0;
  while(a != EOF)
  {
    a = fgetc(fptr);
    if (a == EOF)
      break;
    if (servers_num >= serversCapacity)
    {
      //struct Server *to2 = (struct Server*)realloc(to, sizeof(struct Server) * serversCapacity * 2);
      //Doesn't work for some reasons
      struct Server *to2 = (struct Server*)malloc(sizeof(struct Server) * serversCapacity * 2);
      memcpy(to2, to, sizeof(struct Server) * serversCapacity);
      if(to2 == NULL)
      {
        fprintf(stderr, "Could not allocate memory for array");
        return 1;
      }
      free(to);
      to = to2;
      serversCapacity = serversCapacity * 2;
    }

    char servIp[255];
    int servIpLen = 0;
    while(a != ' ' && a != '\n' && a != '\t' && a != EOF)
    {
      if(servIpLen >= 255)
      {
        printf("servIp is invalid. Char overflow");
        return 1;
      }
      servIp[servIpLen] = a;
      servIpLen++;
      a = fgetc(fptr);
    }
    servIp[servIpLen] = '\0';
    strcpy(to[servers_num].ip, servIp);
    
    fscanf (fptr, "%d", &(to[servers_num].port));
    a = fgetc(fptr);
    servers_num++;
  }

  if(fclose(fptr) != 0)
  {
    printf("Error closing file");
    exit(1);
  }

  uint64_t begin = 1;
  uint64_t step = k / servers_num;

  pthread_t* thArray = (pthread_t*)malloc(sizeof(pthread_t) * servers_num);
  
  int tAm = 0;

  struct ServInteractData** servInteractArray = (struct ServInteractData**)malloc(sizeof(struct ServInteractData**) * servers_num);
  for(; tAm < servers_num; tAm++)
  {
    char task[sizeof(uint64_t) * 3] = "";
    memcpy(task, &begin, sizeof(uint64_t));
    uint64_t end = (uint64_t)(begin + step);
    memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));
    memcpy(task + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));
    
    struct ServInteractData* servInteractPtr = (struct ServInteractData*)malloc(sizeof(struct ServInteractData));
    if (servInteractPtr == NULL)
    {
      printf("Could not allocate memory\n");
      return 1;
    }
    servInteractPtr->serv = &(to[tAm]);

    memcpy(servInteractPtr->str, task, sizeof(uint64_t) * 3);

    if (pthread_create(&(thArray[tAm]), NULL, (void*)serverInteract, (void*)servInteractPtr) != 0)
    {
      perror("pthread_create");
      exit(1);
    }
    servInteractArray[tAm] = servInteractPtr;
    begin += step;
  }

  uint64_t* resData = (uint64_t*)malloc(sizeof(uint64_t) * servers_num);
  for(tAm--; tAm >= 0; tAm--)
  {
    char* resCharPtr = (char*)malloc(sizeof(uint64_t));
    if (pthread_join(thArray[tAm], (void*)(&resCharPtr)) != 0)
    {
        perror("pthread_join");
        exit(1);
    }
    memcpy(&(resData[tAm]), resCharPtr, sizeof(resCharPtr));
    printf ("%d ", resData[tAm]);
  }
  printf("\n");
  fflush(stdout);

  free(thArray);
  for(int i = 0; i < servers_num; i++)
  {
    free(servInteractArray[i]);
  }
  free(servInteractArray);

  uint64_t ans = resData[0];
  for(int i = 1; i < servers_num; i++)
  {
    ans = MultModulo(ans, resData[i], mod);
  }
  
  printf("Answer: %llu\n", ans);
  
  free(resData);
  free(to);

  return 0;
}
