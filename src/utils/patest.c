#include <locale.h>
#include <signal.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pulse/pulseaudio.h>
#include <pulsecore/log.h>
#include <pulsecore/core-util.h>

static void quit(pa_mainloop_api *m, int ret) {
    pa_assert(m);
    m->quit(m, ret);
}

static void exit_signal_cb(pa_mainloop_api *m, pa_signal_event *e,
                                 int sig, void *userdata) {
    pa_log("Got %s, exiting ..", strsignal(sig));
    quit(m, 0);
}

static void get_server_info_cb(pa_context *c, const pa_server_info *i,
								void *userdata) {

	pa_mainloop_api *api = userdata;

	printf("User name: %s\n", i->user_name);
	printf("Host name: %s\n", i->host_name);
	printf("Server name: %s\n", i->server_name);
	printf("Default sink name: %s\n", i->default_sink_name);
	printf("Default source name: %s\n", i->default_source_name);
	printf("Cookie: %04x: %04x\n", i->cookie >> 16, i->cookie & 0xFFFFU);

	quit(api, 0);
}

static void context_state_cb(pa_context *context, void *userdata) {
	pa_operation *o = NULL;
	pa_mainloop_api *api = userdata;

	pa_assert(context);

	switch (pa_context_get_state(context)) {
		case PA_CONTEXT_CONNECTING:
		case PA_CONTEXT_AUTHORIZING:
		case PA_CONTEXT_SETTING_NAME:
			break;

		case PA_CONTEXT_READY:
			o = pa_context_get_server_info(context, get_server_info_cb, api);
			pa_operation_unref(o);
			break;

		case PA_CONTEXT_TERMINATED:
			quit(api, 0);
			break;

		case PA_CONTEXT_FAILED:
		default:
			pa_log_error("Connection failure: %s",
					pa_strerror(pa_context_errno(context)));
			quit(api, 0);
	}
}

int main(int argc, char *argv[]) {
    pa_context *context = NULL;
    pa_mainloop *mainloop = NULL;
    pa_mainloop_api *api = NULL;
    int ret = 1, mainloop_ret;

    mainloop = pa_mainloop_new();
    if (!mainloop) {
        pa_log_error("pa_mainloop_new() failed");
        goto quit;
    }

    api = pa_mainloop_get_api(mainloop);

    pa_assert_se(pa_signal_init(api) == 0);
    pa_signal_new(SIGINT, exit_signal_cb, NULL);
    pa_signal_new(SIGTERM, exit_signal_cb, NULL);
    pa_disable_sigpipe();

    context = pa_context_new(api, "patest application!");
    if (!context) {
        pa_log_error("pa_context_new() failed");
        goto quit;
    }

    pa_context_set_state_callback(context, context_state_cb, api);
    ret = pa_context_connect(context, NULL, 0, NULL);
    if ( ret < 0) {
        pa_log_error("pa_context_connect(): %s",
                pa_strerror(pa_context_errno(context)));
        goto quit;
    }

    ret = pa_mainloop_run(mainloop, &mainloop_ret);
    if (ret < 0) {
        pa_log("pa_mainloop_run() failed");
        goto quit;
    }

 quit:
    if (context)
        pa_context_unref(context);

    if (mainloop)
        pa_mainloop_free(mainloop);

    return ret;
}
