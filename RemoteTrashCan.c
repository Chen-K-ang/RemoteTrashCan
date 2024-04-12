#include<reg52.h>
#include<intrins.h>
#define uint  unsigned int
#define uchar unsigned char
#define NOP() {_nop_();_nop_();_nop_();_nop_();}
//------LCD����-----
sbit lcd_rs=P0^6;

sbit lcd_en=P0^7;
//------����������-------
sbit Tx=P3^3;		//���������ź�����
sbit Rx=P3^2;		//�����ź����
sbit IN1=P3^5;//�綯����������1
sbit IN2=P3^6;//�綯����������2
sbit ENA=P3^7;//�綯��ʹ������
sbit beef=P3^4;
sbit LED=P1^7;
sbit key1=P1^0;
sbit key2=P1^1;
sbit lj1=P1^4;
sbit lj2=P1^5;
sbit lj3=P1^6;
uchar code table[]={"stop         "};		//LCD��һ����ʾ
uchar code table2[]={"forword      "};
uchar code table3[]={"reverse      "};
uchar temp_dis[]= {"000.0 cm  00%"};				//LCD�ڶ���
uchar code hello_world[]="Hello World ";
uchar code hello_world1[]="You can go to position 1 ";
uchar code hello_world2[]="You can go to position 2 ";
uchar code hello_world3[]="You can go to position 3 ";
uchar Dat_rxd;
long int t,                 //��ʱ�����㳬���������źŵļ������������ײ���ʱ���ж�
	distance=500,       //��ʼ�������룬�����500���Ǹ���ֵ����ϵ����
        sy,                 //�õ�100-distance��ֵ���Ǹ���ֵ�����������͹�
        time=0;             //������־λ����ͬ��ֵ���Դ���������Ǹ�ָʾ
uchar cache[4]={0,0,0,0};   //���泬�����������λ��
//--------��ʱ-------
void delay(uint ms)
{
	uint t;
	while(ms--)
		for(t=0;t<30;t++);
}
void write_com(uchar com);//init_lcd()������Ҫ�õ�����������ǰ����
//******************���ڳ�ʼ������****************************//
void Com0_Init(void)
{
	EA=1;        //�����ж�
	ES=1;        //�������ж�
	TMOD=0x20;   //��ʱ��T1���ڷ�ʽ3�жϲ���������
	SCON=0x50;   
	TH1=0xfd;    //����������Ϊ9600(����12M)
	TL1=0xfd;
	PCON=0x00;
	TR1=1;       //����ʱ��T1���п���λ
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
//************LCD1602��ʼ������************************
void init_lcd()
{
	write_com(0x38);	   //����16*2��ʾ��5*7����8λ���ݽӿ�
	write_com(0x08);	   //дһ���ַ����ַָ���1 
	write_com(0x01);	   //��ʾ���㣬����ָ������
	write_com(0x06);	   //дһ���ַ����ַָ���1
	write_com(0x0c);	   //����ʾ����겻��ʾ����˸
}


//************LCD1602дָ���************************
void write_com(uchar com)   //lcd1602дָ�rs�͵�ƽ��rw�͵�ƽ��en�Ӹߵ�ƽ��Ϊ�͵�ƽ
{
	delay(5);
	lcd_rs = 0;			   // ѡ��дָ��
	lcd_en = 0;			   // ��ʹ�ܶ���0
	P2 = com;			   // ��Ҫд�������͵�����������
	delay(5);			   // ��ʱ5ms���������ȶ�
	lcd_en = 1;			   // ���ڳ�ʼ���ѽ�lcd_en��Ϊ0��ʹ�ܶ˸�һ�������壬
	delay(5);			   // ��ʱ5ms���������ȶ�
	lcd_en = 0;			   // ��ʹ�ܶ���0���������
}


//************LCD1602д���ݺ���************************
void write_data(uchar date)//lcd1602д���ݣ�rs�ߵ�ƽ��rw�͵�ƽ��en�Ӹߵ�ƽ��͵�ƽ
{
	delay(5);
	lcd_rs = 1;			   // ѡ��д����
	lcd_en = 0;			   // ��ʹ�ܶ���0
	P2 = date;			   // ��Ҫд�������͵�����������
	delay(5);			   // ��ʱ5ms���������ȶ�
	lcd_en = 1;			   // ���ڳ�ʼ���ѽ�lcd_en��Ϊ0��ʹ�ܶ˸�һ�������壬
	delay(5);			   // ��ʱ5ms���������ȶ�
	lcd_en = 0;			   // ��ʹ�ܶ���0���������
}

//************LCD1602��ʾ����******************
void lcd1602_display(uchar *q,uchar *p)//*ͨ��ָ��p��q�ֱ�ָ����������ĵ�һ������Ȼ���������ɨ����ʾ��һ������ʾ�����ַ�
{
	write_com(0x80);	     // �ֽ�����ָ�붨λ����һ�е�һ���ִ�
	while(*q!='\0')
	{
		write_data(*q);
		q++;
		delay(1);	
	}
	write_com(0xc0);	   // �ֽ�����ָ�붨λ���ڶ��е�һ���ִ�
	while(*p!='\0')
	{
		write_data(*p);
		p++;
		delay(1);	
	}
}

//************LCD1602��ʾ��ӭ����******************
//void welcome()        //ɨ����ʾѧ�ż�������������ʱ�����
//{	
//	init_lcd();
//	lcd1602_display(xuehao,xingming);
//	delay(200);	  

//}
void HC05_Init()                          //������ģ���ʼ������
{
	Tx=1;						//��������
	NOP();NOP();NOP();NOP();
	Tx=0;
	distance=0.1872*t;			//�������
}
void distance_convert(long int dat)             //������㺯��
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
//------------������-----------
void main()
{
	Com0_Init();    //���ڳ�ʼ��   
	init_lcd();     //lcd��ʼ��
	delay(5);
	/*��ʱ����ʼ��*/
	TMOD|=0x09;     //��ʱ��T0������ʽ1  GATE0=1
	EA=1;	        //�����ж�
	TR0=1;		//������ʱ��
	EX0=1;		//���ⲿ�ж�
	IT0=1;		//����Ϊ�½����жϷ�ʽ
	beef=0;
	while(1)
	{
		HC05_Init();   //loraͨ��ģ��
		distance_convert(distance);  //��ȡ����
		lcd1602_display(temp_dis,table);  //��ʾ����
		if(key1==0)
		{
			while(key1==0);
			lcd1602_display(temp_dis,table2); 	 
			IN1=1;//�綯�����У����䳵�Ŵ�
			IN2=0;
			ENA=1;
			delay(6000);	  
			IN1=0;//�綯����ֹͣ
			IN2=0;
			ENA=0;	
		}
		
		if(key2==0)
		{
			while(key2==0);
			lcd1602_display(temp_dis,table3);  
			IN1=0;//�綯�����У����䳵�Ŵ�
			IN2=1;
			ENA=1;
			delay(6000);	  
			IN1=0;//�綯����ֹͣ
			IN2=0;
			ENA=0;		 

		}
		
		time=time+1;

		/*ģ������*/
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
//�ⲿ�ж�0
void int0() interrupt 0
{
	t=(TH0*256+TL0);	//����ߵ�ƽ������ʱ��,�����ص���ʱ��ʼ��ʱ���½��ص��������ⲿ�жϣ��رռ�ʱ����ֹͣ��ʱ
	TH0=0;
	TL0=0;
}