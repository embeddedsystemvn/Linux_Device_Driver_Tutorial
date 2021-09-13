#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

static char receive[255];    

int main(){
   int ret, fd;
   char send_data[255];

   fd = open("/dev/mydevice", O_RDWR);             
   if (fd < 0){
      perror("Failed to open the device...");
      return errno;
   }

   printf("Enter data to send to kernel:	");
   scanf("%[^\n]%*c", send_data);                

   ret = write(fd, send_data, strlen(send_data)); 
   if (ret < 0){
      perror("Failed to write the message to the device.");
      return errno;
   } printf("write return %d \n",ret);

   printf("\nPress ENTER to read back from the kernel...\n");
   getchar();

   ret = read(fd, receive, 255);
   if (ret < 0){
      perror("Failed to read the message from the device.");
      return errno;
   }printf("read return %d\n",ret);
   printf("Received data from kernel:	[%s]\n", receive);

   return 0;
}
