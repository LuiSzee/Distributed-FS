#ifndef DFS
#define DFS

#define FUSE_USE_VERSION 26

#include<pthread.h>
#include<fuse.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<dirent.h>
#include<errno.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>

#define DEBUG
#define MAX_CLIENTS 10
#define LISTEN_PORT 5000
#define CTRL_PORT 5001
#define MAXLEN 1030

#define BLOCKSIZE 1024

typedef struct file_info_ {
	char *path;
	int writer;
	int reader[10];
	int lease_expired;
	struct file_info_ *next;
} file_info;

typedef struct write_list_ {
	char *path;
	int write_back;
	struct write_file_ *next;	
} write_list;

typedef struct blocks
{
  int blockNumber;
  char blockData[BLOCKSIZE];
}blocks;

typedef struct client_info_{
	int id;
	int conn_socket;
	pthread_t thread;
} client_info;

typedef struct server_{
	char ip_addr[128];
	char iface_name[64];
	int listen_soc;
} server;

extern char *rootpath;
extern server s;
extern client_info *clientList;
extern int sock;
extern file_info *list_head;

server populatePublicIp(server);

#endif
