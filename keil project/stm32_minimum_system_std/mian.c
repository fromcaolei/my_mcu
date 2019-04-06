#include <stm32f10x.h>

#define LED_OFF GPIO_SetBits(GPIOB, GPIO_Pin_14)  //端口置1
#define LED_ON GPIO_ResetBits(GPIOB, GPIO_Pin_14)  //端口置0

//==========================使用外部RC晶振========================================
void RCC_Configuration(void) 
{
  RCC_DeInit();  //初始化为缺省状态
  RCC_HSICmd(ENABLE);  //内部8M时钟使能
  //RCC_HSEConfig(RCC_HSE_ON);  //打开外部高速晶振
  while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);  //等待高速时钟使能就绪

  FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);  //开启FLASH预读缓冲功能，加速FLASH的读取。所有程序中必须的用法.位置：RCC初始化子函数里面，时钟起振之后
  //SYSCLK 周期与闪存访问时间的比例设置：
  //0：0 < SYSCLK <= 24M
  //1：24< SYSCLK <= 48M
  //2：48< SYSCLK <= 72M
  FLASH_SetLatency(FLASH_Latency_2);  //flash操作的延时

  RCC_HCLKConfig(RCC_SYSCLK_Div1);  //配置AHB(HCLK)时钟等于==SYSCLK
  RCC_PCLK2Config(RCC_HCLK_Div1);  //配置APB2(PCLK2)钟==AHB时钟
  RCC_PCLK1Config(RCC_HCLK_Div2);  //配置APB1(PCLK1)钟==AHB1/2时钟
  RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_9);  //配置PLL时钟 == 内部8M/2时钟 * 9 = 36MHz
  RCC_PLLCmd(ENABLE);  //使能PLL时钟
  while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);  //等待PLL时钟就绪

  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);  //配置系统时钟 = PLL时钟
  while(RCC_GetSYSCLKSource() != 0x08);  //检查PLL时钟是否作为系统时钟
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);   //使能GPIOB的时钟
}

//==========================延时函数========================================
void delay(u32 t)
{
  u16 i;
  while(t--)
    for(i=0;i<1000;i++);
}
	

//==========================按键GPIO初始化========================================
void Key_GPIO_Config(void)
{
  GPIO_InitTypeDef key_struct;  //定义结构体
  key_struct.GPIO_Pin = GPIO_Pin_15;  //结构体成员设置；对于按键初始化而言，由于GPIO是输入模式，故不必设置最大输出速度
  key_struct.GPIO_Mode = GPIO_Mode_IPU;  //上拉输入模式
  GPIO_Init(GPIOB, &key_struct);  //初始化外设GPIOx寄存器
}

//==========================灯GPIO初始化========================================
void Led_GPIO_Config(void)
{
  GPIO_InitTypeDef led_struct;  //定义结构体变量
  led_struct.GPIO_Pin = GPIO_Pin_14;  //指定引脚14
  led_struct.GPIO_Speed = GPIO_Speed_2MHz;  //设置输出速率
  led_struct.GPIO_Mode = GPIO_Mode_Out_PP;  //推挽输出模式
  GPIO_Init(GPIOB, &led_struct);  //初始化外设GPIOx寄存器
}

int main(void)
{
  //分别以标准库方式、抽象层方式点亮一个灯
  RCC_Configuration();  //配置系统时钟
  Key_GPIO_Config();
  Led_GPIO_Config();
  LED_OFF;

  while(1)
  {
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15) == 0)
    {
      delay(50);  //延迟去抖
      if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15) == 0)
        LED_ON;
    }
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15) != 0)
    {
      delay(50);  //松手检测
      if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15) != 0)
        LED_OFF;
    }
  }
}
