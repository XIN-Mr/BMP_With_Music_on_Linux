#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include <stdlib.h>

int *lcd = NULL;
int showbmp(const char *pic);	

void *routine(void *arg)
{	
	//pthread_detach(pthread_self());
	// 以下函数会创建一个子进程，并且让他去执行指定的命令
	system("./madplay   1.mp3 ");
}

int main()
{
	
	//启动一条独立的线程，去执行指定的函数	
	pthread_t  tid;
	pthread_create(&tid, NULL, routine, NULL);
	
	
	//打开LCD设备
	int  fd = open("/dev/fb0", O_RDWR);	
	if(fd < 0)
	{
		printf("open lcd fail\n");
	}
	
	//对LCD设备进行内存映射操作
	lcd = mmap(NULL, 800*480*4, PROT_READ|PROT_WRITE,MAP_SHARED, fd, 0);
	
	int i = 0;
	char buf[] = "0.bmp";
	char num[10] = {'0','1','2','3','4','5','6','7','8','9'};
	
	while(1)
	{
		for(i=0; i<11; i++)	//循环显示11张bmp图片
		{
			buf[0] = num[i];
			printf("%s\n", buf);
			showbmp(buf);
			sleep(5);	//5s延迟
		}
	}
	
	close(fd);
	munmap(lcd, 800*480*4);
}

//显示24位 bmp图片
int showbmp(const char *pic)
{
	//printf("pic=%s\n",pic);
	//打开图片文件 
	int bmpfd = open(pic, O_RDWR);
	if(bmpfd < 0)
	{
		perror("open pictures failed");
		return -1;
	}
	
	//读取图片的头信息 
	unsigned char head[54]={0};
	read(bmpfd, head, 54);
	
	//解析宽高  
	int  kuan = *((int *)&head[18]);
	int  gao  = *((int *)&head[22]);

	//变长数组 
	int buf[gao][kuan];//32位图的buf
	char tmp_buf[gao*kuan*3]; //24位图的buf
	
	//读取文件的数据到 buf中  
	read(bmpfd, tmp_buf, sizeof(tmp_buf));
	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;
	unsigned char a = 0;
	unsigned char *p = tmp_buf;
	unsigned int color = 0;
	unsigned int x = 0,y = 0;
	
	int *tmplcd = lcd;
	
	for(y = 0; y < gao; y++)
	{
		//跳行
		tmplcd = lcd;
		int tmpy = gao - 1 - y;
		if( tmpy < 480)
		{
			tmplcd = tmplcd + tmpy * 800;
		}
		
		for(x = 0; x < kuan; x++)  //画 X 轴 
		{
			b = *p++;
			g = *p++;
			r = *p++;
			a = 0;
			color  = a << 24 | r << 16 | g << 8 | b;
			
			//把图片的像素点赋值到lcd设备中
			//边界判断		
			if( x < 800) //越界了
			{
				*(tmplcd + x) = color;  //显示颜色值
			}	
		}		
	}
	
	//关闭所有设备
	close(bmpfd);	
}	