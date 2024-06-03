# 基于项目的STM32学习与实践
# 总述

本文档及相关Github将会记录我个人是如何学习和实现STM32相关功能的。在这个文档中，我将会使用STM32实现一个基础的项目，并在实现过程中一步步介绍必要的概念，从而帮助我们更好的掌握这些知识。

**说明**：大部分教程的视角要么是工具书，即仿照芯片手册一类堆叠知识，新手看的话很容易没有重点，搞不清楚哪些要理解并记忆。要么是实战课程跳过了大量入门者可能产生困惑的问题，而直接贴上大段代码。由于我本人也是新手，所以我将会从我的视角出发一步步揭示我遇到的问题并且说明我是怎么解决的。

# 为自己设计一个主题

我自己首先购买了野火的STM32F103开发板，我额外配了遥控器和显示屏。为了学习STM32的各个方面，所以我打算实现一个能与手机通信的功能，大体流程如下：

1. 手机上打开一个TXT文件并输入一些文字信息
2. 将文件发送到开发板上
3. 开发板收到文件后自动显示
4. 使用遥控器进行文档翻页
5. 使用遥控器可关闭显示器

这些功能包含了STM32不少的方面，虽然现在我们对于这些功能该如何操作还一无所知，但我们将会一步步的熟悉了解这个过程。对于读者而言，如何能自己设计一个类似的其他主题当然更好，举个例子：比方说按下某个按键后STM32向手机发送信息等，这样能帮助读者切实的去思考问题。

# Hello World

在开始之前，让我们先从第一个程序开始建立起对于嵌入式开发的认知，这部分内容我不愿多做讨论，大家应该查阅你所持有的开发板的教程，完成第一个最简单程序的编译、下载、运行。

这一部分应该不需要花太多的时间，请确认如下几点就ok.

- KEIL芯片库可以正常加载
- 程序可以正确编译
- 开发板正确连线与程序的正确下载
- 程序在板上正常运行

这也就是后续任何开发步骤的最最基本的流程了。

# 从一段代码开始

```c
/*本文件用于添加寄存器地址及结构体定义*/

/*片上外设基地址  */
#define PERIPH_BASE           ((unsigned int)0x40000000)

/*APB2 总线基地址 */
#define APB2PERIPH_BASE       (PERIPH_BASE + 0x10000)
/* AHB总线基地址 */
#define AHBPERIPH_BASE        (PERIPH_BASE + 0x20000)

/*GPIOB外设基地址*/
#define GPIOB_BASE            (APB2PERIPH_BASE + 0x0C00)

/* GPIOB寄存器地址,强制转换成指针 */
#define GPIOB_CRL			*(unsigned int*)(GPIOB_BASE+0x00)
#define GPIOB_CRH			*(unsigned int*)(GPIOB_BASE+0x04)
#define GPIOB_IDR			*(unsigned int*)(GPIOB_BASE+0x08)
#define GPIOB_ODR			*(unsigned int*)(GPIOB_BASE+0x0C)
#define GPIOB_BSRR	  *(unsigned int*)(GPIOB_BASE+0x10)
#define GPIOB_BRR			*(unsigned int*)(GPIOB_BASE+0x14)
#define GPIOB_LCKR		*(unsigned int*)(GPIOB_BASE+0x18)

/*RCC外设基地址*/
#define RCC_BASE      (AHBPERIPH_BASE + 0x1000)
/*RCC的AHB1时钟使能寄存器地址,强制转换成指针*/
#define RCC_APB2ENR		 *(unsigned int*)(RCC_BASE+0x18)

/*
	*工程模板(寄存器版本)
  */
#include "stm32f10x.h" 


/**
  *   主函数
  */
int main(void)
{	
	// 开启GPIOB 端口时钟
	RCC_APB2ENR |= (1<<3);

	//清空控制PB0的端口位
	GPIOB_CRL &= ~( 0x0F<< (4*0));	
	// 配置PB0为通用推挽输出，速度为10M
	GPIOB_CRL |= (1<<4*0);

	// PB0 输出 低电平
	GPIOB_ODR &= ~(1<<0);
	
	while(1);
}



// 函数为空，目的是为了骗过编译器不报错
void SystemInit(void)
{	
}
```

上面是一小段关于GPIO控制的最基本的代码，我们将简要说明几个问题：

## 代码是怎么在板上跑起来的

如果你看过KEIL工程里的代码，你会发现除了这个涉及GPIO的C文件外，还有一个文件叫startup_stm32f10x_hd.s。这是由汇编语言写成的文件，是由芯片厂商提供的。

我们知道，在C语言中，编译流程包含预处理-编译-汇编-链接。对于KEIL而言，这些操作对用户是不可见的。但简单来说，实际在我们点击build之后，KEIL会对C文件以及汇编文件执行上面流程，也就是说它们会分别形成汇编后的二进制文件，并最终被STM32对应的Linker链接在一起。链接器会自动将startup_stm32f10x_hd.s这一部分置于代码的开始部分。而这一文件最终被下载到开发板的内存中时，STM32开发板硬件会首先执行内存最开始部分对应的代码，也即startup_stm32f10x_hd.s对应的代码。

而startup_stm32f10x_hd.s中就包含了__main对应的入口跳转，也就最终执行了我们写的代码。

## 我们是如何控制嵌入式设备的

在C代码中，我们可以直观的看到我们在直接操作外设对应的寄存器的地址。也就是说，对于外设的操作，不过是对于某个地址的操作。这是我们后续操作的基础。
