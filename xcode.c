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
    