#include "../rom/mw_usbd_rom_api.h"
#include "../inc/power_api.h"
#include "../../core/LPC13Uxx.h"
#include "../inc/CDC.h"
#include "../inc/cdc_desc.h"
#include "../inc/serial_fifo.h"
//#include "../../library/libDNCLABS.h"
#include <string.h>

#if defined (__cplusplus)
extern "C" {
#endif

#define EOF      (-1)
volatile unsigned char SOFcounter=0;

uint8_t txdata[VCOM_FIFO_SIZE];
uint8_t rxdata[VCOM_FIFO_SIZE];

fifo_t txfifo;
fifo_t rxfifo;

#define NOT_USED(x) ((void)(x))

USBD_API_T* pUsbApi;
struct VCOM_DATA;
typedef void (*VCOM_SEND_T) (struct VCOM_DATA* pVcom);

typedef struct VCOM_DATA {
	USBD_HANDLE_T hUsb;
	USBD_HANDLE_T hCdc;
	uint8_t* rxBuf;
	uint8_t* txBuf;
	volatile uint8_t ser_pos;
	volatile uint16_t rxlen;
	volatile uint16_t txlen;
	VCOM_SEND_T send_fn;
	volatile uint32_t sof_counter;
	volatile uint32_t last_ser_rx;
	volatile uint16_t break_time;
	volatile uint16_t usbrx_pend;
} VCOM_DATA_T; 
VCOM_DATA_T g_vCOM;

volatile int current_time;				//holds the current Ms count of SysTick
volatile uint32_t msTicks = 0;
void delay_ms(uint32_t ms) {
	uint32_t now = msTicks;
	while ((msTicks-now) < ms);
}

//Interrupt handler for SysTick timer
void SysTick_Handler(void)
{
	msTicks++;
	current_time++;
	CDC_Send();
}

void USB_pin_clk_init(void)
{
	// Enable AHB clock to the GPIO domain.
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6);

	// Enable AHB clock to the USB block and USB RAM.
	LPC_SYSCON->SYSAHBCLKCTRL |= ((0x1<<14)|(0x1<<27));

	// Pull-down is needed, or internally, VBUS will be floating. This is to
	//address the wrong status in VBUSDebouncing bit in CmdStatus register.
	LPC_IOCON->PIO0_3   &= ~0x1F; 
	LPC_IOCON->PIO0_3   |= (0x01<<0);			// Secondary function VBUS
	LPC_IOCON->PIO0_6   &= ~0x07;
	LPC_IOCON->PIO0_6   |= (0x01<<0);			// Secondary function SoftConn
	return;
}

void VCOM_usb_send(VCOM_DATA_T* pVcom)
{ 
	NOT_USED(pVcom);
}

void usb_send_str(char *text)
{
	VCOM_DATA_T* pVcom = &g_vCOM;
	int len = strlen(text);
	for(int u = 0;u < len;u++){
		pVcom->txBuf[u] = text[u];
	}
	pVcom->txlen -= pUsbApi->hw->WriteEP (pVcom->hUsb, USB_CDC_EP_BULK_IN,pVcom->txBuf, len);   
}

ErrorCode_t VCOM_SendBreak (USBD_HANDLE_T hCDC, uint16_t mstime)
{
	NOT_USED(hCDC);
	VCOM_DATA_T* pVcom = &g_vCOM;
	uint8_t lcr = LPC_USART->LCR;

	if ( mstime) {
		lcr |= (1 << 6);
	} else {
		lcr &= ~(1 << 6);
	}

	pVcom->break_time = mstime;
	LPC_USART->LCR = lcr;

	return LPC_OK;
}

ErrorCode_t BulkIn(USBD_HANDLE_T hUsb, void* data, uint32_t event) 
{	
	NOT_USED(hUsb);
	NOT_USED(data);
	NOT_USED(event);
	return LPC_OK;
}

ErrorCode_t BulkOut(USBD_HANDLE_T hUsb, void* data, uint32_t event) 
{
	VCOM_DATA_T* pVcom = (VCOM_DATA_T*) data;
	int i;

	switch (event) {
		case USB_EVT_OUT:
			if (fifo_free(&rxfifo) < CDC_BUF_SIZE) {
				return ERR_FAILED;							// may not fit into fifo
			}

			pVcom->rxlen = pUsbApi->hw->ReadEP(hUsb, USB_CDC_EP_BULK_OUT, pVcom->rxBuf); 
			for (i = 0; i < pVcom->rxlen; i++) {
				fifo_put(&rxfifo, pVcom->rxBuf[i]);
			}		   
			break;
		default: 
			break;
	}
	return LPC_OK;
}
void USB_IRQHandler(void)
{
	pUsbApi->hw->ISR(g_vCOM.hUsb);
}

void usb_init(void) {

	USBD_API_INIT_PARAM_T usb_param;
	USBD_CDC_INIT_PARAM_T cdc_param;
	USB_CORE_DESCS_T desc;
	USBD_HANDLE_T hUsb, hCdc;
	ErrorCode_t ret = LPC_OK;
	uint32_t ep_indx;

	// get USB API table pointer
	pUsbApi = (USBD_API_T*)((*(ROM **)(0x1FFF1FF8))->pUSBD);

	// enable clocks and pinmux 
	USB_pin_clk_init();

	// initilize call back structures 
	memset((void*)&usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));
	usb_param.usb_reg_base = LPC_USB_BASE;
	usb_param.mem_base = 0x10001000;
	usb_param.mem_size = 0x1000;
	usb_param.max_num_ep = 3;

	//init CDC params 
	memset((void*)&cdc_param, 0, sizeof(USBD_CDC_INIT_PARAM_T));
	memset((void*)&g_vCOM, 0, sizeof(VCOM_DATA_T));

	// user defined functions 
//  cdc_param.SetLineCode = VCOM_SetLineCode; 
	cdc_param.SendBreak = VCOM_SendBreak;
//	usb_param.USB_SOF_Event = VCOM_sof_event;  

	// Initialize Descriptor pointers
	memset((void*)&desc, 0, sizeof(USB_CORE_DESCS_T));
	desc.device_desc = (uint8_t *)&VCOM_DeviceDescriptor[0];
	desc.string_desc = (uint8_t *)&VCOM_StringDescriptor[0];
	desc.full_speed_desc = (uint8_t *)&VCOM_ConfigDescriptor[0];
	desc.high_speed_desc = (uint8_t *)&VCOM_ConfigDescriptor[0];

	// USB Initialization 
	ret = pUsbApi->hw->Init(&hUsb, &desc, &usb_param);  
	if (ret == LPC_OK) {

		// init CDC params
		cdc_param.mem_base = usb_param.mem_base;
		cdc_param.mem_size = usb_param.mem_size;
		cdc_param.cif_intf_desc = (uint8_t *)&VCOM_ConfigDescriptor[USB_CONFIGUARTION_DESC_SIZE];
		cdc_param.dif_intf_desc = (uint8_t *)&VCOM_ConfigDescriptor[USB_CONFIGUARTION_DESC_SIZE + \
								  USB_INTERFACE_DESC_SIZE + 0x0013 + USB_ENDPOINT_DESC_SIZE ];

		ret = pUsbApi->cdc->init(hUsb, &cdc_param, &hCdc);

		if (ret == LPC_OK) {
			// store USB handle
			g_vCOM.hUsb = hUsb;
			g_vCOM.hCdc = hCdc;
			g_vCOM.send_fn = VCOM_usb_send;

			// allocate transfer buffers
			g_vCOM.rxBuf = (uint8_t*)(cdc_param.mem_base + (0 * USB_HS_MAX_BULK_PACKET));
			g_vCOM.txBuf = (uint8_t*)(cdc_param.mem_base + (1 * USB_HS_MAX_BULK_PACKET));
			cdc_param.mem_size -= (4 * USB_HS_MAX_BULK_PACKET);

			//register endpoint interrupt handler
			ep_indx = (((USB_CDC_EP_BULK_IN & 0x0F) << 1) + 1);
			ret = pUsbApi->core->RegisterEpHandler (hUsb, ep_indx, BulkIn, &g_vCOM);
			if (ret == LPC_OK) {
				// register endpoint interrupt handler
				ep_indx = ((USB_CDC_EP_BULK_OUT & 0x0F) << 1);
				ret = pUsbApi->core->RegisterEpHandler (hUsb, ep_indx, BulkOut, &g_vCOM);
				if (ret == LPC_OK) {
					NVIC_EnableIRQ(USB_IRQ_IRQn); 		//  enable USB0 interrrupts 
					pUsbApi->hw->Connect(hUsb, 1);		// USB Connect
					SOFcounter = 0;
				}
			}
		}    
	}
}

void CDC_Init(void) 
{
	SysTick_Config (SystemCoreClock/1000);		//1kHz (1ms)
	fifo_init(&txfifo, txdata);
	fifo_init(&rxfifo, rxdata);
	usb_init();
	SOFcounter = 0;
//	pUsbApi->hw->EnableEvent(pVcom->hUsb, 0, USB_EVT_SOF, 1);//EP0, SOF, enable
//	LPC_USB->INTEN  |= 1<<30;		//Really enable SOF interrupt
									//the previous api call doesn't do it.
									//Well the internal drivers disable this!
}

//	Writes one character (c) to VCOM port
//	returns character written, or EOF if character could not be written
void USB_putchar(unsigned char c)
{
	if(c == '\n'){
		fifo_put(&txfifo, '\r');
	}
	c = fifo_put(&txfifo, c) ? c : EOF;
}

unsigned char USB_RxReady(void)
{
	if(!fifo_avail(&rxfifo)){	// check if FIFO has data
		return FALSE;
	}
	return TRUE;
}

unsigned char USB_getchar(void)
{
	VCOM_DATA_T* pVcom = &g_vCOM;
	unsigned char c;
	c = fifo_get(&rxfifo, &c) ? c : EOF;
	while(c == 0xff){c = fifo_get(&rxfifo, &c) ? c : EOF;}
	pVcom->last_ser_rx = pVcom->sof_counter;
	return c;
}

void CDC_Send(void)
{
uint8_t c;
int i,length;
	VCOM_DATA_T* pVcom = &g_vCOM;

	length = fifo_avail(&txfifo);
	SOFcounter++;
	
	if (length && SOFcounter > 5) {
		SOFcounter = 0;
		if(length > 64){
			length = 64;
			for(i = 0; i < length; i++){
				c = fifo_get(&txfifo, &c) ? c : EOF;
				pVcom->txBuf[i] = c;
			}
			pUsbApi->hw->WriteEP (pVcom->hUsb, USB_CDC_EP_BULK_IN,pVcom->txBuf, length); 
			length = 0;
		};
	
		if(length){
			for(i = 0; i < length; i++){
				c = fifo_get(&txfifo, &c) ? c : EOF;
				pVcom->txBuf[i] = c;
			}
			pUsbApi->hw->WriteEP (pVcom->hUsb, USB_CDC_EP_BULK_IN,pVcom->txBuf, length); 
		}
	}
}

#if defined (__cplusplus)
} // extern "C"
#endif
