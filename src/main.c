#include <stdlib.h>
#include <stdio.h>
#include <glib-object.h>
#include <mosquitto.h>
#include <yawsw524.h>

typedef struct {
        GMainLoop *main_loop;
        YawSw524 *sound_meter;
        struct mosquitto *mqtt_client;
} ApplicationData;

static gboolean
_read_value (gpointer user_data)
{
        ApplicationData *app = user_data;
	GError *error = NULL;
        double value;
        YawUnit unit;

        yaw_sw524_set_unit (app->sound_meter, YAW_UNIT_DB_A, &error);
        if (error == NULL)
                value = yaw_sw524_read_value (app->sound_meter, &unit, &error);

        if (error == NULL) {
                char *payload;

                payload = g_strdup_printf ("%.1f", value);
                mosquitto_publish (app->mqtt_client, NULL, "soundmeter", strlen (payload) + 1, payload, 0, true);
                printf ("%.1f %s\n", value, unit == YAW_UNIT_DB_C ? "dbC" : "dbA");
                g_free (payload);
                return TRUE;
        }

        printf ("Failed: %s\n", error->message);

        g_main_loop_quit (app->main_loop);

        return FALSE;
}

int
main (int argc, char **argv)
{
	GError *error = NULL;
        ApplicationData app;

	mosquitto_lib_init ();

	app.sound_meter = yaw_sw524_new (&error);
        app.mqtt_client = mosquitto_new (NULL, TRUE, NULL);

        mosquitto_connect (app.mqtt_client, "whydah.local", 1883, 5);

	if (error != NULL) {
		printf ("Failed: %s\n", error->message);
        } else {
                app.main_loop = g_main_loop_new (NULL, FALSE);
                g_timeout_add_seconds (1, _read_value, &app);

                g_main_loop_run (app.main_loop);
                g_main_loop_unref (app.main_loop);
        }

	g_clear_object (&app.sound_meter);
	mosquitto_destroy (app.mqtt_client);

	mosquitto_lib_cleanup ();

	return EXIT_SUCCESS;
}
