#include "main.h"
volatile uint16_t first; // press
volatile uint16_t second; // release
volatile uint16_t last_time; //previous release
volatile uint16_t diff; // release - press
volatile uint16_t current_time; //current time form timer register
volatile uint8_t state = 0; // Encode or decode
volatile uint8_t symbol_ready; // ready to store a symbol
volatile uint16_t dot_duration = 2100; //initial value
volatile uint32_t silence = 0; // end of input signal.

volatile char arr[50];
void uart2_init(void)
{
    // Enable GPIOA clock (already enabled in your code, but safe)
    RCC_IOPENR |= (1<<0);

    // Enable USART2 clock
    RCC_APB1ENR |= (1<<17);

    // PA2, PA3 → Alternate function mode
    GPIOA_MODER &= ~((3U<<(2*2)) | (3U<<(3*2)));
    GPIOA_MODER |=  ((2U<<(2*2)) | (2U<<(3*2)));

    // AF1 = USART2 on PA2, PA3
    GPIOA_AFRL &= ~((0xF<<(4*2)) | (0xF<<(4*3)));
    GPIOA_AFRL |=  ((1<<(4*2)) | (1<<(4*3)));

    // Baud = 115200 @ 16MHz
    USART2_BRR = 16000000 / 115200;

    // Enable USART, TX, RX
    USART2_CR1 |= (1<<0) | (1<<2) | (1<<3);
}
void uart2_write(char c)
{
    while(!(USART2_ISR & (1<<7))); // TXE
    USART2_TDR = c;
}
void uart2_print(char *s)
{
    while(*s) uart2_write(*s++);
}

// print integer number
void uart2_print_num(uint16_t n)
{
    char buf[6];
    int i = 0;

    if(n == 0){
        uart2_write('0');
        return;
    }

    while(n > 0){
        buf[i++] = (n % 10) + '0';
        n /= 10;
    }
    while(i--) uart2_write(buf[i]);
}


volatile uint8_t button_state = 0;
// 0 = released, 1 = pressed

void EXTI0_1_IRQHandler(void)
{
    current_time = TIM6_CNT;

    // Clear EXTI pending flags
    if(EXTI_RPR1 & 1) EXTI_RPR1 = (1<<0);
    if(EXTI_FPR1 & 1) EXTI_FPR1 = (1<<0);

    // Debounce check
    if((uint16_t)(current_time - last_time) < 150U)
        return;

    // Read actual pin level (PA0)
    uint8_t pin_level = (*(volatile uint32_t*)(GPIOA_BASE + 0x10)) & 1;

    //  State machine
    if(pin_level && button_state == 0)
    {
        // Valid press
        first = current_time;
        last_time = current_time;
        button_state = 1;
    }
    else if(!pin_level && button_state == 1)
    {
        // Valid release
        second = current_time;
        diff = second - first;
        last_time = current_time;
        button_state = 0;
        symbol_ready = 1;
    }
}


void calibration_mode(void)
{
	uart2_print("Enter few random dots for calibration\n\r");
	uint16_t gap=0;
	uint8_t count=0;
	uint32_t dot_time=0;
	uint16_t previous_time = 0;
	while(gap < 30000)
	{
		if(symbol_ready)
		{
			dot_time = dot_time + diff;
			count++;
			if(previous_time!=0)
			{
				gap =  first - previous_time;
			}
			previous_time = second;
			symbol_ready=0;
		}
		else if(previous_time!=0)
		{
			current_time = TIM6_CNT;
			silence = current_time - previous_time;
			if(silence > 20000)
				break;
		}
	}
	dot_duration = dot_time/count;
}


void init_externel_interrupr(void)
{
	EXTICR1 &= ~(0xF << 0);   // EXTI0 select Port A (0000)
	EXTI_RTSR1 |= (1<<0); //set rising edge
	EXTI_FTSR1 |= (1<<0); //set falling edge
	EXTI_IMR1 |= (1<<0); //unmasking the interrupt siganl source]
	NVIC_ISER |= (1<<5); //enables IRQ number 5.
}

void inti_gpio(void)
{
	RCC_IOPENR = RCC_IOPENR | (1 << 0); // Enable GPIOA clock
	GPIOA_MODER &= ~(3U << 0); //sets GPIOA pin 0 as input.
	GPIOA_PU = GPIOA_PU & ~(3U << (0 * 2));
	GPIOA_PU = GPIOA_PU | (2U << (0 * 2));
}

void init_clock_enable(void)
{
	RCC_APBENR2 = RCC_APBENR2 | (1<<0); //enables the clock for SYSCFG.
}

void init_timer(void)
{
	RCC_APB1ENR = RCC_APB1ENR | (1<<4); //enables clock for TIM6.
	TIM6_PSC = 999;      // divide 16 MHz by 1000
	TIM6_ARR = 65535;    // full 16-bit range
	TIM6_CR1 |= 1;       // enable counter
}

void iterate_foreward(int i, uint16_t *arr_indx)
{
	while(arr[*arr_indx] != '\0' && *arr_indx < i)
	{
		*arr_indx = *arr_indx + 1;
	}
	if(arr[*arr_indx] == '\0')
	{
		*arr_indx = *arr_indx + 1;
		if(arr[*arr_indx] == '\0')
		{
			*arr_indx++;
			uart2_write(' ');
		}
	}

}
int main(void)
{
	volatile uint16_t gap = 0;
	volatile uint16_t previous_second = 0;
	init_clock_enable();
	inti_gpio();
	init_externel_interrupr();
	init_timer();
	uart2_init();
	//calibration_mode();
	silence=0;
	symbol_ready=0;
	int i=0;
	while(1)
	{
		if(state) //decode the MORSE CODE
		{
			volatile uint16_t code_indx = 0;
			volatile uint16_t arr_indx = 0;
			uart2_print_num(i);
			uart2_print("\n\r");
			while(arr_indx < i)
			{
				code_indx = 0;
				while(code_indx < 26) //compare with the code table
				{
					if((strcmp(*(morse_table + code_indx),(const char *)(arr + arr_indx))) == 0) //found
					{
						uart2_write(65+code_indx);
						iterate_foreward(i,&arr_indx);
						break;
					}
					else
					{
						code_indx++;
						if(code_indx==26)
						{
							iterate_foreward(i,&arr_indx);
							uart2_write('?'); //print '?' if the code is error.
						}
					}
				}
			}
			state = 0; //sets to encode mode.
			symbol_ready=0;
			previous_second=0;
			i=0;
		}
		else //encode the MORSE CODE
		{
			if(symbol_ready)
			{
				if(previous_second!=0)
				{
					gap = first - previous_second;
				}

				//checking the gap;
				if(gap >= 3 * dot_duration && gap < 8 * dot_duration) // letter gap.
				{
					arr[i++] = '\0';
				}
				else if(gap >= 8 * dot_duration)//word_gap.
				{
					arr[i++]='\0';
					arr[i++]='\0';
				}

				//writing the symbol into buffer
				if(diff< 2*dot_duration)
					arr[i++] = '.';
				else
					arr[i++] = '-';

				symbol_ready=0;
				previous_second = second;
			}
			else if(previous_second!=0 && button_state == 0)
			{
				current_time = TIM6_CNT; //if symbol is not ready then check the current time.
				silence = (uint16_t)(current_time - previous_second); //find the the silince time form current and release time.
				if(silence > 25*dot_duration) //if silinece time is greater than twice of word time then exit
						state = 1; // sets to decode mode
			}
		}
	}
}