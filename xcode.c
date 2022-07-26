/*
 * gstfs - gstreamer glue routines for transcoding
 */

#include <gst/gst.h>
#include <glib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

struct pipe_params
{
    int fd;
    void (*add_data_cb)(char *, size_t, void *);
    void *user_data;
};

void *send_pipe(void *data)
{
    struct pipe_params *param = (struct pipe_params *) data;
    char buf[PIPE_BUF];
    size_t sizeread;
    
    while ((sizeread = read(param->fd, buf, sizeof(buf))) > 0)
    {
        param->add_data_cb(buf, sizeread, param->user_data);
    }
    return NULL;
}

/*
 *  Transcodes a file into a buffer, blocking until done.
 */
int transcode(char *pipeline_str, char *filename, 
    void (*add_data_cb)(char *, size_t, void *), void *user_data)
{
    GstElement *pipeline, *source, *dest;
    GError *error = NULL;
    GstBus *bus;
    int pipefds[2];

    struct pipe_params thread_params;
    pthread_t thread;
    void *thread_status;

    pipeline = gst_parse_launch(pipeline_str, &error);
    if (error)
    {
        fprintf(stderr, "Error parsing pipeline: %s\n", error->message);
        return -1;
    }

    source = gst_bin_get_by_name(GST_BIN(pipeline), "_source");
    dest = gst_bin_get_by_name(GST_BIN(pipeline), "_dest");

    if (!pipeline || !source || !dest) 
    {
        fprintf(stderr, "Could not initialize pipeline\n");
        return -2;
    }

    if (pipe(pipef