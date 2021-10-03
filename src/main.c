#include <stdlib.h>
#include <stdio.h>
#include <glib-object.h>
#include <mosquitto.h>
#include <yawsw524.h>

int
main (int argc, char **argv)
{
	GError *error = NULL;
	struct mosquitto *mqtt_client = NULL;
	YawSw524 *sonometer;

	mosquitto_lib_init ();

	sonometer = yaw_sw524_new (&error);

	mqtt_client = mosquitto_new (NULL, TRUE, NULL);

	if (error == NULL) {
		int i;

		for (i = 0; i < 100; i++) {
			double value;
			YawUnit unit;

			value = yaw_sw524_read_value (sonometer, &unit, &error);

			printf ("Success %g %s\n", value, unit == YAW_UNIT_DB_C ? "dbC" : "dbA");
		}
	}

	if (error != NULL)
		printf ("Failed: %s\n", error->message);

	g_clear_object (&sonometer);

	mosquitto_destroy (mqtt_client);

	mosquitto_lib_cleanup ();

	return EXIT_SUCCESS;
}
