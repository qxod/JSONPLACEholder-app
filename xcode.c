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
    
    while ((siz