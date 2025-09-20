#if 1
/*
 * Copyright (c) 2024, sakumisu
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "usbd_core.h"
#include "usbd_hid.h"

#define USBD_VID					  0x1234
#define USBD_PID					  0xffff
#define USBD_MAX_POWER				  100
#define USBD_LANGID_STRING			  1033

#define HID_INT_EP					  0x81
#define HID_INT_EP_SIZE				  8
#define HID_INT_EP_INTERVAL			  10

#define USB_HID_CONFIG_DESC_SIZ		  (9 + HID_KEYBOARD_DESCRIPTOR_LEN)
#define HID_KEYBOARD_REPORT_DESC_SIZE 63

#ifdef CONFIG_USBDEV_ADVANCE_DESC
static const uint8_t device_descriptor[] = {
	USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0x00, 0x00, 0x00, USBD_VID, USBD_PID, 0x0002, 0x01)};

static const uint8_t config_descriptor[] = {
	USB_CONFIG_DESCRIPTOR_INIT(USB_HID_CONFIG_DESC_SIZ, 0x01, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
	HID_KEYBOARD_DESCRIPTOR_INIT(
		0x00,						   /* bInterfaceNumber */
		0x01,						   /* bInterfaceSubClass : 1=BOOT, 0=no boot */
		HID_KEYBOARD_REPORT_DESC_SIZE, /* wItemLength: Total length of Report descriptor */
		HID_INT_EP,					   /* bEndpointAddress: Endpoint Address (IN) */
		HID_INT_EP_SIZE,			   /* wMaxPacketSize: 4 Byte max */
		HID_INT_EP_INTERVAL			   /* bInterval: Polling Interval */
		)};

static const uint8_t device_quality_descriptor[] = {
	///////////////////////////////////////
	/// device qualifier descriptor
	///////////////////////////////////////
	0x0a,
	USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
	0x00,
	0x02,
	0x00,
	0x00,
	0x00,
	0x40,
	0x00,
	0x00,
};

static const char *string_descriptors[] = {
	(const char[]){0x09, 0x04}, /* Langid */
	"CherryUSB",				/* Manufacturer */
	"CherryUSB HID DEMO",		/* Product */
	"2022123456",				/* Serial Number */
};

static const uint8_t *device_descriptor_callback(uint8_t speed)
{
	return device_descriptor;
}

static const uint8_t *config_descriptor_callback(uint8_t speed)
{
	return config_descriptor;
}

static const uint8_t *device_quality_descriptor_callback(uint8_t speed)
{
	return device_quality_descriptor;
}

static const char *string_descriptor_callback(uint8_t speed, uint8_t index)
{
	if (index > 3)
	{
		return NULL;
	}
	return string_descriptors[index];
}

const struct usb_descriptor hid_descriptor = {
	.device_descriptor_callback			= device_descriptor_callback,
	.config_descriptor_callback			= config_descriptor_callback,
	.device_quality_descriptor_callback = device_quality_descriptor_callback,
	.string_descriptor_callback			= string_descriptor_callback};
#endif

static const uint8_t hid_keyboard_report_desc[HID_KEYBOARD_REPORT_DESC_SIZE] = {
	0x05, 0x01, // USAGE_PAGE (Generic Desktop)
	0x09, 0x06, // USAGE (Keyboard)
	0xa1, 0x01, // COLLECTION (Application)
	0x05, 0x07, // USAGE_PAGE (Keyboard)
	0x19, 0xe0, // USAGE_MINIMUM (Keyboard LeftControl)
	0x29, 0xe7, // USAGE_MAXIMUM (Keyboard Right GUI)
	0x15, 0x00, // LOGICAL_MINIMUM (0)
	0x25, 0x01, // LOGICAL_MAXIMUM (1)
	0x75, 0x01, // REPORT_SIZE (1)
	0x95, 0x08, // REPORT_COUNT (8)
	0x81, 0x02, // INPUT (Data,Var,Abs)
	0x95, 0x01, // REPORT_COUNT (1)
	0x75, 0x08, // REPORT_SIZE (8)
	0x81, 0x03, // INPUT (Cnst,Var,Abs)
	0x95, 0x05, // REPORT_COUNT (5)
	0x75, 0x01, // REPORT_SIZE (1)
	0x05, 0x08, // USAGE_PAGE (LEDs)
	0x19, 0x01, // USAGE_MINIMUM (Num Lock)
	0x29, 0x05, // USAGE_MAXIMUM (Kana)
	0x91, 0x02, // OUTPUT (Data,Var,Abs)
	0x95, 0x01, // REPORT_COUNT (1)
	0x75, 0x03, // REPORT_SIZE (3)
	0x91, 0x03, // OUTPUT (Cnst,Var,Abs)
	0x95, 0x06, // REPORT_COUNT (6)
	0x75, 0x08, // REPORT_SIZE (8)
	0x15, 0x00, // LOGICAL_MINIMUM (0)
	0x25, 0xFF, // LOGICAL_MAXIMUM (255)
	0x05, 0x07, // USAGE_PAGE (Keyboard)
	0x19, 0x00, // USAGE_MINIMUM (Reserved (no event indicated))
	0x29, 0x65, // USAGE_MAXIMUM (Keyboard Application)
	0x81, 0x00, // INPUT (Data,Ary,Abs)
	0xc0		// END_COLLECTION
};

#define HID_STATE_IDLE 0
#define HID_STATE_BUSY 1

/*!< hid state ! Data can be sent only when state is idle  */
// static volatile uint8_t hid_state = HID_STATE_IDLE;

extern volatile uint8_t hid_state;
static void usbd_event_handler(uint8_t busid, uint8_t event)
{
	switch (event)
	{
		case USBD_EVENT_RESET:
			break;
		case USBD_EVENT_CONNECTED:
			break;
		case USBD_EVENT_DISCONNECTED:
			break;
		case USBD_EVENT_RESUME:
			break;
		case USBD_EVENT_SUSPEND:
			break;
		case USBD_EVENT_CONFIGURED:
			hid_state = HID_STATE_IDLE;
			break;
		case USBD_EVENT_SET_REMOTE_WAKEUP:
			break;
		case USBD_EVENT_CLR_REMOTE_WAKEUP:
			break;

		default:
			break;
	}
}

void usbd_hid_int_callback(uint8_t busid, uint8_t ep, uint32_t nbytes);

static struct usbd_endpoint hid_in_ep = {
	.ep_cb	 = usbd_hid_int_callback,
	.ep_addr = HID_INT_EP};

struct usbd_interface intf0;

void hid_keyboard_init(uint8_t busid, uintptr_t reg_base)
{
#ifdef CONFIG_USBDEV_ADVANCE_DESC
	usbd_desc_register(busid, &hid_descriptor);
#endif
	usbd_add_interface(busid, usbd_hid_init_intf(busid, &intf0, hid_keyboard_report_desc, HID_KEYBOARD_REPORT_DESC_SIZE));
	usbd_add_endpoint(busid, &hid_in_ep);

	usbd_initialize(busid, reg_base, usbd_event_handler);
}

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t write_buffer[64];

void hid_keyboard_test(uint8_t busid)
{
	const uint8_t sendbuffer[8] = {0x00, 0x00, HID_KBD_USAGE_TAB, 0x00, 0x00, 0x00, 0x00, 0x00};

	if (usb_device_is_configured(busid) == false)
	{
		return;
	}

	memcpy(write_buffer, sendbuffer, 8);
	hid_state = HID_STATE_BUSY;
	usbd_ep_start_write(busid, HID_INT_EP, write_buffer, 8);
	while (hid_state == HID_STATE_BUSY)
	{
	}
}
#endif