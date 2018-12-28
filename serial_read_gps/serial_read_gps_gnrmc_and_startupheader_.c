#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <stdint.h>



static int fd;
static int ret;


int uart_open(int fd,const char *pathname);
int uart_config(int fd,int baude,int c_flow, int bits, char parity, int stop);
int safe_read(int fd, char *vptr, size_t len);
int uart_read(int fd, char *r_buf, size_t lenth);//串口读取数据
int uart_close(int fd);


int uart_open(int fd, const char *pathname)
{
    assert(pathname);//检测串口路径是否存在
    fd = open(pathname, O_RDWR|O_NOCTTY|O_NDELAY);//以只读形式、不将此终端作为此进程的终端控制器、非阻塞的形式打开串口
    if(fd == -1)
    {
        perror("uart open failed!");
        return -1;
    }
    if(fcntl(fd,F_SETFL,0)<0)//设置串口非阻塞，因为这里是以非阻塞形式打开的，所以第三个参数为0，后面会详细介绍fcntl函数
    {
        perror("fcntl failed!");
        return -1;
    }
    return fd;
}

int uart_config(int fd,int baude,int c_flow, int bits, char parity, int stop)
{
    struct termios uart;
    
    if(tcgetattr(fd,&uart)!=0)
    {
        perror("tcgetattr failed!");
        return -1;
    }
    
	switch(baude)
	{
		case 4800:
		    cfsetispeed(&uart,B4800);//设置输入波特率
		    cfsetospeed(&uart,B4800);//设置输出波特率
		    break;
		case 9600:
		    cfsetispeed(&uart,B9600);
		    cfsetospeed(&uart,B9600);
		    break;
		case 19200:
		    cfsetispeed(&uart,B19200);
		    cfsetospeed(&uart,B19200);
		    break;
		case 38400:
		    cfsetispeed(&uart,B38400);
		    cfsetospeed(&uart,B38400);
		    break;
		default:
		    fprintf(stderr,"Unknown baude!");
		    return -1;
	}
	
	switch(c_flow)
	{
		case 'N':
		case 'n':
		  uart.c_cflag &= ~CRTSCTS;//不进行硬件流控制
		  break;
		case 'H':
		case 'h':
		  uart.c_cflag |= CRTSCTS;//进行硬件流控制
		  break;
		case 'S':
		case 's':
		  uart.c_cflag |= (IXON | IXOFF | IXANY);//进行软件流控制
		  break;
		default:
		  fprintf(stderr,"Unknown c_cflag");
		  return -1;
	}

    switch(bits)
    {
	    case 5:
	        uart.c_cflag &= ~CSIZE;//屏蔽其他标志位
	        uart.c_cflag |= CS5;//数据位为5位
	        break;
	    case 6:
	        uart.c_cflag &= ~CSIZE;
	        uart.c_cflag |= CS6;
	        break;
	    case 7:
	        uart.c_cflag &= ~CSIZE;
	        uart.c_cflag |= CS7;
	        break;
	    case 8:
	        uart.c_cflag &= ~CSIZE;
	        uart.c_cflag |= CS8;
	      break;
	    default:
	        fprintf(stderr,"Unknown bits!");
	        return -1;
    }
    
    switch(parity)
    {
	    case 'n':
	    case 'N':
	        uart.c_cflag &= ~PARENB;//PARENB：产生奇偶校验
	        //uart.c_cflag &= ~INPCK;	//INPCK：使奇偶校验起作用			
	        break;
	    case 's':
	    case 'S':
	        uart.c_cflag &= ~PARENB;
	        uart.c_cflag &= ~CSTOPB;	//使用两位停止位
	        break;
	    case 'o':
	    case 'O':
	        uart.c_cflag |= PARENB;
	        uart.c_cflag |= PARODD;		//使用奇校验
	        uart.c_cflag |= INPCK;
	        uart.c_cflag |= ISTRIP;//使字符串剥离第八个字符，即校验位
	        break;
	    case 'e':
	    case 'E':
	        uart.c_cflag |= PARENB;
	        uart.c_cflag &= ~PARODD;//非奇校验，即偶校验
	        uart.c_cflag |= INPCK;
	        uart.c_cflag |= ISTRIP;
	        break;
	    default:
	        fprintf(stderr,"Unknown parity!\n");
	        return -1;
    }
    
    switch(stop)
    {
	    case 1:
	        uart.c_cflag &= ~CSTOPB;//CSTOPB：使用两位停止位
	        break;
	    case 2:
	        uart.c_cflag |= CSTOPB;
	        break;
	    default:
	        fprintf(stderr,"Unknown stop!\n");
	        return -1;
    }
    uart.c_oflag &= ~OPOST;//OPOST:表示数据经过处理后输出
	uart.c_cflag |= CLOCAL;
	uart.c_cflag |= CREAD;
	

	if(tcsetattr(fd,TCSANOW,&uart)<0)//激活配置，失败返回-1
	{
		return -1;
	}
    uart.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG );//使串口工作在原始模式下
    uart.c_cc[VTIME] = 0;//设置等待时间为0
    uart.c_cc[VMIN] = 1;//设置最小接受字符为1
    tcflush(fd,TCIFLUSH);//清空输入缓冲区
    if(tcsetattr(fd,TCSANOW,&uart)<0)//激活配置
    {
        perror("tcgetattr failed!");
        return -1;
    }
    return 0;
}


int safe_read(int fd, char *vptr, size_t len)
{
    size_t left;
    left = len;
    ssize_t nread;
    char *ptr;
    ptr = vptr;
    while(left > 0)
    {
        if ((nread = read(fd, ptr, left)) < 0)
        {
            if (errno == EINTR)
            {
                nread = 0;
            }
            else if(nread == 0)
            {
                break;
            }
        }
        left -= nread;//read成功后，剩余要读取的字节自减
        ptr += nread;//指针向后移，避免后读到的字符覆盖先读到的字符
    }
    return (len - left);
}






int uart_read(int fd, char *r_buf, size_t lenth)
{
    fd_set rfds;
    struct timeval time;
    ssize_t cnt = 0;
    /*将读文件描述符加入描述符集合*/
    FD_ZERO(&rfds);
    FD_SET(fd,&rfds);
    /*设置超时为15s*/
    time.tv_sec = 15;
    time.tv_usec = 0;
    /*实现多路IO*/
    ret = select(fd+1, &rfds ,NULL, NULL, &time);
    switch (ret) {
    case -1:
        fprintf(stderr,"select error!\n");
        break;
    case 0:
        fprintf(stderr, "time over!\n");
        break;
    default:
        cnt = safe_read(fd, r_buf, lenth);
        if(cnt == -1)
        {
            fprintf(stderr, "safe read failed!\n");
            return -1;
        }
        return cnt;
    }
}



int uart_close(int fd)
{
    assert(fd);//assert先检查文件描述符是否存在
    close(fd);
    return 0;
}

void save_log(uint8_t *filename, uint8_t *strline)
{
	if(filename != NULL)
	{
		FILE *fp = NULL;
		fp=fopen(filename,"a");
		fwrite(strline, 1, strlen(strline), fp);
		fflush(fp);
		int fd = fileno(fp);
		fsync(fd);
		fclose(fp);
	}
}


#define YES				1
#define NO				0
#define ONCE_READ_SIZE	1024
#define KEY_LEN			20
int main(int args, char **argv)
{	

	uint8_t save_buf[ONCE_READ_SIZE+1];
	uint16_t save_index = 0;

	uint8_t devname[20] = {0};

	uint8_t continue_analysis_flag = NO;
	uint8_t last_count = 0;
	uint8_t analysis_key[KEY_LEN] = "$GNRMC";
	uint8_t analysis_key_len = strlen(analysis_key);

	uint8_t analysis_key_2[KEY_LEN] = "UM220";
	uint8_t analysis_key_len_2 = strlen(analysis_key_2);

	uint8_t analysis_key_3[KEY_LEN] = "FWVer";
	uint8_t analysis_key_len_3 = strlen(analysis_key_3);

	uint16_t analysis_data_index = 0;
	
	strcpy(devname, argv[1]);
	
	uint8_t r_buf[ONCE_READ_SIZE+1];
	bzero(r_buf,ONCE_READ_SIZE+1);
	
	fd = uart_open(fd, devname);//选择的是ttsY1串口
	if(fd == -1)
	{
		fprintf(stderr,"open failed!\n");
		exit(EXIT_FAILURE);
	}
	
	if(uart_config(fd, 9600, 'N', 8, 'N', 1) == -1)
	{
		fprintf(stderr,"configure failed!\n");
		exit(EXIT_FAILURE);
	}

	while(1){
		ret = uart_read(fd, r_buf+last_count, ONCE_READ_SIZE-last_count);
		ret += last_count;
		if(ret == -1)
		{
             fprintf(stderr, "uart_read failed!\n");
             exit(EXIT_FAILURE);
		}	
		
#if 1	
		//display specfic data from serial
		analysis_data_index = 0;
		if(continue_analysis_flag == YES)
		{
			continue_analysis_flag = NO;
			
			save_index = 0;
			memset(save_buf, 0, sizeof(save_buf));
			
			while(r_buf[analysis_data_index] != '\n')
			{
				printf("%c", r_buf[analysis_data_index]);
				
				save_buf[save_index++] = r_buf[analysis_data_index];
				
				analysis_data_index++;
			}
			printf("%c", r_buf[analysis_data_index]);

			save_buf[save_index++] = r_buf[analysis_data_index];

			save_log(argv[2], save_buf);
				
			analysis_data_index++;
		}
		
		while(analysis_data_index<ret)
		{	
			if(r_buf[analysis_data_index] == analysis_key[0])
			{
				if((ret-analysis_data_index) > strlen(analysis_key))
				{
					if(strncmp(r_buf+analysis_data_index, analysis_key, strlen(analysis_key)) == 0)
					{	
						save_index = 0;
						memset(save_buf, 0, sizeof(save_buf));
						
						while(r_buf[analysis_data_index] != '\n')
						{
							printf("%c", r_buf[analysis_data_index]);

							save_buf[save_index++] = r_buf[analysis_data_index];
	
							analysis_data_index++;
							if(analysis_data_index>=ret)
							{
								continue_analysis_flag = YES;
								last_count = 0;
								break;
							}
						}
						
						printf("%c", r_buf[analysis_data_index]);
						
						save_buf[save_index++] = r_buf[analysis_data_index];

						save_log(argv[2], save_buf);
						
						analysis_data_index++;
					}
					else
					{
						analysis_data_index++;
					}
				}
				else
				{
					last_count = ret-analysis_data_index;
					memcpy(r_buf, r_buf+analysis_data_index, last_count);		
					break;
				}
				
			}

			if(r_buf[analysis_data_index] == analysis_key_2[0])
			{
				if((ret-analysis_data_index) > strlen(analysis_key_2))
				{
					if(strncmp(r_buf+analysis_data_index, analysis_key_2, strlen(analysis_key_2)) == 0)
					{
						save_index = 0;
						memset(save_buf, 0, sizeof(save_buf));
						
						while(r_buf[analysis_data_index] != '\n')
						{
							printf("%c", r_buf[analysis_data_index]);

							save_buf[save_index++] = r_buf[analysis_data_index];

							analysis_data_index++;
							if(analysis_data_index>=ret)
							{
								continue_analysis_flag = YES;
								last_count = 0;
								break;
							}
						}
						printf("%c", r_buf[analysis_data_index]);

						save_buf[save_index++] = r_buf[analysis_data_index];

						save_log(argv[2], save_buf);

						
						analysis_data_index++;
					}
					else
					{
						analysis_data_index++;
					}
				}
				else
				{
					last_count = ret-analysis_data_index;
					memcpy(r_buf, r_buf+analysis_data_index, last_count);		
					break;
				}
				
			}



			if(r_buf[analysis_data_index] == analysis_key_3[0])
			{
				if((ret-analysis_data_index) > strlen(analysis_key_3))
				{
					if(strncmp(r_buf+analysis_data_index, analysis_key_3, strlen(analysis_key_3)) == 0)
					{
						save_index = 0;
						memset(save_buf, 0, sizeof(save_buf));
						
						while(r_buf[analysis_data_index] != '\n')
						{
							printf("%c", r_buf[analysis_data_index]);

							save_buf[save_index++] = r_buf[analysis_data_index];

							analysis_data_index++;
							if(analysis_data_index>=ret)
							{
								continue_analysis_flag = YES;
								last_count = 0;
								break;
							}
						}
						printf("%c", r_buf[analysis_data_index]);

						save_buf[save_index++] = r_buf[analysis_data_index];

						save_log(argv[2], save_buf);

						
						analysis_data_index++;
					}
					else
					{
						analysis_data_index++;
					}
				}
				else
				{
					last_count = ret-analysis_data_index;
					memcpy(r_buf, r_buf+analysis_data_index, last_count);		
					break;
				}
				
			}
			
			else
			{
				analysis_data_index++;
			}

			if(continue_analysis_flag == YES)
			{
				break;
			}
								
			
		}
#else
		//display all data from serial
		printf("%s", r_buf);
#endif	
	}


    ret = close(fd);
	if(ret == -1)
	{
	 fprintf(stderr, "close failed!\n");
	 exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
 }
