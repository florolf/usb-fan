#include <stdlib.h>
#include <string.h>

#include <libopencm3/usb/usbd.h>
#include <libopencm3/stm32/desig.h>
#include <libopencm3/stm32/st_usbfs.h>
#include <libopencm3/cm3/scb.h>

#include "usb.h"
#include "fan.h"

static usbd_device *usbd_dev;

static const struct usb_device_descriptor dev = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0xff,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = 0x621,
	.idProduct = 0x6,
	.bcdDevice = 0x0200,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};

const struct usb_endpoint_descriptor transfer_endp[] = {
	{
		.bLength = USB_DT_ENDPOINT_SIZE,
		.bDescriptorType = USB_DT_ENDPOINT,
		.bEndpointAddress = 0x01,
		.bmAttributes = USB_ENDPOINT_ATTR_BULK,
		.wMaxPacketSize = 64,
		.bInterval = 1,
	},
	{
		.bLength = USB_DT_ENDPOINT_SIZE,
		.bDescriptorType = USB_DT_ENDPOINT,
		.bEndpointAddress = 0x82,
		.bmAttributes = USB_ENDPOINT_ATTR_BULK,
		.wMaxPacketSize = 64,
		.bInterval = 1,
	}
};

const struct usb_interface_descriptor iface = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = 0xFF,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = transfer_endp,
};

const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.altsetting = &iface,
}};

const struct usb_config_descriptor config = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = 1,
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0x80,
	.bMaxPower = 0x32,

	.interface = ifaces,
};

char serial[25];
const char *usb_strings[] = {
	"n621",
	"usbdim",
	serial,
};

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];

extern uint32_t *bootloader_magic;
static enum usbd_request_return_codes control_request(usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf,
		uint16_t *len, void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
	(void)buf;
	(void)len;
	(void)complete;
	(void)usbd_dev;
	(void)req;

	switch (req->bRequest) {
		case 0:
			fan_set((int16_t)req->wValue);
			return USBD_REQ_HANDLED;

		case 0xff:
			*bootloader_magic = 0xb007b007;
			scb_reset_system();

			// never reached
			return USBD_REQ_HANDLED;
	}

	return USBD_REQ_NOTSUPP;
}

static void set_config(usbd_device *usbd_dev, uint16_t wValue)
{
	(void) wValue;

	usbd_register_control_callback(
			usbd_dev,
			USB_REQ_TYPE_DEVICE | USB_REQ_TYPE_VENDOR,
			USB_REQ_TYPE_DEVICE | USB_REQ_TYPE_VENDOR,
			control_request);
}

static void usb_suspend_cb(void)
{
	fan_sleep(true);
}

static void usb_wake_cb(void)
{
	fan_sleep(false);
}

void init_usb(void)
{
	usbd_dev = usbd_init(&st_usbfs_v2_usb_driver, &dev, &config, usb_strings, 3, usbd_control_buffer, sizeof(usbd_control_buffer));

	usbd_register_set_config_callback(usbd_dev, set_config);
	usbd_register_suspend_callback(usbd_dev, usb_suspend_cb);
	usbd_register_resume_callback(usbd_dev, usb_wake_cb);
	usbd_register_reset_callback(usbd_dev, usb_wake_cb);

	desig_get_unique_id_as_string(serial, sizeof(serial));
}

void usb_poll(void)
{
	usbd_poll(usbd_dev);
}
