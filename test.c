/*
 * - create client
 * - list output sinks
 * - list all sink sources
 * - display output select and get user response
 * - use `pa_context_move_sink_input_by_name` to move all sink sources to new sink
 * - use `pa_context_set_default_sink` to set sink for new streams
 */

/* extensions
 * - remember last selected output, show that it is "current"
 */

#include <stdio.h>
#include <unistd.h>

#include <pulse/pulseaudio.h>
#include <pulse/error.h>

#define GOT_SINKS 0x1
#define GOT_SINK_INPUTS 0x2
int ready = 0;

// use pa_context_get_sink_info_list() to get this called
void sink_callback(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
    if (!i) {
        ready |= GOT_SINKS;
        return;
    }
    printf("s %d: %s, '%s'\n", i->index, i->name, i->description);
}

// use pa_context_get_sink_input_list() to get all sink inputs
void sink_input_callback(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata) {
    if (!i) {
        ready |= GOT_SINK_INPUTS;
        return;
    }
    printf("si %d: %s -> %d\n", i->index, i->name, i->sink);
}

void context_state_callback(pa_context *c, void *userdata) {
    printf("Context is ");
    switch(pa_context_get_state(c)) {
    case PA_CONTEXT_UNCONNECTED:
        printf("not connected\n");
        break;
    case PA_CONTEXT_CONNECTING:
        printf("still connecting\n");
        break;
    case PA_CONTEXT_AUTHORIZING:
        printf("authorizing\n");
        break;
    case PA_CONTEXT_SETTING_NAME:
        printf("setting name\n");
        break;
    case PA_CONTEXT_READY:
        {
            printf("ready\n");
            pa_operation *o1 = pa_context_get_sink_info_list(c, sink_callback, userdata);
            if (!o1) printf("failed to get sink info\n");
            pa_operation *o2 = pa_context_get_sink_input_info_list(c, sink_input_callback, userdata);
            if (!o2) printf("failed to get sink info\n");
        }
        break;
    case PA_CONTEXT_FAILED:
        printf("failed\n");
        break;
    case PA_CONTEXT_TERMINATED:
        printf("terminated\n");
        break;
    }
}

int main(int argc, char **argv) {
    pa_context *context;
    pa_mainloop_api *api;
    pa_mainloop *ml;

    ml = pa_mainloop_new();

    api = pa_mainloop_get_api(ml);

    pa_proplist *proplist = pa_proplist_new();
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, "PulseAudio Output Select");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, "com.davjgardner.paoutput");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ICON_NAME, "audio-card");

    context = pa_context_new_with_proplist(api, "paoutput", proplist);
    assert(context);

    pa_proplist_free(proplist);

    pa_context_set_state_callback(context, context_state_callback, ml);

    if (pa_context_connect(context, NULL, PA_CONTEXT_NOFAIL, NULL) < 0) {
        printf("Connection to PulseAudio Server failed.\n");
        exit(1);
    }

    int r;
    while (ready != (GOT_SINKS | GOT_SINK_INPUTS))
        pa_mainloop_iterate(ml, 0, &r);
    printf("%d\n", r);
    //pa_mainloop_run(ml, &r);

}
