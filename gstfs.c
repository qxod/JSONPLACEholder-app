
/*
 *  gstfs - a gstreamer filesystem
 */
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>
#include <fuse.h>
#include <errno.h>
#include <glib.h>
#include <gst/gst.h>
#include "xcode.h"

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define GSTFS_OPT_KEY(templ, elem, key) \
    { templ, offsetof(struct gstfs_mount_info, elem), key }

/* per-mount options and data structures */
struct gstfs_mount_info