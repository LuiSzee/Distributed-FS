#include<stdio.h>
#include <string.h>
#include <fcntl.h>
#include<errno.h>
#include"dfs.h"

char tcp_buf[MAXLEN];
char *a;

//below is implementation of all FS functions. They should be split in client and server
static int dfs_getattr(const char *path, struct stat *stbuf)
{
	int res;
//client side code goes here

	struct stat temp_stbuf;
        FILE *fp;
        char *fname;

	printf("Inside getattr Path is: %s\n",path);
	memset(tcp_buf,0,MAXLEN);
	sprintf(tcp_buf,"GETATTR\n%s\n",path);
	printf("path copied\n%s\n",tcp_buf);	
	//res=lstat(path,stbuf);

//tcp code goes here
	send(sock,tcp_buf,strlen(tcp_buf),0);
	memset(tcp_buf,0,MAXLEN);
	recv(sock,tcp_buf,MAXLEN,0);
	printf("recvd\n");
	

	a = strtok(tcp_buf,"\n");
	if(strcmp(a,"FAIL")==0)
	{
		printf("getattr failed\n");
		a = strtok(NULL,"\n");
		errno = atoi(a);
		return -errno;
	}

	printf("%s\n",a);
	memset(tcp_buf,0,MAXLEN);
	strcpy(tcp_buf,"ACK\n");
	send(sock,tcp_buf,strlen(tcp_buf),0);
	recv(sock,(char *)&temp_stbuf,sizeof(struct stat),0);
	printf("Received stbuf\n");	
	
//rest of the code goes here

	stbuf->st_dev = temp_stbuf.st_dev;
	stbuf->st_ino = temp_stbuf.st_ino;
	stbuf->st_mode = temp_stbuf.st_mode;
	stbuf->st_nlink = temp_stbuf.st_nlink;
	stbuf->st_uid = temp_stbuf.st_uid;
	stbuf->st_gid = temp_stbuf.st_gid;
	stbuf->st_rdev = temp_stbuf.st_rdev;
	stbuf->st_size = temp_stbuf.st_size;
	stbuf->st_blksize = temp_stbuf.st_blksize;
	stbuf->st_blocks = temp_stbuf.st_blocks;
	stbuf->st_atime = temp_stbuf.st_atime;
	stbuf->st_mtime = temp_stbuf.st_mtime;
	stbuf->st_ctime = temp_stbuf.st_ctime;

//	printf("User id %d\n",stbuf->st_uid);

	printf("End of getattr\n");
	return 0;
}

static int dfs_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;
	printf("Inside mknod Path is: %s\n",path);

//client side code goes here

        memset(tcp_buf,0,MAXLEN);
	sprintf(tcp_buf,"MKNOD\n%s\n",path);	


//tcp code goes here
	send(sock,tcp_buf,strlen(tcp_buf),0);
	recv(sock,tcp_buf,MAXLEN,0);

	send(sock,(char*)&mode,sizeof(mode_t),0);
	recv(sock,tcp_buf,MAXLEN,0);

	send(sock,(char*)&rdev,sizeof(dev_t),0);

	memset(tcp_buf,0,MAXLEN);
	recv(sock,tcp_buf,MAXLEN,0);
	printf("Received message: %s\n",tcp_buf);	
	sscanf(tcp_buf,"%d",&res);

//rest of the code goes here
	if (res == -1)
		return -errno;
	printf("End of mknod\n");
	return 0;
}

static int dfs_mkdir(const char *path, mode_t mode)
{
	int res;
//client side code goes here
        printf("Inside mkdir Path is: %s\n",path);
        memset(tcp_buf,0,MAXLEN);
        sprintf(tcp_buf,"MKDIR\n%s\n",path);    

//tcp code goes here
        send(sock,tcp_buf,strlen(tcp_buf),0);
        recv(sock,tcp_buf,MAXLEN,0);

        send(sock,(char*)&mode,sizeof(mode_t),0);

        memset(tcp_buf,0,MAXLEN);
        recv(sock,tcp_buf,MAXLEN,0);
        printf("Received message: %s\n",tcp_buf);       

	a = strtok(tcp_buf,"\n");
	a = strtok(NULL,"\n");
	res = atoi(a);
	printf("End of mkdir\n");	
        return res;
}

static int dfs_open(const char *path, struct fuse_file_info *fi)
{
        printf("Inside open Path is: %s\n",path);
        memset(tcp_buf,0,MAXLEN);
        sprintf(tcp_buf,"OPEN\n%s",path);

//tcp code goes here
       
        send(sock,tcp_buf,strlen(tcp_buf),0);
        memset(tcp_buf,0,MAXLEN);
        recv(sock,tcp_buf,MAXLEN,0);
	
        memset(tcp_buf,0,MAXLEN);
	sprintf(tcp_buf,"%d",fi->flags);
        send(sock,tcp_buf,strlen(tcp_buf),0);
	memset(tcp_buf,0,MAXLEN);
	recv(sock,tcp_buf,MAXLEN,0);

	//	if(strcmp(tcp_buf,"success")!=0)
	printf("\n%s\n",tcp_buf);
	printf("End of open\n");
        return 0;

}

void writeToFile(FILE *fp,blocks *temp)
{
  printf("writing to file");fflush(stdout);
  printf("%d",temp->blockNumber);fflush(stdout);
  //	 sleep(5);
  fwrite(temp,sizeof(blocks),1,fp);
}

int searchFile(int a)
{
  return(0);
}

static int dfs_read(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi)
{
	int ret;
  	printf("Inside read. Path is: %s \n",path);
        memset(tcp_buf,0,MAXLEN);
        sprintf(tcp_buf,"READ\n%s\n",path);

	char cacheFile[100];
	char *tempBuf;
	tempBuf=(char *)malloc(sizeof(char)*((int)size*2));
	FILE *fd;
	int recvflag;
	strcpy(cacheFile,".");
	strcat(cacheFile,path);
	strcat(cacheFile,".cache");
	//fd=fopen(cacheFile,"ab+");
	//	printf("cacheFile %s",cacheFile);fflush(stdout);
//tcp code goes here
       
        send(sock,tcp_buf,strlen(tcp_buf),0);
        memset(tcp_buf,0,MAXLEN);
        recv(sock,tcp_buf,MAXLEN,0);
	
        memset(tcp_buf,0,MAXLEN);
	sprintf(tcp_buf,"%d",fi->flags);
        send(sock,tcp_buf,strlen(tcp_buf),0);
	memset(tcp_buf,0,MAXLEN);
	recv(sock,tcp_buf,MAXLEN,0);
	
	if(strcmp(tcp_buf,"success")==0)
	{
		printf("open successful\n");
	    	memset(tcp_buf,0,MAXLEN);
	    	sprintf(tcp_buf,"%d",(int)offset);	//send offset as off_t
	    	send(sock,tcp_buf,strlen(tcp_buf),0);
		printf("sent offset\n");
	    	strcpy(tempBuf,"");
	    	int nsize,noff;
	    	noff=(int)offset/BLOCKSIZE;
	    	nsize=((int)size/BLOCKSIZE)+1;	//this cud be issue
		printf("offset is %d and blockes are %d\n",noff,nsize);

	     
	     	//printf("\n\nnoff %d offset %d\nnsize %d size %d",noff,(int)offset,nsize,(int)size);fflush(stdout);
	    	int i;
	    	for(i=0;i<nsize;i++)
	      	{
			if(searchFile(i+noff)==1)
		  	{


		  	}
			else
		  	{
		  		char *b;
		    		memset(tcp_buf,0,MAXLEN);
		    		recvflag=recv(sock,tcp_buf,MAXLEN,0);
				printf("recvd block %d\n",i+noff);
		    		if(recvflag<0)
		      		{
					printf("Receiving error");
					exit(0);
		      		}
				a = strtok_r(tcp_buf,"\n",&b);
				if(strcmp(a,"NEXT")!=0)
				{
					printf("eroor or EOF\n");
					a = strtok_r(NULL,"\n",&b);
					ret = atoi(a);
					break;
				}	
		
		    		//if(i==nsize-1)
		    		// tcp_buf[recvflag]='\0';
		    		//strcat(
		    		//tcp_buf[recvflag]='\0';
		
		    		//tcp_buf[recvflag]='\0';
		
		    		/*blocks *temp;
		    		temp=(blocks *)malloc(sizeof(blocks));
		    		temp->blockNumber=noff+i;
		    		strcpy(temp->blockData,tcp_buf);
		    		temp->blockData[recvflag]='\0';
		    		writeToFile(fd,temp);*/
		    		//	printf("tcp_buf %s",tcp_buf);fflush(stdout);

		    		/*if(recvflag<BLOCKSIZE){
		      			//tcp_buf[recvflag]='\0';
		      			send(sock,"next",strlen("next"),0);
		      			printf("next\ni is %d and nsize is %d\n",i,nsize);		  
		      			break;
		    		}
		    		else*/
				//a = strtok(NULL,"\n");
				strcat(tempBuf,b);
		       		memset(tcp_buf,0,MAXLEN);

		    		if(i+1==nsize)
		      		{
					send(sock,"FINISH",strlen("FINISH"),0);
					printf("finished reading\ni is %d and nsize is%d\n",i,nsize);
					recv(sock,tcp_buf,strlen(tcp_buf),0);
					ret = atoi(tcp_buf);
					printf("recvd bytes read\n");
		      		}
		    		else
		      		{
					send(sock,"NEXT",strlen("NEXT"),0);
					printf("need next block\ni is %d and nsize is %d\n",i,nsize);
		      		}
		    		fflush(stdout);
		    		//sleep(1);
		  	}
	      	}
	}
	else
	{
		printf("error\n");
		printf("\n%s\n",tcp_buf);
		a = strtok(tcp_buf,"\n");
		a = strtok(NULL,"\n");
		ret = atoi(a);
	}
//	strcat(tempBuf,tcp_buf);
//	strcat(tempBuf,"\n");
	memcpy(buf,tempBuf,size);
	
	printf("%shere5",buf);fflush(stdout);
	//fclose(fd);
	printf("End of read\nret is %d\n",ret);
	free(tempBuf);
	//int ret=atoi(tcp_buf);
        return ret;
}

static int dfs_write(const char *path, const char *buf, size_t size,off_t offset, struct fuse_file_info *fi)
{
	printf("Inside write. Path is: %s buf is %s\n",path,buf);
        memset(tcp_buf,0,MAXLEN);
        sprintf(tcp_buf,"WRITE\n%s\n",path);

	char wrtFile[100];
	char tempBuf[MAXLEN];
	char *bufptr;
	int ret;
	//tempBuf=(char *)malloc(sizeof(char)*(int)size);
	FILE *fd;
	int recvflag;
	strcpy(wrtFile,".");
	strcat(wrtFile,path);
	strcat(wrtFile,".wrt");
//	fd=fopen(wrtFile,"ab+");
	//	printf("cacheFile %s",cacheFile);fflush(stdout);
//tcp code goes here
       
        send(sock,tcp_buf,strlen(tcp_buf),0);
        memset(tcp_buf,0,MAXLEN);
        recv(sock,tcp_buf,MAXLEN,0);
	printf("recvd\n%s\n",tcp_buf);
	
        memset(tcp_buf,0,MAXLEN);
	sprintf(tcp_buf,"%d",fi->flags);
        send(sock,tcp_buf,strlen(tcp_buf),0);
	memset(tcp_buf,0,MAXLEN);
	recv(sock,tcp_buf,MAXLEN,0);
	printf("recvd (this should be success)\n%s\n",tcp_buf);
	
	if(strcmp(tcp_buf,"success")==0)
	{
		int i;
		printf("file opened\n");
	    	memset(tcp_buf,0,MAXLEN);
	    	sprintf(tcp_buf,"%d",(int)offset);
	    	send(sock,tcp_buf,strlen(tcp_buf),0);
	    	strcpy(tempBuf,"");
	    	int nsize,noff;
		printf("size is %d\n",size);
	    	noff=(int)offset/BLOCKSIZE;
	    	nsize=((int)size-1)/BLOCKSIZE+1;
		printf("offset is %d\nand blocks are %d\n",noff,nsize);
	    	recv(sock,tcp_buf,MAXLEN,0);
		printf("recvd\n%s\n",tcp_buf);
		bufptr = buf;
		printf("starting the write loop\n");
		for(i=0;i<nsize;i++)
		{
			memset(tempBuf,0,BLOCKSIZE);
			memcpy(tempBuf,bufptr,BLOCKSIZE);
			sprintf(tcp_buf,"NEXT\n%s",tempBuf);
	    		send(sock,tcp_buf,strlen(tcp_buf),0);
	    		recv(sock,tcp_buf,MAXLEN,0);
			a = strtok(tcp_buf,"\n");
			if(strcmp(a,"ERROR")==0)
			{
				a = strtok(NULL,"\n");
				ret = atoi(a);
				printf("Error\nexiting write\n");
				return ret;
			}
			printf("recvd\n%s\n",tcp_buf);
			printf("Wrote block %d\n",i+noff);
			bufptr = bufptr + BLOCKSIZE;	
		}
	}
	else
	{
		printf("\n%s\n",tcp_buf);
		a = strtok(tcp_buf,"\n");
		a = strtok(NULL,"\n");
		ret = atoi(a);
		printf("Error\nexiting write\n");
		return ret;
	}
	     
//	fclose(fd);
	printf("End of write\n");
	send(sock,"END",strlen("END"),0);
	memset(tcp_buf,0,MAXLEN);
	int rf=recv(sock,tcp_buf,MAXLEN,0);
	printf("bytes written\n%s\n",tcp_buf);
//	tcp_buf[rf]='\0';
	ret=atoi(tcp_buf);
	printf("%d bytes wrote\n",ret);
        return ret;
}


static int dfs_getdir(const char *path, void *buf, fuse_fill_dir_t filler,off_t offset, struct fuse_file_info *fi)
{
//client side code goes here
        printf("Inside getdir Path is: %s\n",path);
        memset(tcp_buf,0,MAXLEN);
    sprintf(tcp_buf,"GETDIR\n%s\n",path);


//tcp code goes here
        send(sock,tcp_buf,strlen(tcp_buf),0);
        memset(tcp_buf,0,MAXLEN);
        int recFlag=recv(sock,tcp_buf,MAXLEN,0);
    if(recFlag<0){
      printf("Error while receiving");
      exit(1);
    }

        send(sock,"ok",strlen("ok"),0);
    int flag=1;
    while(1)
      {
        struct stat tempSt;
        recv(sock,(char*)&tempSt,sizeof(struct stat),0);
            send(sock,"ok",strlen("ok"),0);
        int recFlag=recv(sock,tcp_buf,MAXLEN,0);
        if(recFlag<0){
          printf("Error while receiving");
          exit(1);
        }
        tcp_buf[recFlag]='\0';
	if(strcmp(tcp_buf,"end")==0)
	  break;
        if (filler(buf, tcp_buf, &tempSt, 0))
          {
	    flag=0;
	    break;
          }
      }
//rest of the code goes here
        return 0;
}


static int dfs_access(const char *path, int mask)
{
	int res;
//client side code goes here
        printf("Inside access Path is: %s\n",path);
        memset(tcp_buf,0,MAXLEN);
        sprintf(tcp_buf,"ACCESS\n%s\n%d",path,mask);

//tcp code goes here
        send(sock,tcp_buf,strlen(tcp_buf),0);
        memset(tcp_buf,0,MAXLEN);
        recv(sock,tcp_buf,MAXLEN,0);

//rest of the code goes here
        printf("Received message: %s\n",tcp_buf);
	a = strtok(tcp_buf,"\n");
	a = strtok(NULL,"\n");
	res = atoi(a);
        return res;
}

static int dfs_chmod(const char *path, mode_t mode)
{

        int res;
//client side code goes here
        printf("Inside chmod Path is: %s\n",path);
        memset(tcp_buf,0,MAXLEN);
        sprintf(tcp_buf,"CHMOD\n%s\n",path);

//tcp code goes here
        send(sock,tcp_buf,strlen(tcp_buf),0);
        recv(sock,tcp_buf,MAXLEN,0);

        send(sock,(char*)&mode,sizeof(mode_t),0);

        memset(tcp_buf,0,MAXLEN);
        recv(sock,tcp_buf,MAXLEN,0);
        printf("Received message: %s\n",tcp_buf);

        a = strtok(tcp_buf,"\n");
        a = strtok(NULL,"\n");
        res = atoi(a);
	printf("res is %d\n",res);
        return res;
}

static int dfs_chown(const char *path, uid_t uid, gid_t gid)
{

        int res;
//client side code goes here
        printf("Inside chown Path is: %s\n",path);
        memset(tcp_buf,0,MAXLEN);
        sprintf(tcp_buf,"CHOWN\n%s\n",path);

//tcp code goes here
        send(sock,tcp_buf,strlen(tcp_buf),0);
        recv(sock,tcp_buf,MAXLEN,0);

        send(sock,(char*)&uid,sizeof(uid_t),0);
        recv(sock,tcp_buf,MAXLEN,0);
        send(sock,(char*)&gid,sizeof(uid_t),0);

        memset(tcp_buf,0,MAXLEN);
        recv(sock,tcp_buf,MAXLEN,0);
        printf("Received message: %s\n",tcp_buf);

        a = strtok(tcp_buf,"\n");
        a = strtok(NULL,"\n");
        res = atoi(a);
        return res;
}

static int dfs_rmdir(const char *path)
{
	int res;
//client side code goes here
        printf("Inside rmdir path is: %s\n",path);
        memset(tcp_buf,0,MAXLEN);
        sprintf(tcp_buf,"RMDIR\n%s\n",path);

//tcp code goes here
        send(sock,tcp_buf,strlen(tcp_buf),0);
        memset(tcp_buf,0,MAXLEN);
        recv(sock,tcp_buf,MAXLEN,0);

//rest of the code goes here
        printf("Received message: %s\n",tcp_buf);
        a = strtok(tcp_buf,"\n");
        a = strtok(NULL,"\n");
        res = atoi(a);
	printf("rmdir ends\n");
        return res;
}

static int dfs_rename(const char *from, const char *to)
{
	int res;
//client side code goes here
        printf("Inside rename Path is: %s\n",from);
        memset(tcp_buf,0,MAXLEN);
        sprintf(tcp_buf,"RENAME\n%s\n%s\n",from,to);

//tcp code goes here
        send(sock,tcp_buf,strlen(tcp_buf),0);
        memset(tcp_buf,0,MAXLEN);
        recv(sock,tcp_buf,MAXLEN,0);

//rest of the code goes here
        printf("Received message: %s\n",tcp_buf);
        a = strtok(tcp_buf,"\n");
        a = strtok(NULL,"\n");
        res = atoi(a);
	printf("rename ends\n");
        return res;
}

static int dfs_symlink(const char *from, const char *to)
{
	int res;
//client side code goes here
        printf("Inside symlink Path is: %s\n",from);
        memset(tcp_buf,0,MAXLEN);
        sprintf(tcp_buf,"SYMLINK\n%s\n%s\n",from,to);

//tcp code goes here
        send(sock,tcp_buf,strlen(tcp_buf),0);
        memset(tcp_buf,0,MAXLEN);
        recv(sock,tcp_buf,MAXLEN,0);

//rest of the code goes here
        printf("Received message: %s\n",tcp_buf);
        a = strtok(tcp_buf,"\n");
        a = strtok(NULL,"\n");
        res = atoi(a);
	printf("symlink ends\n");
        return res;
}

static int dfs_link(const char *from, const char *to)
{ 
	int res;
//client side code goes here
        printf("Inside link Path is: %s\n",from);
        memset(tcp_buf,0,MAXLEN);
        sprintf(tcp_buf,"LINK\n%s\n%s\n",from,to);

//tcp code goes here
        send(sock,tcp_buf,strlen(tcp_buf),0);
        memset(tcp_buf,0,MAXLEN);
        recv(sock,tcp_buf,MAXLEN,0);

//rest of the code goes here
        printf("Received message: %s\n",tcp_buf);
        a = strtok(tcp_buf,"\n");
        a = strtok(NULL,"\n");
        res = atoi(a);
	printf("link ends\n");
        return res;
}

static int dfs_unlink(const char *path)
{
	int res;
//client side code goes here
        printf("Inside unlink path is: %s\n",path);
        memset(tcp_buf,0,MAXLEN);
        sprintf(tcp_buf,"UNLINK\n%s\n",path);

//tcp code goes here
        send(sock,tcp_buf,strlen(tcp_buf),0);
        memset(tcp_buf,0,MAXLEN);
        recv(sock,tcp_buf,MAXLEN,0);

//rest of the code goes here
        printf("Received message: %s\n",tcp_buf);
        a = strtok(tcp_buf,"\n");
        a = strtok(NULL,"\n");
        res = atoi(a);
	printf("unlink ends\n");
        return res;
}

static int dfs_flush(const char *path, struct fuse_file_info *fi)
{
	//will be called when close is called on the file specified by path.
	//Write back the cached data and delete the cache files
	return 0;
}

static int dfs_utimens(const char *path, const struct timespec ts[2])
{
	int res;

	//client side code goes here
	printf("Inside utime Path is: %s\n",path);
        memset(tcp_buf,0,MAXLEN);
        sprintf(tcp_buf,"UTIME\n%s\n",path);

//tcp code goes here
	send(sock,tcp_buf,strlen(tcp_buf),0);
	recv(sock,tcp_buf,MAXLEN,0);

	send(sock,(char*)&ts[0],sizeof(struct timespec),0);
	recv(sock,tcp_buf,MAXLEN,0);
	send(sock,(char*)&ts[1],sizeof(struct timespec),0);

	memset(tcp_buf,0,MAXLEN);
	recv(sock,tcp_buf,MAXLEN,0);
	printf("Received message: %s\n",tcp_buf);

	a = strtok(tcp_buf,"\n");
	a = strtok(NULL,"\n");
	res = atoi(a);
	printf("utimens ends\n");
	return res;
}
static int dfs_readlink(const char *path, char *buf, size_t size)
{
	int res;
//client side code goes here
	printf("Inside readlink Path is: %s\n",path);
	memset(tcp_buf,0,MAXLEN);
	sprintf(tcp_buf,"READLINK\n%s\n",path);

//tcp code goes here
	send(sock,tcp_buf,strlen(tcp_buf),0);
	recv(sock,tcp_buf,MAXLEN,0);

	send(sock,(char*)&size,sizeof(size_t),0);

	memset(tcp_buf,0,MAXLEN);
	recv(sock,tcp_buf,MAXLEN,0);
	printf("Received message: %s\n",tcp_buf);

	a = strtok(tcp_buf,"\n");
	if(strcmp(a,"SUCCESS")==0)
	{
		a=strtok(NULL,"\n");
		strcpy(buf,a);
	}
	a = strtok(NULL,"\n");
	res = atoi(a);

	printf("readlink ends\n");
	return res;
}
static int dfs_setxattr(const char *path, const char *name, const char *value, size_t size, int flags)
{
	int res;
//client side code goes here
	printf("Inside setxattr Path is: %s\n",path);
	memset(tcp_buf,0,MAXLEN);
	sprintf(tcp_buf,"SETXATTR\n%s\n%s\n%s\n%d\n",path,name,value,flags);    

//tcp code goes here
	send(sock,tcp_buf,strlen(tcp_buf),0);
	recv(sock,tcp_buf,MAXLEN,0);

	send(sock,(char*)&size,sizeof(size_t),0);

	memset(tcp_buf,0,MAXLEN);
	recv(sock,tcp_buf,MAXLEN,0);
	printf("Received message: %s\n",tcp_buf);       

	a = strtok(tcp_buf,"\n");
	a = strtok(NULL,"\n");
	res = atoi(a);
	printf("End of setxattr\n");	
	return res;
}

static int dfs_getxattr(const char *path, const char *name, char *value, size_t size)
{
	char *b;
	int res;
//client side code goes here
        printf("Inside getxattr Path is: %s\n",path);
        memset(tcp_buf,0,MAXLEN);
        sprintf(tcp_buf,"GETXATTR\n%s\n%s\n",path,name);    

//tcp code goes here
        send(sock,tcp_buf,strlen(tcp_buf),0);

        memset(tcp_buf,0,MAXLEN);
        recv(sock,tcp_buf,MAXLEN,0);
        printf("Received message: %s\n",tcp_buf);       

	a = strtok(tcp_buf,"\n");
	b = strtok(NULL,"\n");
	res = atoi(b);
	value = NULL;
	size = 0;
	if(strcmp(a,"SUCCESS")==0)
	{
		a=strtok(NULL,"\n");
	//	value = malloc(sizeof(char)*strlen(a)+1);
		strcpy(value,a);
		send(sock,"ACK\n",strlen("ACK\n"),0);
	        recv(sock,(char*)&size,sizeof(size_t),0);
	}

	printf("End of getxattr\n");	
        return res;
}


//below is client code
static struct fuse_operations dfs_oper = {
.getattr = (void *)dfs_getattr,
.mknod = (void *)dfs_mknod,
.mkdir = (void *)dfs_mkdir,
.open = (void *)dfs_open,
.read = (void *)dfs_read,
.write = (void *)dfs_write,
.readdir = (void *)dfs_getdir,
.access = (void *)dfs_access,
.chmod = (void *)dfs_chmod,
.chown = (void *)dfs_chown,
.rmdir = (void *)dfs_rmdir,
.rename = (void *)dfs_rename,
.symlink = (void *)dfs_symlink,
.link = (void *)dfs_link,
.unlink = (void *)dfs_unlink,
.flush = (void*)dfs_flush, 
.utimens = (void*)dfs_utimens,
.readlink = (void*)dfs_readlink,
.getxattr = (void*)dfs_getxattr,
.setxattr = (void*)dfs_setxattr,
};

int main(int argc, char *argv[])
{
	int i;
	initClient(argc,argv);
	for(i=1;i<argc;i++)
		argv[i] = argv[i+1];
	argc--;
	umask(0);
	return fuse_main(argc, argv, &dfs_oper, NULL);	
}
