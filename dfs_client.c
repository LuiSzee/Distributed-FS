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
	int rf=0;
//client side code goes here

	struct stat temp_stbuf;
        FILE *fp;
        char *fname;
	char *cachefile;	
	printf("Inside getattr Path is: %s\n",path);
	cachefile = (char*)malloc(strlen(rootpath)+strlen(path)+10);
	strcpy(cachefile,rootpath);
	strcat(cachefile,path);
	strcat(cachefile,".attrib");
	//cachefile[0]='.';
	printf("cache file is %s\n",cachefile);
	fp = fopen(cachefile,"rb");	
	if(fp!=NULL)
	{
		rf = fread(&temp_stbuf,sizeof(temp_stbuf),1,fp);
		fclose(fp);
	}

	if(!rf)
	{
		printf("data not cached\n");
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

		fp = fopen(cachefile,"wb+");	
		if(fp!=NULL)
		{
			rf = fwrite(&temp_stbuf,sizeof(temp_stbuf),1,fp);
			fclose(fp);
		}
	}
	else
		printf("cached data found\n%s\n",cachefile);
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
  int ret=0;      
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
	int res=recv(sock,tcp_buf,MAXLEN,0);
	if(res<0){
	  printf("\nError receiving flags");
	  exit(1);
	}
	tcp_buf[res]='\0';

	if(strcmp(tcp_buf,"failed")==0)
	  {
	  printf("sending errno");fflush(stdout);
	    send(sock,tcp_buf,strlen(tcp_buf),0);
	    memset(tcp_buf,0,MAXLEN);
	    res=recv(sock,tcp_buf,MAXLEN,0);
	    if(res<0){
	      printf("\nError receiving flags");
	      exit(1);
	    }
	    tcp_buf[res]='\0';
	    ret=atoi(tcp_buf);
	  }

	//	if(strcmp(tcp_buf,"success")!=0)
	printf("\n%s\n",tcp_buf);
	printf("End of open\n");
        return ret;

}

void writeToFile(FILE *fp,blocks *temp)
{
  printf("writing to file");fflush(stdout);
  printf("%d",temp->blockNumber);fflush(stdout);
    printf("\n%s\n\n%d\n",temp->blockData,temp->blockNumber);
  //	 sleep(5);
  fwrite(temp,sizeof(blocks),1,fp);
}

int searchFile(int blkNo,FILE *fd)
{
   FILE *fp;
   //  fp=fopen("abc.txt.cache","rb");
  
  blocks temp;  
  
  while(fread(&temp,sizeof(blocks),1,fd)){
    if(temp.blockNumber==blkNo){
      printf("present in cache");
      return(1);
    }
    printf("\n%s\n\n%d\n",temp.blockData,temp.blockNumber);
  }
  return(0);
}

static int dfs_read(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi)
{
  printf("Inside read. Path is: %s buf is %s",path,buf);
        memset(tcp_buf,0,MAXLEN);
        sprintf(tcp_buf,"READ\n%s",path);

	(void)fi;
	char cacheFile[100];
	char *tempBuf;
	//	tempBuf=(char *)malloc(sizeof(char)*(int)size);
	FILE *fd;
	int cacheflag=0,exitflag=0;
	int ret=0,tempsize,res;
	int recvflag,retval=0;
	strcpy(cacheFile,".");
	strcat(cacheFile,path);
	strcat(cacheFile,".cache");
	fd=fopen(cacheFile,"ab+");
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
	    
	    
	    int nsize,noff;
	    noff=(int)offset/BLOCKSIZE;
	    nsize=(int)size/BLOCKSIZE+1;
	    tempBuf=(char *)malloc(sizeof(char)*(nsize*BLOCKSIZE));
	    strcpy(tempBuf,""); 
	     //printf("\n\nnoff %d offset %d\nnsize %d size %d",noff,(int)offset,nsize,(int)size);fflush(stdout);
	    int i;
	    for(i=1;i<nsize;i++)
	      {
	/*************************cached data*******************************/
		if(searchFile(noff+1,fd)==1)
		  {
		    printf("\n\nfound!!!!!\n\n");
		    
		    cacheflag=1;
		    memset(tcp_buf,0,MAXLEN);
		    
		    FILE *fp;
		    //  fp=fopen("abc.txt.cache","rb");
  
		    blocks temp;  
		    fseek(fd,0,SEEK_SET);
		    while(fread(&temp,sizeof(blocks),1,fd))
		      {
		      
		      if(temp.blockNumber==noff+1)
			{
			//			   printf("present in cache");
			  
			tempsize=strlen(temp.blockData);
 printf("%s\n\n<3<3<3<3<3%d:: %d",temp.blockData,(int)size,tempsize);fflush(stdout);
			//if(tempsize>=BLOCKSIZE)
			// tempsize=tempsize-1;
 /*	char *tempblock=(char *)malloc(sizeof(char)*tempsize);
			int j;
			memset(tempblock,0,tempsize);
			for(j=0;j<tempsize-1;j++)
			tempblock[j]=temp.blockData[j];*/
			strcat(tempBuf,temp.blockData);
			ret=ret+tempsize;
			retval=retval+tempsize;
			printf("This happens");fflush(stdout);
			//		free(tempblock);
			break;
		      }
		    }
		    if(tempsize<BLOCKSIZE)
		      {
			send(sock,"cache",strlen("cache"),0);
			printf("next\ni is %d and nsize is %d\n",i,nsize);
			exitflag=1;
			break;
		      }
		    if(i!=nsize)
		      {
			send(sock,"ok",strlen("ok"),0);
			printf("ok\ni is %d and nsize is%d\n",i,nsize);
		      }
		    else
		      {
			send(sock,"done",strlen("done"),0);
			printf("next\ni is %d and nsize is %d\n",i,nsize);
			exitflag=1;
			break;
		      }

		    fflush(stdout);

		  }

		/*********************************if uncached***************************************************/
		else
		  {
		    
		    memset(tcp_buf,0,MAXLEN);
		    // if(i==1)
		      sprintf(tcp_buf,"%d",(noff*BLOCKSIZE));
		      //		else
		      // sprintf(tcp_buf,"%d",(((int)offset) + (BLOCKSIZE * (i-1))));
		    send(sock,tcp_buf,strlen(tcp_buf),0);
		    
		    memset(tcp_buf,0,MAXLEN);
		    recvflag=recv(sock,tcp_buf,BLOCKSIZE,0);
		    if(recvflag<0)
		      {
			printf("Receiving error");
			exit(0);
		      }
		    
		    //if(i==nsize-1)
		    // tcp_buf[recvflag]='\0';
		    //strcat(
		    //tcp_buf[recvflag]='\0';
		
		    //tcp_buf[recvflag]='\0';
		
		    blocks temp;
		    //temp=(blocks *)malloc(sizeof(blocks));
		    temp.blockNumber=noff;

		    strcpy(temp.blockData,tcp_buf);
		    // temp->blockData[recvflag]='\0';
		    writeToFile(fd,&temp);
		    retval=retval+recvflag;
			strcat(tempBuf,tcp_buf);
		    //  printf("block :@:@:@:@ %s",temp->blockData);
		    if(strcmp(temp.blockData,"file_is_finished")==0 || recvflag<BLOCKSIZE){
		      //tcp_buf[recvflag]='\0';
		      //		      strcat(tempBuf,tcp_buf);
		      send(sock,"next",strlen("next"),0);
		      printf("next\ni is %d and nsize is %d\n",i,nsize);		  
			exitflag=1;
			printf("Break from here");fflush(stdout);
		      break;
		    }
		    // else
		    //  {

		    //  }
		    // free(temp);
		    // writeToFile(fd,temp);
		    //	printf("tcp_buf %s",tcp_buf);fflush(stdout);

		    memset(tcp_buf,0,MAXLEN);
		    if(i!=nsize)
		      {
			send(sock,"ok",strlen("ok"),0);
			printf("ok\ni is %d and nsize is%d\n",i,nsize);
		      }
		    else
		      {
			send(sock,"next",strlen("next"),0);
			printf("next\ni is %d and nsize is %d\n",i,nsize);
		      }
		    fflush(stdout);
		    //sleep(1);
		  }
		//		offset++;
		//		printf("off:%d",(int)offset);
		printf("hi pathav ki");fflush(stdout);
		recv(sock,tcp_buf,MAXLEN,0);
		noff++;
	      }
	  }
	else{
	  //printf("\n%s\n",tcp_buf);
	  printf("sending errno");fflush(stdout);
	    send(sock,tcp_buf,strlen(tcp_buf),0);
	    memset(tcp_buf,0,MAXLEN);
	    res=recv(sock,tcp_buf,MAXLEN,0);
	    if(res<0){
	      printf("\nError receiving flags");
	      exit(1);
	    }
	    tcp_buf[res]='\0';
	    ret=atoi(tcp_buf);
	    // send(sock,"next",strlen("next"),0);
	    return ret;

	//	printf("next\ni is %d and nsize is %d\n",i,nsize);
	}

	
	if(exitflag==1)
	  recv(sock,tcp_buf,MAXLEN,0);

	//printf("%shere5",buf);fflush(stdout);
	fclose(fd);
	printf("End of read\n");
	//	if(cacheflag==0){
	  //strcat(tempBuf,tcp_buf);
	//send(sock,"total",strlen("total"),0);
	//int rf=recv(sock,tcp_buf,MAXLEN,0);
	//tcp_buf[rf]='\0';
	//ret=atoi(tcp_buf);
	//}*/

	//	strcpy(buf,tempBuf);
	printf("print");fflush(stdout);
	memcpy(buf,tempBuf+((int)offset%BLOCKSIZE),(int)size);
	buf[(int)size]='\0';
	printf("\n\nbuf,%s",buf);fflush(stdout);
	free(tempBuf);
	memset(tcp_buf,0,MAXLEN);
	printf("return %d:: %d",strlen(buf),(int)size);
        return (int)size;
}


static int dfs_write(const char *path, const char *buf, size_t size,off_t offset, struct fuse_file_info *fi)
{
	printf("Inside write. Path is: %s buf is %s",path,buf);
        memset(tcp_buf,0,MAXLEN);
        sprintf(tcp_buf,"WRITE\n%s",path);

	char wrtFile[100];
	char *tempBuf;
	tempBuf=(char *)malloc(sizeof(char)*(int)size);
	FILE *fd;
	int recvflag;
	strcpy(wrtFile,".");
	strcat(wrtFile,path);
	strcat(wrtFile,".wrt");
	fd=fopen(wrtFile,"ab+");
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

	    memset(tcp_buf,0,MAXLEN);
	    sprintf(tcp_buf,"%d",(int)offset);
	    send(sock,tcp_buf,strlen(tcp_buf),0);
	    strcpy(tempBuf,"");
	    int nsize,noff;
	    noff=(int)offset/BLOCKSIZE;
	    nsize=(int)size/BLOCKSIZE;
	    recv(sock,tcp_buf,MAXLEN,0);
		printf("recvd\n%s\n",tcp_buf);
	    memset(tcp_buf,0,MAXLEN);
	    sprintf(tcp_buf,"%d",(int)size);
	    send(sock,tcp_buf,MAXLEN,0);
	    recv(sock,tcp_buf,MAXLEN,0);
	printf("recvd\n%s\n",tcp_buf);
	    memset(tcp_buf,0,MAXLEN);
	    // sprintf(tcp_buf,"%d",(int)size);
	    send(sock,buf,strlen(buf),0);
	    recv(sock,tcp_buf,MAXLEN,0);
	printf("recvd\n%s\n",tcp_buf);
	    memset(tcp_buf,0,MAXLEN);
	  }
	else
	  {
	    int ret,res;
	  printf("\n%s\n",tcp_buf);
	    send(sock,tcp_buf,strlen(tcp_buf),0);
	    memset(tcp_buf,0,MAXLEN);
	    res=recv(sock,tcp_buf,MAXLEN,0);
	    if(res<0){
	      printf("\nError receiving flags");
	      exit(1);
	    }
	    tcp_buf[res]='\0';
	    ret=atoi(tcp_buf);
	    // send(sock,"next",strlen("next"),0);
	    return ret;

	  }
	     
	fclose(fd);
	printf("End of write\n");
	send(sock,"total",strlen("total"),0);
	int rf=recv(sock,tcp_buf,MAXLEN,0);
	tcp_buf[rf]='\0';
	int ret=atoi(tcp_buf);
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
//.flush = (void*)dfs_flush, 
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
