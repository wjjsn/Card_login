#include "cmsis_os2.h"
#include "pn532.hpp"
#include "hal.hpp"
#include "chry_ringbuffer.hpp"
#include "SEGGER_RTT.h"
#include "stm32f1xx_hal_def.h"
#include <cstdio>
#include <cstring>
#include <stdint.h>
#include "usb_hid.h"
#include "usbd_core.h"
#include "usbd_hid.h"
using namespace HAL::stm32;
chry_ringbuffer_t rb;
std::uint8_t data[128];
using RB	= RingBuffer<&rb, static_cast<uint8_t *>(data), 128>;
using uart1 = UART_LL<USART1_BASE>;
using PN532 = PN532_<PN532_HAL_UART<uart1>, RB>;
PN532 pn532;

bool sem = false;
std::uint8_t scan_card_id[4];
constexpr std::uint8_t target_card_id[4]{0x62, 0x3F, 0x7B, 0x51};
extern "C"
{
	void StartPN532_tsstTask(void *argument)
	{
		(void)argument;
		RB::init();
		pn532.wake_up();
		osDelay(20);
		pn532.get_firmware_version();
		osDelay(20);

		while (true)
		{
			pn532.scan_card(PN532::card_type::ISO14443A);
			osDelay(2000);
		}
	}
	void Start_PN532_Task(void *argument)
	{
		(void)argument;
		while (true)
		{
			pn532.Rx_Handle();
			if (memcmp(scan_card_id, target_card_id, 4) == 0)
			{
				sem = true;
				memset(scan_card_id, 0, 4);
			}
			osDelay(5);
		}
	}

	extern USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t write_buffer[64];
#define HID_STATE_IDLE 0
#define HID_STATE_BUSY 1
#define HID_INT_EP	   0x81
	volatile uint8_t hid_state = HID_STATE_IDLE;
	void usbd_hid_int_callback(uint8_t busid, uint8_t ep, uint32_t nbytes)
	{
		hid_state = HID_STATE_IDLE;
	}
	void send_key(uint8_t *sendbuffer)
	{
		if (usb_device_is_configured(0) == false)
		{
			osDelay(1);
		}

		memcpy(write_buffer, sendbuffer, 8);
		hid_state = HID_STATE_BUSY;
		usbd_ep_start_write(0, HID_INT_EP, write_buffer, 8);
		while (hid_state == HID_STATE_BUSY)
		{
			osDelay(1);
		}
		osDelay(1);
	}
	void StartKeyBoardTask(void *argument)
	{

		UNUSED(argument);
		uint8_t sendbuffer[8] = {0x00, 0x00, HID_KBD_USAGE_TAB, 0x00, 0x00, 0x00, 0x00, 0x00};
		while (true)
		{
			if (sem)
			{
				sendbuffer[2] = HID_KBD_USAGE_A + 'm' - 'a';
				send_key(sendbuffer);
				sendbuffer[2] = HID_KBD_USAGE_A + 'i' - 'a';
				send_key(sendbuffer);
				sendbuffer[2] = HID_KBD_USAGE_A + 'k' - 'a';
				send_key(sendbuffer);
				sendbuffer[2] = HID_KBD_USAGE_A + 'u' - 'a';
				send_key(sendbuffer);
				sendbuffer[2] = HID_KBD_USAGE_1 + 8;
				send_key(sendbuffer);
				sendbuffer[2] = HID_KBD_USAGE_1 + 5;
				send_key(sendbuffer);
				sendbuffer[2] = HID_KBD_USAGE_1 + 1;
				send_key(sendbuffer);
				sendbuffer[2] = HID_KBD_USAGE_1 + 3;
				send_key(sendbuffer);
				sendbuffer[2] = HID_KBD_USAGE_1 + 5;
				send_key(sendbuffer);
				sendbuffer[2] = HID_KBD_USAGE_1 + 3;
				send_key(sendbuffer);
				sendbuffer[2] = HID_KBD_USAGE_ENTER;
				send_key(sendbuffer);
				// osDelay(1);
				sendbuffer[2] = 0;
				send_key(sendbuffer);

				sem = false;
			}
			else
			{
				osDelay(1);
			}
		}
	}
	void USART1_IRQHandler(void)
	{
		if (LL_USART_IsActiveFlag_RXNE(USART1))
		{
			RB::overwrite_byte(LL_USART_ReceiveData8(USART1));
		}
	}
	int _write(int file, char *ptr, int len)
	{
		(void)file;
		// uart1::transmit(reinterpret_cast<const std::uint8_t *>(ptr), len, 0);
		SEGGER_RTT_Write(0, ptr, len);
		return len;
	}
}