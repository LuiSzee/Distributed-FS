#include<stdio.h>
#include<string.h>
#include"dfs.h"

char tcp_buf[MAXLEN];

void* processClient(void* clientptr)
{
	int n = *((int*)clientptr) ;
	char *path;
//	client_info client = clientList[n];

	#ifdef DEBUG
		printf("Connected to client %d\n",n);
	#endif	

	//send initial data structures if any

	//wait for receiving the requests	
	while(1)
	{
		char *a;
		memset(tcp_buf,0,MAXLEN);
		recv(clientList[n].conn_socket,tcp_buf,MAXLEN,0);
		printf("Recvd %d\n%s\n",n,tcp_buf);
		a = strtok(tcp_buf,"\n");
		printf("%s\n",a);
		if(strcmp(a,"GETATTR") == 0)
		{
			printf("inside getattr\n");
			struct stat stbuf;
			int res;
			printf("Received message is %s\n",a);
			a = strtok(NULL,"\n");
			path = (char*)malloc(strlen(rootpath)+strlen(a)+5);
			strcpy(path,rootpath);
			strcat(path,a);
			printf("Path is %s\n",path);
			memset(tcp_buf,0,MAXLEN);
			res=lstat(path,&stbuf);
			if(res == -1)
			{
				sprintf(tcp_buf,"FAIL\n%d\n",errno);	
				send(clientList[n].conn_socket,tcp_buf,strlen(tcp_buf),0);	
			}
			else
			{
				printf("User id is %d\n",stbuf.st_uid);
				printf("Sent stbuf\n");
				sprintf(tcp_buf,"SUCCESS\n");	
				send(clientList[n].conn_socket,tcp_buf,strlen(tcp_buf),0);
				recv(clientList[n].conn_socket,tcp_buf,MAXLEN,0);
				send(clientList[n].conn_socket,(char*)&stbuf,sizeof(struct stat),0);
			}
			free(path);
			printf("getattr ends\n");
		}
		else if(strcmp(a,"MKNOD") == 0)
                {
			mode_t mode;
			dev_t rdev;
			int res;
                        
			printf("Received message is %s\n",a);
			a = strtok(NULL,"\n");
			path = (char*)malloc(strlen(rootpath)+strlen(a)+5);
			strcpy(path,rootpath);
			strcat(path,a);
			printf("Path is %s\n",path);
			
			memset(tcp_buf,0,MAXLEN);
			strcpy(tcp_buf,"ACK\n");
			send(clientList[n].conn_socket,tcp_buf,strlen(tcp_buf),0);

			recv(clientList[n].conn_socket,(char *)&mode,sizeof(mode_t),0);
			printf("Received mode\n");
			memset(tcp_buf,0,MAXLEN);
			strcpy(tcp_buf,"ACK\n");
			send(clientList[n].conn_socket,tcp_buf,strlen(tcp_buf),0);

			recv(clientList[n].conn_socket,(char *)&rdev,sizeof(dev_t),0);
			printf("Received dev\n");

			if (S_ISREG(mode)) {
				res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
				if (res >= 0)
					res = close(res);
			} else if (S_ISFIFO(mode))
				res = mkfifo(path, mode);
	 		else
				res = mknod(path, mode, rdev);

			memset(tcp_buf,0,MAXLEN);
			sprintf(tcp_buf,"%d",res);
			printf("Sent %s\n",tcp_buf);
			send(clientList[n].conn_socket,tcp_buf,strlen(tcp_buf),0);
			memset(tcp_buf,0,MAXLEN);
			free(path);	
                }
               else if(strcmp(a,"MKDIR") == 0)
                {
			int res;
			mode_t mode;

                        printf("Received message is %s\n",a);
			
			a = strtok(NULL,"\n");
			path = (char*)malloc(strlen(rootpath)+strlen(a)+5);
			strcpy(path,rootpath);
			strcat(path,a);
			printf("Path is %s\n",path);

                        memset(tcp_buf,0,MAXLEN);
			strcpy(tcp_buf,"ACK\n");
			printf("Sent %s",tcp_buf);
			send(clientList[n].conn_socket,tcp_buf,strlen(tcp_buf),0);

			recv(clientList[n].conn_socket,(char *)&mode,sizeof(mode_t),0);
                        printf("Received mode\n");
			res = mkdir(path, mode);

                        memset(tcp_buf,0,MAXLEN);
			if (res == -1)
				sprintf(tcp_buf,"FAIL\n%d\n",-errno);
			else
				sprintf(tcp_buf,"SUCCESS\n%d\n",0);
                
			send(clientList[n].conn_socket,tcp_buf,strlen(tcp_buf),0);
			free(path);
			memset(tcp_buf,0,MAXLEN);
                }
               else if(strcmp(a,"OPEN") == 0)
                {
 		  int res,flags;
		  printf("Received message is %s\n",a);
		  a = strtok(NULL,"\n");
		  path = (char*)malloc(strlen(rootpath)+strlen(a)+5);
		  strcpy(path,rootpath);
		  strcat(path,a);
		  printf("Path is %s\n",path);
		  memset(tcp_buf,0,MAXLEN);
		  send(clientList[n].conn_socket,"ok",strlen("ok"),0);
		  res=recv(clientList[n].conn_socket,tcp_buf,MAXLEN,0);
		  if(res<0){
		    printf("\nError receiving flags");
		    exit(1);
		  }
		  tcp_buf[res]='\0';
		  flags=atoi(tcp_buf);
		  res = open(path,flags);
		  if (res != -1)
		    send(clientList[n].conn_socket,"success",strlen("success"),0);
		  else
		    send(clientList[n].conn_socket,"failed",strlen("failed"),0);
		  close(res);
		free(path);
		memset(tcp_buf,0,MAXLEN);
                }
               else if(strcmp(a,"READ") == 0)
                {
		  int res,fd,flags;
		  printf("Received message is %s\n",a);fflush(stdout);
		  a = strtok(NULL,"\n");
		  path = (char*)malloc(strlen(rootpath)+strlen(a)+5);
		  strcpy(path,rootpath);
		  strcat(path,a);
		  printf("Path is %s\n",path);fflush(stdout);
		  memset(tcp_buf,0,MAXLEN);
		  send(clientList[n].conn_socket,"ok",strlen("ok"),0);
		  
		  res=recv(clientList[n].conn_socket,tcp_buf,MAXLEN,0);
		  if(res<0){
		    printf("\nError receiving flags");
		    exit(1);
		  }
		  tcp_buf[res]='\0';
		  flags=atoi(tcp_buf);
		  fd = open(path,flags);
		  if (fd != -1)
		    {
		      send(clientList[n].conn_socket,"success",strlen("success"),0);
		      memset(tcp_buf,0,MAXLEN);
		      res=recv(clientList[n].conn_socket,tcp_buf,MAXLEN,0);
		      if(res<0){
			printf("\nError receiving flags");
			exit(1);
		      }
		      tcp_buf[res]='\0';

		      int offset=atoi(tcp_buf);
		      memset(tcp_buf,0,MAXLEN);
		      strcpy(tcp_buf,"ok");
		      printf("here2");fflush(stdout);		      
		      while(strcmp(tcp_buf,"ok")==0)
			{
			  printf("here3");fflush(stdout);
			  memset(tcp_buf,0,MAXLEN);
			  int rflag=pread(fd,tcp_buf,BLOCKSIZE,offset);
			  printf("%s",tcp_buf);fflush(stdout);
			  if(rflag==-1)
			    {
			      send(clientList[n].conn_socket,"file_read_error",strlen("file_read_error"),0);
			    }
			  else
			    {
			      send(clientList[n].conn_socket,tcp_buf,strlen(tcp_buf),0);
			    }
			  

			  memset(tcp_buf,0,MAXLEN);
			  res=recv(clientList[n].conn_socket,tcp_buf,MAXLEN,0);
			  if(res<0){
			    printf("\nError receiving flags");
			    exit(1);
			  }
			  tcp_buf[res]='\0';
			  printf("%s",tcp_buf);fflush(stdout);
			  offset=offset+BLOCKSIZE;
			}
		      printf("here6");fflush(stdout);
		      memset(tcp_buf,0,MAXLEN);
		      //sleep(10);
		    }
		  else
		    {
		      //printf("here5");fflush(stdout);
		    send(clientList[n].conn_socket,"failed",strlen("failed"),0);
		    }
		      printf("here4");fflush(stdout);
		  close(res);

                }
               else if(strcmp(a,"GETDIR") == 0)
                {
                        struct stat stbuf;
			int res;
			printf("Received message is %s\n",a);
			a = strtok(NULL,"\n");
			path = (char*)malloc(strlen(rootpath)+strlen(a)+5);
			strcpy(path,rootpath);
			//strcat(path,"/");
			strcat(path,a);
			printf("Path is %s\n",path);
			memset(tcp_buf,0,MAXLEN);
			printf("Sent %s",tcp_buf);

			DIR *dp;
			struct dirent *de;
			printf("here1");fflush(stdout);
			dp = opendir(path);
			if (dp == NULL)
			  {
			    printf("Error during open dir");
			    // return -errno;
			  }
			send(clientList[n].conn_socket,"Start",strlen("Start"),0);
			recv(clientList[n].conn_socket,tcp_buf,MAXLEN,0);
			while ((de = readdir(dp)) != NULL) {
			  struct stat st;
			  memset(&st, 0, sizeof(st));
			  st.st_ino = de->d_ino;
			  st.st_mode = de->d_type << 12;
			  send(clientList[n].conn_socket,(char*)&stbuf,sizeof(struct stat),0);
			  recv(clientList[n].conn_socket,tcp_buf,MAXLEN,0);
			  send(clientList[n].conn_socket,de->d_name,strlen(de->d_name),0);
			//printf("here3");fflush(stdout);
	            }
        	 send(clientList[n].conn_socket,(char*)&stbuf,sizeof(struct stat),0);
		 recv(clientList[n].conn_socket,tcp_buf,MAXLEN,0);
		 send(clientList[n].conn_socket,"end",strlen("end"),0);
		free(path);
		 closedir(dp);
                }
               else if(strcmp(a,"ACCESS") == 0)
                {
			char *path;
			int mask;
			int res;
                        printf("Received message is %s\n",a);
			a = strtok(NULL,"\n");
			
			path = (char*)malloc(strlen(rootpath)+strlen(a)+5);
			strcpy(path,rootpath);
			strcat(path,a);
			printf("Path is %s\n",path);

			a = strtok(NULL,"\n");
			mask = atoi(a);
			printf("Mask is %d\n",mask);
			
			memset(tcp_buf,0,MAXLEN);
		        res = access(path, mask);
		        if (res == -1)
                		sprintf(tcp_buf,"FAIL\n%d\n",-errno);	
			else
				sprintf(tcp_buf,"SUCCESS\n%d\n",0);
			printf("Sent %s",tcp_buf);
			send(clientList[n].conn_socket,tcp_buf,strlen(tcp_buf),0);
			memset(tcp_buf,0,MAXLEN);
			free(path);
                }
               else if(strcmp(a,"CHMOD") == 0)
                {
			int res;
                        mode_t mode;

                        printf("Received message is %s\n",a);

			a = strtok(NULL,"\n");
                        path = (char*)malloc(strlen(rootpath)+strlen(a)+5);
                        strcpy(path,rootpath);
                        strcat(path,a);
                        printf("Path is %s\n",path);

                        memset(tcp_buf,0,MAXLEN);
                        strcpy(tcp_buf,"ACK\n");
                        printf("Sent %s",tcp_buf);
                        send(clientList[n].conn_socket,tcp_buf,strlen(tcp_buf),0);

                        recv(clientList[n].conn_socket,(char *)&mode,sizeof(mode_t),0);
                        printf("Received mode\n");
			fflush(stdout);
                        res = chmod(path, mode);

                        memset(tcp_buf,0,MAXLEN);
                        if (res == -1)
                                sprintf(tcp_buf,"FAIL\n%d\n",-errno);
                        else
                                sprintf(tcp_buf,"SUCCESS\n%d\n",0);

                        send(clientList[n].conn_socket,tcp_buf,strlen(tcp_buf),0);
			free(path);
			memset(tcp_buf,0,MAXLEN);
			printf("Chmod completed\n");
			fflush(stdout);
                }
               else if(strcmp(a,"CHOWN") == 0)
                {
			int res;
			uid_t uid;
			gid_t gid;

			printf("Received message is %s\n",a);

                        a = strtok(NULL,"\n");
                        path = (char*)malloc(strlen(rootpath)+strlen(a)+5);
                        strcpy(path,rootpath);
                        strcat(path,a);
                        printf("Path is %s\n",path);

			memset(tcp_buf,0,MAXLEN);
                        strcpy(tcp_buf,"ACK\n");
                        printf("Sent %s",tcp_buf);
                        send(clientList[n].conn_socket,tcp_buf,strlen(tcp_buf),0);

                        recv(clientList[n].conn_socket,(char *)&uid,sizeof(uid_t),0);
                        printf("Received uid\n");

			memset(tcp_buf,0,MAXLEN);
                        strcpy(tcp_buf,"ACK\n");
                        printf("Sent %s",tcp_buf);
                        send(clientList[n].conn_socket,tcp_buf,strlen(tcp_buf),0);

                        recv(clientList[n].conn_socket,(char *)&gid,sizeof(gid_t),0);
                        printf("Received gid\n");

			res = lchown(path, uid, gid);

			memset(tcp_buf,0,MAXLEN);
                        if (res == -1)
                                sprintf(tcp_buf,"FAIL\n%d\n",-errno);
                        else
                                sprintf(tcp_buf,"SUCCESS\n%d\n",0);

                        send(clientList[n].conn_socket,tcp_buf,strlen(tcp_buf),0);
			memset(tcp_buf,0,MAXLEN);
			free(path);
                }
               else if(strcmp(a,"RMDIR") == 0)
                {
                        printf("Received message is %s\n",a);
                        memset(tcp_buf,0,MAXLEN);
                        strcpy(tcp_buf,"ACK\n");
			printf("Sent %s",tcp_buf);
				send(clientList[n].conn_socket,tcp_buf,strlen(tcp_buf),0);
                }
               else if(strcmp(a,"RENAME") == 0)
                {
                        printf("Received message is %s\n",a);
                        memset(tcp_buf,0,MAXLEN);
                        strcpy(tcp_buf,"ACK\n");
			printf("Sent %s",tcp_buf);
				send(clientList[n].conn_socket,tcp_buf,strlen(tcp_buf),0);
                }
               else if(strcmp(a,"SYMLINK") == 0)
                {
                        printf("Received message is %s\n",a);
                        memset(tcp_buf,0,MAXLEN);
                        strcpy(tcp_buf,"ACK\n");
			printf("Sent %s",tcp_buf);
				send(clientList[n].conn_socket,tcp_buf,strlen(tcp_buf),0);
                }
               else if(strcmp(a,"LINK") == 0)
                {
                        printf("Received message is %s\n",a);
                        memset(tcp_buf,0,MAXLEN);
                        strcpy(tcp_buf,"ACK\n");
			printf("Sent %s",tcp_buf);
				send(clientList[n].conn_socket,tcp_buf,strlen(tcp_buf),0);
                }
               else if(strcmp(a,"UNLINK") == 0)
                {
                        printf("Received message is %s\n",a);
                        memset(tcp_buf,0,MAXLEN);
                        strcpy(tcp_buf,"ACK\n");
			printf("Sent %s",tcp_buf);
				send(clientList[n].conn_socket,tcp_buf,strlen(tcp_buf),0);
                }
		
	}
}

int main()
{
        int i=0,j;
        initServer();
        while(i<MAX_CLIENTS)
        {
                clientList[i].conn_socket = acceptConnection(s.listen_soc);
                clientList[i].id = i;

                #ifdef DEBUG
                        printf("Connected to client %d\n",i);
                #endif
		j=i;
                pthread_create(&clientList[i].thread,NULL,processClient,(void*)&j);
                i++;

        }

        for(j=0;j<i;j++)
                pthread_join(clientList[j].thread,NULL);

        return 0;
}

