/* paoutput.c
 *
 * PulseAudio output switcher, intended for use with dmenu.
 *
 * Usage: paoutput <-g | -s [new_sink]>
 * Options:
 * -g: Get a newline separated list of available sinks
 * -s [new_sink]: Set all active streams to use the new sink.
 *                If new_sink is not given, it will be read from stdin.
 */

#include <stdio.h>
#include <string.h>

#include<pulse/pulseaudio.h>


char *usage_string =
    "Usage: paoutput <-g | -s [new_sink]>\n"    \
    "Options:\n"\
    "-g: Get a newline separated list of available sinks\n"\
    "-s [new_sink]: Set all active streams to use the new sink.\n"\
    "               If new_sink is not given, it will be read from stdin.\n";

enum action {GET, SET, SET_STDIN};

#define N_SINKS 10
#define N_SINK_INPUTS 30
int sink_end = 0;
int sink_in_end = 0;
pa_sink_info sinks[N_SINKS];
pa_sink_input_info sink_inputs[N_SINK_INPUTS];

enum action action;
int sink_index;

int done = 0;

#define DONE 3
#define DEFAULT_SET 1
#define SINKS_MOVED 2

void context_success_callback(pa_context *c, int success, void *userdata) {
    if (!success) dprintf(2, "Failed to move sink input %d to new sink.\n", (int) userdata);
}

void sink_context_success_callback(pa_context *c, int success, void *userdata) {
    if (!success) dprintf(2, "Failed to set default sink.\n");
    done |= DEFAULT_SET;
}

void sink_callback(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
    if (action == GET) {
        if (sink_end != N_SINKS && i != NULL) {
            memcpy(&sinks[sink_end], i, sizeof(pa_sink_info));
            sink_end++;
        }
        if (i == NULL) {
            for (int i = 0; i < sink_end; i++) {
                printf("%d: %s\n", sinks[i].index, sinks[i].description);
            }
            done = DONE;
            return;
        }
    }
    if (action != GET && i != NULL) {
        pa_operation *o = pa_context_set_default_sink(c, i->name, sink_context_success_callback, userdata);
        if (!o) dprintf(2, "Failed to set default sink.\n");
    }

}

void sink_input_callback(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata) {
    if (i != NULL) {
        pa_operation *o = pa_context_move_sink_input_by_index(c, i->index, sink_index,
                                                              context_success_callback,
                                                              (void*) (i->index));
        if (!o) dprintf(2, "Failed to move sink input %d: %s\n", i->index, i->name);
    } else {
        done |= SINKS_MOVED;
    }
}

void context_state_callback(pa_context *c, void *userdata) {
    switch(pa_context_get_state(c)) {
    case PA_CONTEXT_UNCONNECTED:
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
        break;
    case PA_CONTEXT_READY:
        {
            if (action == GET) {
                pa_operation *o = pa_context_get_sink_info_list(c, sink_callback, userdata);
                if (!o) dprintf(2, "Failed to get sink info.\n");
            } else {
                pa_operation *o = pa_context_get_sink_info_by_index(c, sink_index, sink_callback, userdata);
                if (!o) dprintf(2, "Failed to get sink info.\n");
                o = pa_context_get_sink_input_info_list(c, sink_input_callback, userdata);
                if (!o) dprintf(2, "Failed to get sink input info.\n");
            }
        }
        break;
    case PA_CONTEXT_FAILED:
        dprintf(2, "Failed to connect to PulseAudio server.\n");
        break;
    case PA_CONTEXT_TERMINATED:
        dprintf(2, "Connection to PulseAudio server terminated.\n");
        break;
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("%s", usage_string);
        return 0;
    }

    if (strncmp(argv[1], "-g", 2) == 0) {
        action = GET;
    } else if (strncmp(argv[1], "-s", 2) == 0) {
        action = (argc == 2)? SET_STDIN : SET;
        if (action == SET) {
            sink_index = strtol(argv[2], NULL, 10);
        } else {
            scanf("%d", &sink_index);
        }
    } else {
        printf("%s", usage_string);
        return 0;
    }

    pa_context *context;
    pa_mainloop *ml;
    pa_mainloop_api *api;

    ml = pa_mainloop_new();
    api = pa_mainloop_get_api(ml);

    context = pa_context_new(api, "PulseAudio Output Select");
    if (!context) {
        dprintf(2, "Failed to create PulseAudio context.\n");
        exit(1);
    }

    pa_context_set_state_callback(context, context_state_callback, ml);

    if (pa_context_connect(context, NULL, PA_CONTEXT_NOFAIL, NULL) < 0) {
        dprintf(2, "Connection to PulseAudio Server failed.\n");
        exit(1);
    }

    int r;
    while (done != DONE) {
        pa_mainloop_iterate(ml, 0, &r);
    }
}
