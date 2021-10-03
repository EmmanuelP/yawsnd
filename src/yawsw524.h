#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define YAW_TYPE_SW524 yaw_sw524_get_type ()
G_DECLARE_FINAL_TYPE (YawSw524, yaw_sw524, YAW, SW524, GObject)

YawSw524 * 		yaw_sw524_new 			(GError **error);

double			yaw_sw524_read_value		(YawSw524 *sonometer, GError **error);

G_END_DECLS
