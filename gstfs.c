
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
{
    pthread_mutex_t cache_mutex; /* protects file_cache, cache_lru accesses */
    GHashTable *file_cache;      /* cache of transcoded audio */
    GQueue *cache_lru;           /* queue of items in LRU order */
    int max_cache_entries;       /* max # of entries in the cache */
    char *src_mnt;               /* directory we are mirroring */
    char *src_ext;               /* extension of files we transcode */
    char *dst_ext;               /* extension of target files */
    char *pipeline;              /* gstreamer pipeline */
};

/* This stuff is stored into file_cache by filename */
struct gstfs_file_info
{
    char *filename;           /* hash key */
    char *src_filename;       /* filename in other mount */