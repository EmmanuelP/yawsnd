#include <yawsw524.h>
#include <gusb.h>

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

#define YAWSND_USB_BUFFER_SIZE          64

struct _YawSw524
{
  GObject parent_instance;

  GUsbContext *context;
  GUsbDevice *device;

  guint8 config;

  GError *init_error;
};

static void yaw_sw524_initable_iface_init (GInitableIface *iface);

G_DEFINE_TYPE_WITH_CODE (YawSw524, yaw_sw524, G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE, yaw_sw524_initable_iface_init))

static gboolean
_do_command (YawSw524 *self, guint8 command, guint8 value, guint8 *buffer, gsize size, GError **error)
{
	GError *local_error = NULL;
	guint8 data[YAWSND_USB_BUFFER_SIZE];
	gsize actual_length;

        if (buffer != NULL)
                memset (buffer, 0, size);

        g_assert (size <= YAWSND_USB_BUFFER_SIZE);

	memset (data, 0, YAWSND_USB_BUFFER_SIZE);
	data[0] = command;
        data[1] = value;

	g_usb_device_interrupt_transfer (self->device, 0x01, data, YAWSND_USB_BUFFER_SIZE, &actual_length,
                                         1000, NULL, &local_error);
	if (local_error != NULL) {
		g_propagate_error (error, local_error);
		return FALSE;
	}

        if (buffer != NULL && size > 0) {
                memset (data, 0, YAWSND_USB_BUFFER_SIZE);
                g_usb_device_interrupt_transfer (self->device, 0x81, data, YAWSND_USB_BUFFER_SIZE, &actual_length,
                                                 1000, NULL, &local_error);
                if (local_error != NULL) {
                        g_propagate_error (error, local_error);
                        return FALSE;
                }
                memcpy (buffer, data, size);
        }

        return TRUE;
}

double
yaw_sw524_read_value (YawSw524 *self, YawUnit *unit, GError **error)
{
	GError *local_error = NULL;
	guint8 data[4];

	g_return_val_if_fail (YAW_IS_SW524 (self), 0.0);

        _do_command (self, YAWSND_COMMAND_READ_CURRENT, 0, data, 4, &local_error);
	if (local_error != NULL) {
		g_propagate_error (error, local_error);
		return 0.0;
	}

	if (unit != NULL)
		*unit = (data[2] & YAWSND_CONFIG_DB_C) != 0 ? YAW_UNIT_DB_C : YAW_UNIT_DB_A;

	return GUINT16_FROM_BE (*((guint16 *) data)) / 10.0;
}

gboolean
yaw_sw524_set_unit (YawSw524 *self, YawUnit unit, GError **error)
{
	GError *local_error = NULL;
        guint8 value;

	g_return_val_if_fail (YAW_IS_SW524 (self), 0.0);

        if (unit == YAW_UNIT_DB_C)
                value = self->config | YAWSND_CONFIG_DB_C;
        else
                value = self->config & (~YAWSND_CONFIG_DB_C);

        _do_command (self, YAWSND_COMMAND_CONFIG, value, NULL, 0, &local_error);
	if (local_error != NULL) {
		g_propagate_error (error, local_error);
		return FALSE;
	}

        return TRUE;
}

YawSw524 *
yaw_sw524_new (GError **error)
{
	return g_initable_new (YAW_TYPE_SW524, NULL, error,
			       NULL);
}

static void
yaw_sw524_take_init_error (YawSw524 *self, GError *error)
{

	g_return_if_fail (YAW_IS_SW524 (self));

	g_clear_error (&self->init_error);
	self->init_error = error;
}

static void
yaw_sw524_constructed (GObject *obj)
{
	YawSw524 *self = YAW_SW524 (obj);
	GError *error = NULL;

        self->config = 0;
	self->context = g_usb_context_new (&error);

	if (error == NULL)
		self->device = g_usb_context_find_by_vid_pid (self->context, 0x0483, 0x5750, &error);

	if (error == NULL)
		g_usb_device_open (self->device, &error);

	if (error == NULL)
		g_usb_device_claim_interface (self->device, 0, G_USB_DEVICE_CLAIM_INTERFACE_BIND_KERNEL_DRIVER, &error);

	if (error != NULL) {
		yaw_sw524_take_init_error (self, error);
		return;
	}

	G_OBJECT_CLASS (yaw_sw524_parent_class)->constructed (obj);
}

static void
yaw_sw524_finalize (GObject *obj)
{
	YawSw524 *self = YAW_SW524 (obj);

	g_clear_object (&self->device);
	g_clear_object (&self->context);

	G_OBJECT_CLASS (yaw_sw524_parent_class)->finalize (obj);
}


static void
yaw_sw524_class_init (YawSw524Class *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = yaw_sw524_constructed;
  object_class->finalize = yaw_sw524_finalize;
}

static void
yaw_sw524_init (YawSw524 *self)
{
}

static gboolean
yaw_sw524_initable_init (GInitable     *initable,
			 GCancellable  *cancellable,
			 GError       **error)
{
	YawSw524 *self = YAW_SW524 (initable);

	g_return_val_if_fail (YAW_IS_SW524 (initable), FALSE);

	if (cancellable != NULL)
	{
		g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
				     "Cancellable initialization not supported");
		return FALSE;
	}

	if (self->init_error) {
		if (error != NULL)
			*error = g_error_copy (self->init_error);
		return FALSE;
	}

	return TRUE;
}

static void
yaw_sw524_initable_iface_init (GInitableIface *iface)
{
	iface->init = yaw_sw524_initable_init;
}

