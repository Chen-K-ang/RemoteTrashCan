#include<reg52.h>
#include<intrins.h>
#define uint  unsigned int
#define uchar unsigned char
#define NOP() {_nop_();_nop_();_nop_();_nop_();}
//------LCD引脚-----
sbit lcd_rs=P0^6;

sbit lcd_en=P0^7;
//------超声波引脚-------
sbit Tx=P3^3;		//触发控制信号输入
sbit Rx=P3^2;		//回响信号输出
sbit IN1=P3^5;//电动机方向引脚1
sbit IN2=P3^6;//电动机方向引脚2
sbit ENA=P3^7;//电动机使能引脚
sbit beef=P3^4;
sbit LED=P1^7;
sbit key1=P1^0;
sbit key2=P1^1;
sbit lj1=P1^4;
sbit lj2=P1^5;
sbit lj3=P1^6;
uchar code table[]={"stop         "};		//LCD第一行显示
uchar code table2[]={"forword      "};
uchar code table3[]={"reverse      "};
uchar temp_dis[]= {"000.0 cm  00%"};				//LCD第二行
uchar code hello_world[]="Hello World ";
uchar code hello_world1[]="You can go to position 1 ";
uchar code hello_world2[]="You can go to position 2 ";
uchar code hello_world3[]="You can go to position 3 ";
uchar Dat_rxd;
long int t,                 //定时器计算超声波接收信号的计数参数，见底部定时器中断
	distance=500,       //初始化检测距离，这个“500”是个初值，关系不大
        sy,                 //得到100-distance的值，是个反值，这个给你解释过
        time=0;             //计数标志位，不同的值可以触发紫外的那个指示
uchar cache[4]={0,0,0,0};   //储存超声波距离各个位数
//--------延时-------
void delay(uint ms)
{
	uint t;
	while(ms--)
		for(t=0;t<30;t++);
}
void write_com(uchar com);//init_lcd()函数需要用到，所以先提前声明
//******************串口初始化函数****************************//
void Com0_Init(void)
{
	EA=1;        //开总中断
	ES=1;        //允许串口中断
	TMOD=0x20;   //定时器T1，在方式3中断产生波特率
	SCON=0x50;   
	TH1=0xfd;    //波特率设置为9600(晶振12M)
	TL1=0xfd;
	PCON=0x00;
	TR1=1;       //开定时器T1运行控制位
}

void send_string(uchar *p)
{
	while(*p!= '\0')
	{
		SBUF=*p;
		p++;
		while(TI==0);
		TI=0;
	}
}
//************LCD1602初始化函数************************
void init_lcd()
{
	write_com(0x38);	   //设置16*2显示，5*7点阵，8位数据接口
	write_com(0x08);	   //写一个字符后地址指针加1 
	write_com(0x01);	   //显示清零，数据指针清零
	write_com(0x06);	   //写一个字符后地址指针加1
	write_com(0x0c);	   //关显示，光标不显示不闪烁
}


//************LCD1602写指令函数************************
void write_com(uchar com)   //lcd1602写指令：rs低电平，rw低电平，en从高电平变为低电平
{
	delay(5);
	lcd_rs = 0;			   // 选择写指令
	lcd_en = 0;			   // 将使能端置0
	P2 = com;			   // 将要写的命令送到数据总线上
	delay(5);			   // 延时5ms，待数据稳定
	lcd_en = 1;			   // 由于初始化已将lcd_en置为0，使能端给一个高脉冲，
	delay(5);			   // 延时5ms，待数据稳定
	lcd_en = 0;			   // 将使能端置0以完成脉冲
}


//************LCD1602写数据函数************************
void write_data(uchar date)//lcd1602写数据：rs高电平，rw低电平，en从高电平变低电平
{
	delay(5);
	lcd_rs = 1;			   // 选择写数据
	lcd_en = 0;			   // 将使能端置0
	P2 = date;			   // 将要写的数据送到数据总线上
	delay(5);			   // 延时5ms，待数据稳定
	lcd_en = 1;			   // 由于初始化已将lcd_en置为0，使能端给一个高脉冲，
	delay(5);			   // 延时5ms，待数据稳定
	lcd_en = 0;			   // 将使能端置0以完成脉冲
}

//************LCD1602显示函数******************
void lcd1602_display(uchar *q,uchar *p)//*通过指针p和q分别指向两个数组的第一个数，然后进行数组扫描显示，一次性显示两行字符
{
	write_com(0x80);	     // 现将数据指针定位到第一行第一个字处
	while(*q!='\0')
	{
		write_data(*q);
		q++;
		delay(1);	
	}
	write_com(0xc0);	   // 现将数据指针定位到第二行第一个字处
	while(*p!='\0')
	{
		write_data(*p);
		p++;
		delay(1);	
	}
}

//************LCD1602显示欢迎函数******************
//void welcome()        //扫描显示学号加姓名，几秒延时后结束
//{	
//	init_lcd();
//	lcd1602_display(xuehao,xingming);
//	delay(200);	  

//}
void HC05_Init()                          //超声波模块初始化函数
{
	Tx=1;						//触发脉冲
	NOP();NOP();NOP();NOP();
	Tx=0;
	distance=0.1872*t;			//距离计算
}
void distance_convert(long int dat)             //距离计算函数
{
	cache[0]=dat/1000;
	cache[1]=dat/100%10;
	cache[2]=dat/10%10;
	cache[3]=dat%10;
	temp_dis[0]=cache[0]+'0';
	temp_dis[1]=cache[1]+'0';
	temp_dis[2]=cache[2]+'0';
	temp_dis[4]=cache[3]+'0';
	sy=100-(dat/10);
	temp_dis[10]=sy/10+'0';
	temp_dis[11]=sy%10+'0';
	if(sy>80)
	{
		beef=1;
		if(lj1==1)
		{
			EA=0;	
			send_string(hello_world1);
			EA=1;	
		}
		if(lj2==1)
		{
			EA=0;	
			send_string(hello_world2);
			EA=1;	
		}
		if(lj3==1)
		{
			EA=0;	
			send_string(hello_world3);
			EA=1;	
		}

	}
	else
		beef=0;
}
//------------主程序-----------
void main()
{
	Com0_Init();    //串口初始化   
	init_lcd();     //lcd初始化
	delay(5);
	/*定时器初始化*/
	TMOD|=0x09;     //定时器T0工作方式1  GATE0=1
	EA=1;	        //开总中断
	TR0=1;		//启动定时器
	EX0=1;		//开外部中断
	IT0=1;		//设置为下降沿中断方式
	beef=0;
	while(1)
	{
		HC05_Init();   //lora通信模块
		distance_convert(distance);  //读取距离
		lcd1602_display(temp_dis,table);  //显示距离
		if(key1==0)
		{
			while(key1==0);
			lcd1602_display(temp_dis,table2); 	 
			IN1=1;//电动机运行，后备箱车门打开
			IN2=0;
			ENA=1;
			delay(6000);	  
			IN1=0;//电动机运停止
			IN2=0;
			ENA=0;	
		}
		
		if(key2==0)
		{
			while(key2==0);
			lcd1602_display(temp_dis,table3);  
			IN1=0;//电动机运行，后备箱车门打开
			IN2=1;
			ENA=1;
			delay(6000);	  
			IN1=0;//电动机运停止
			IN2=0;
			ENA=0;		 

		}
		
		time=time+1;

		/*模拟紫外*/
		if(time==50)
		{
			LED=0;
		}
		if(time==60)
		{
			LED=1;
			time=0;
		}
	}
}
//外部中断0
void int0() interrupt 0
{
	t=(TH0*256+TL0);	//计算高电平持续的时间,上升沿到来时候开始计时，下降沿到来进入外部中断，关闭计时器，停止计时
	TH0=0;
	TL0=0;
}