.section .vectors, "a", %progbits
    jmp __vector_RESET
    jmp __vector_INT0
    jmp __vector_INT1
    jmp __vector_PCINT0
    jmp __vector_PCINT1
    jmp __vector_PCINT2
    jmp __vector_WDT
    jmp __vector_TIMER2_COMPA
    jmp __vector_TIMER2_COMPB
    jmp __vector_TIMER2_OVF
    jmp __vector_TIMER1_CAPT
    jmp __vector_TIMER1_COMPA
    jmp __vector_TIMER1_COMPB
    jmp __vector_TIMER1_OVF
    jmp __vector_TIMER0_COMPA
    jmp __vector_TIMER0_COMPB
    jmp __vector_TIMER0_OVF
    jmp __vector_SPI_STC
    jmp __vector_USART_RX
    jmp __vector_USART_UDRE
    jmp __vector_USART_TX
    jmp __vector_ADC
    jmp __vector_EE_READY
    jmp __vector_ANALOG_COMP
    jmp __vector_TWI
    jmp __vector_SPM_Ready

.section .text
.global  __vector_default
__vector_default:
    rjmp __vector_default

.macro IRQ handler
    .weak  \handler
    .set   \handler, __vector_default
.endm

IRQ __vector_INT0
IRQ __vector_INT1
IRQ __vector_PCINT0
IRQ __vector_PCINT1
IRQ __vector_PCINT2
IRQ __vector_WDT
IRQ __vector_TIMER2_COMPA
IRQ __vector_TIMER2_COMPB
IRQ __vector_TIMER2_OVF
IRQ __vector_TIMER1_CAPT
IRQ __vector_TIMER1_COMPA
IRQ __vector_TIMER1_COMPB
IRQ __vector_TIMER1_OVF
IRQ __vector_TIMER0_COMPA
IRQ __vector_TIMER0_COMPB
IRQ __vector_TIMER0_OVF
IRQ __vector_SPI_STC
IRQ __vector_USART_RX
IRQ __vector_USART_UDRE
IRQ __vector_USART_TX
IRQ __vector_ADC
IRQ __vector_EE_READY
IRQ __vector_ANALOG_COMP
IRQ __vector_TWI
IRQ __vector_SPM_Ready
