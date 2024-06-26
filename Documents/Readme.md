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

# STM32与手机如何通信

下面我们不再花时间到STM32的一个个功能或模块上了，我们开始讨论这里的具体的问题。

最开始摆在我们面前的问题是STM32与手机如何通信？

借助搜索和chatGPT等AI工具，我们可以大概确定一些可能的方案，列表如下。

| 通信技术     | 特点                                        | 典型应用场景                         |
| ------------ | ------------------------------------------- | ------------------------------------ |
| **ZigBee**   | 低功耗，支持大型网络，自组织网络            | 家庭自动化，工业控制，能源管理       |
| **蓝牙**     | 易用，低功耗（特别是BLE），广泛的设备兼容性 | 消费电子（耳机，键盘），健康监测设备 |
| **Wi-Fi**    | 高数据速率，广泛使用，基于IP的网络          | 远程控制，智能家居，互联网接入       |
| **NFC**      | 极短距离通信，高安全性                      | 触摸支付，门禁系统，简单的数据交换   |
| **USB**      | 高数据传输速率，物理连接                    | 设备配置，数据同步，调试             |
| **移动网络** | 广覆盖范围，高数据速率                      | 远程监控，移动通信，紧急响应系统     |
| **串行通信** | 低成本，简单配置                            | 工业设备控制，旧式设备接口           |

我手头刚好有GSM的模块，所以我最终选定以此作为通信的基础。

# STM32中GSM使用

开始之前，请谨记：

我们不需要关注并理解与STM32、GSM模块相关的每个细节，我们只关心如何用最快的速度实现我们的目的。

## GSM是什么

GSM模块是一种基于全球移动通信系统（Global System for Mobile Communications，简称GSM）标凈的无线模块，用于实现移动设备的语音和数据传输。这种模块广泛应用于移动通信领域，特别是在嵌入式系统中，它允许设备通过移动电话网络发送和接收数据，进行语音通话，发送和接收短信(SMS)等。

GSM模块内部包含了一系列硬件组件，这些组件共同工作以支持模块的各种功能，包括接收、解析和响应AT命令。以下是GSM模块内部通常包含的主要硬件组件：微处理器、存储单元、射频（RF）模块、SIM卡接口等。

## GSM模块硬件连接

查看厂商提供的官方文档，会看到连接示意图。

我们简单介绍里面提到的两种信号电平，用于简单的概念理解(本段跳过阅读也可，只需完成硬件连接）。TTL信号定义为低电平（0V 至 0.8V）和高电平（2V 至 5V），这些电平用于表示数字逻辑的0和1。MAX3232是一种与RS-232标准兼容的串行通信接口芯片，它用于将TTL电平信号转换为RS-232电平信号，RS-232标准中，逻辑“1”（也称为“MARK”）通常表示为-25V至-3V，逻辑“0”（也称为“SPACE”）表示为+3V至+25V。

## GSM的AT命令

在厂商提供的GSM模块的介绍中，会看到此模块的控制是通过AT命令实现的。

SIM800系列芯片另有一个SIM800_Series_AT_Command_Manual_V1.09.pdf的文档，用于详细说明每一个AT命令。

我们此处暂不细看，后面先保证单片机能用AT命令对于模块进行基本的控制管理。

# UART RS-232 串口通信

使用D9线连接好STM32开发板和外设后，我们第一个问题在于我们如何与外设即GSM模块通信。

我们先来区分这几个概念：

**UART**是实现串口通信的一种硬件设备或硬件功能模块，存在于许多微控制器和处理器中。UART主要负责数据的串行化和解串行化，即将并行数据转换为串行数据进行发送，以及将接收到的串行数据转换回并行数据。UART本身并不定义电气信号的具体电平或物理连接方式，它只负责数据的基本传输机制。关于UART的起始位、数据位、校验位、终止位此处我们不再赘述，请自行查找资料了解。

**RS-232**是定义了串口通信电气特性的一种标准，包括信号电压级别、引脚分配、连接器类型等。RS-232主要用于计算机与外围设备或其他计算机之间的串行通信。它规定了使用正负电压来表示二进制的0和1，与TTL电平（0和5伏）不同。

**串口通信**是一个广泛的术语，通常指使用串行接口进行的数据传输。在讨论串口通信时，可能涉及到多种不同的硬件接口和通信标凼，包括RS-232、RS-485、USB等。

让我再用浅显的语言描述一下：

我们的开发板与外设连接时，最基本的要保证它们表示'0','1'时电压值是对应一致的，这种物理层面或者或电气层面的标准就是RS-232定义的。

而当我们要发送数据时，就会涉及数据的封装问题，比如说一次发送多少数据，发送的速率(波特率)等等，对于发送和接受方而言，要保证这两者的一致性。

比如当我们把PC和GSM模块连接的时候，发送方的发送速率和数据位数等可以使用串口传输软件控制，而接受方这些对UART的支持实际上在模块内部已经实现了(前面我们提到了GSM也是有处理器、闪存等单元的)。当二者一致时就可以完成通信，这些都是UART层面的问题。

# STM中UART的连接与使用

## UART模块

**物理连接：**

UART/USART是集成在芯片内部的模块，对于STM32F103ZE而言，数据手册上记录了其共有5个USART/UART模块。一般来讲，我们需要选择几个GPIO口作为UART的引脚以便和外部设备连接。这是因为STM32芯片提供了GPIO口复用的功能，也就是说GPIO接口在内部是与芯片内部模块有连接的，并且这种连接是可以配置的。也就是说，通过配置GPIO口的复用，可以将GPIO连接到内部UART模块，而GPIO再连接到外部设备，自然就完成了与通信设备的通路连接。

**数据寄存器**：

发送数据寄存器 (Transmit Data Register, TDR)：用于暂存待发送的数据。当UART准备发送数据时，数据首先被写入此寄存器。
接收数据寄存器 (Receive Data Register, RDR)：用于暂存接收到的数据。当数据通过UART接口接收时，它会被存储在此寄存器中，等待CPU读取。

**控制寄存器：**

UE: USART使能。设置后USART模块才能正常工作。

TE: 发送使能

RE: 接受使能

TXEIE：发送缓冲区空中断使能 (TXE interrupt enable) 

RXNEIE：接收缓冲区非空中断使能 (RXNE interrupt enable)

**状态寄存器：**

TXE 表示数据已经从数据寄存器（USART_DR）转移到了移位寄存器，并准备进行串行发送。

TC 则表示整个数据包已经完全发送出去，包括最后的停止位，发送线已经空闲。

RXNE即USART_DR寄存器不为空。当接收到数据并存入USART_DR寄存器后，RXNE位被硬件自动设置为1。这个标志位用于指示有数据可供读取，从而避免在读取空寄存器时可能发生的错误或无效操作。

**波特比率寄存器(USART_BRR)** 

这个寄存器涉及波特率的设置，我们暂不细看。

## UART代码实现

**GPIO复用设置：**

STM32中有一个AFIO寄存器，用于重映射外设功能到其他引脚或解决引脚冲突。我们使用开发板上的串口已经默认复用到了UART模块，所以无需进行复用设置。

**使能GPIO时钟：** 

对于GPIO而言，GPIO端口的寄存器只有在其时钟使能后才可以被访问。 所以在配置前应该首先使能GPIO时钟。

**GPIO引脚设置：**

设置GPIO模式和输出类型：即设置STM32的GPIO为UART TX（复用推挽输出）和RX（浮空输入），确保信号的正确传输和接收。

复用推挽：CNF1: 1 CNF0: 0 MODE1 MODE0 : 11 

浮空输入：CNF1: 0 CNF0: 1 MODE1 MODE0 : 00

官方文档中描述： 

PB10 I2C2_SCL/USART3_TX 

PB11 I2C2_SDA/USART3_RX

即此处需要分别设置GPIOB的10 ，10 端口

**配置USART参数：**
包括波特率、数据位、停止位和奇偶校验位等。

**使能USART：**
在完成所有必要的配置后，最后步骤是使能USART。这是因为改变USART基本的通信参数通常需要USART处于禁用状态，以确保设置的正确应用并防止在通信过程中发生配置更改带来的潜在错误。

**说明：** 开发板的硬件文档和数据手册会说明哪个GPIO口和哪个UART模块是对应的，请参阅文档完成。
