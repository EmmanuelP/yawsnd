#include <stdlib.h>
#include <stdio.h>
#include <glib-object.h>
#include <gusb.h>
#include <mosquitto.h>

#define YAWSND_COMMAND_CONFIG		0x56
#define YAWSND_COMMAND_READ_CURRENT	0xb3
#define YAWSND_COMMAND_GET_MEMORY_SIZE	0xb5 /* ? 00xb5 0x41 */
#define YAWSND_COMMAND_READ_MEMORY	0xc4 /* ? 00xc4 0x41 */

#define YAWSND_CONFIG_RANGE_30_80	(1 << 0)
#define YAWSND_CONFIG_RANGE_60_110	(3 << 0)
#define YAWSND_CONFIG_RANGE_80_130	(4 << 0)
#define YAWSND_CONFIG_MIN		(1 << 3)
#define YAWSND_CONFIG_DB_C		(1 << 4)
#define YAWSND_CONFIG_MAX		(1 << 5)
#define YAWSND_CONFIG_FAST		(1 << 6)

int
main (int argc, char **argv)
{
	GError *error = NULL;
	GUsbContext *ctx = NULL;
	GUsbDevice *device = NULL;
	struct mosquitto *mqtt_client = NULL;

	mosquitto_lib_init ();

	ctx = g_usb_context_new (&error);

	mqtt_client = mosquitto_new (NULL, TRUE, NULL);

	if (error == NULL)
		device = g_usb_context_find_by_vid_pid (ctx,
							0x0483,
							0x5750,
							&error);

	if (error == NULL)
		g_usb_device_open (device, &error);

	if (error == NULL)
		g_usb_device_claim_interface (device,
					      0,
					      G_USB_DEVICE_CLAIM_INTERFACE_BIND_KERNEL_DRIVER,
					      &error);

	if (error == NULL) {
		int i;

		for (i = 0; i < 100; i++) {
			guint8 data[64];
			gsize actual_length;

			memset (data, 0, 64);
			data[0] = 0xb3;

			g_usb_device_interrupt_transfer (device, 0x01, data, 64, &actual_length, 1000, NULL, &error);

			memset (data, 0, 64);
			g_usb_device_interrupt_transfer (device, 0x81, data, 64, &actual_length, 1000, NULL, &error);

			printf ("Success %g 0x%02x\n", GUINT16_FROM_BE (*((guint16 *) data)) / 10.0,
				data[2]);
		}
		g_object_unref (device);
	}

	if (error != NULL)
		printf ("Failed: %s\n", error->message);

	g_clear_object (&device);
	g_clear_object (&ctx);

	mosquitto_destroy (mqtt_client);

	mosquitto_lib_cleanup ();

	return EXIT_SUCCESS;
}
