#include "includs.h" 

uint8 sec_read,min_read,hour_read,dom_read,month_read;
uint8 sec_write,min_write,hour_write,dom_write,month_write;
uint16 year_read,year_write;

void rtc_init(void)
{
//判断是首次上电开机还是复位开机，如果是首次上电开机则对时间日期初始化
	if(ALSEC == 30 && ALMIN == 30 && ALHOUR == 10 && ALDOM == 15 && ALMON == 6 && ALYEAR == 1111)
	{
		PREINT = Fpclk / 32768 - 1;			// 设置基准时钟分频器
		PREFRAC = Fpclk - (Fpclk / 32768) * 32768;		
	}
	else
	{
		PREINT = Fpclk / 32768 - 1;			// 设置基准时钟分频器
		PREFRAC = Fpclk - (Fpclk / 32768) * 32768;	
	
		CCR = 0x00;//禁止时间计数器
		YEAR = 2011;	
		MONTH = 01;	
		DOM = 1;
		HOUR = 8;		
		MIN = 30;		
		SEC = 00;	
		
		ALSEC = 30;
		ALMIN = 30;
		ALHOUR = 10;
		ALDOM = 15;
		ALMON = 6;
		ALYEAR = 1111;
		
		AMR = 0xff;
		
		CCR = 0x01;//启动RTC
	}
} 

void read_rtc(void)
{
	sec_read = SEC;
	min_read = MIN;
	hour_read = HOUR;
	dom_read = DOM;
	month_read = MONTH;		
	year_read = YEAR;
				
}

void write_rtc(void)
{
	PREINT = Fpclk / 32768 - 1;			// 设置基准时钟分频器
	PREFRAC = Fpclk - (Fpclk / 32768) * 32768;
		
	CCR = 0x00;//禁止时间计数器
		
	YEAR = year_write;	
	MONTH = month_write;	
	DOM = dom_write;
	HOUR = hour_write;		
	MIN = min_write;		
	SEC = sec_write;	

	CCR = 0x01;//启动RTC
}