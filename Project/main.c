#define UART5_BASE 0x40005000
#define UART4_BASE 0x40004C00
#define USART3_BASE 0x40004800
#define USART2_BASE 0x40004400
#define USART1_BASE 0x40013800

#define USART_SR_OFFSET 0x000
#define USART_DR_OFFSET 0x004
#define USART_BRR_OFFSET 0x008
#define USART_CR1_OFFSET 0x00C
#define USART_CR2_OFFSET 0x010
#define USART_CR3_OFFSET 0x014
#define USART_GTPR_OFFSET 0x018

#define GPIO_B_BASE 0x40010C00
#define GPIO_CRL_OFFSET 0x000
#define GPIO_CRH_OFFSET 0x004
#define GPIO_IDR_OFFSET 0x008
#define GPIO_ODR_OFFSET 0x00c

#define RCC_BASE 0x40021000
#define RCC_APB2RSTR_OFFSET 0x0c
#define RCC_APB1EN_OFFSET 0x1c

int main(void){
	// 使能GPIO时钟
	unsigned int rccApb2Addr = RCC_BASE + RCC_APB2RSTR_OFFSET;
	*((unsigned int*)rccApb2Addr) |= 1 << 3;
	
	// GPIO引脚设置
	unsigned int gpioHighAddr = GPIO_B_BASE + GPIO_CRH_OFFSET;
	
	// PB10 复用推挽输出
	*((unsigned int*)gpioHighAddr) &= ~((unsigned int)0xF << 8);
	*((unsigned int*)gpioHighAddr) |= ((unsigned int)0xB << 8);
	
	// PB11 浮空输入
	*((unsigned int*)gpioHighAddr) &= ~((unsigned int)0xF << 12);
	*((unsigned int*)gpioHighAddr) |= ((unsigned int)0x4 << 12);
	
	// 配置USART参数 ctrl1: 1000010101100 波特率寄存器0x013D 暂时假设时钟为36 
	unsigned int uart3Ctl1Addr =  USART3_BASE + USART_CR1_OFFSET;
	*((unsigned int*)uart3Ctl1Addr) |= (unsigned int)0x10AC;
	
	unsigned int uart3BRRAddr = USART3_BASE + USART_BRR_OFFSET;
	*((unsigned int*)uart3BRRAddr) |= (unsigned int)0x013D;
	
	// USART3使能
	unsigned int usart3EnAddr = RCC_BASE + RCC_APB1EN_OFFSET;
	*((unsigned int*)usart3EnAddr) |= 1<<18;
	return 0;
}
