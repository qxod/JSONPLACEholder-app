
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
    pthread_mutex_t mutex;    /* protects this file info */
    bool passthru;            /* true if this is a file in the mirror */
    size_t len;               /* size of file */
    size_t alloc_len;         /* allocated size of buf */
    char *buf;                /* completely converted file */
    GList *list_node;         /* pointer for cache_lru */
};
static char *get_source_path(const char *filename);

static struct gstfs_mount_info mount_info;

void usage(const char *prog)
{
    printf("Usage: %s -o [options] mount_point\n\n"
           "where options can be:\n"
           "   src=[source directory]    (required)\n"
           "   src_ext=[mp3|ogg|...]     (required)\n"
           "   dst_ext=[mp3|ogg|...]     (required)\n"
           "   pipeline=[gst pipeline]   (required)\n"
           "   ncache=[0-9]*             (optional)\n",
           prog);
}

/*
 *  Create a new gstfs_file_info object using the specified destination file.
 */
struct gstfs_file_info *get_file_info(const char *filename)
{
    struct gstfs_file_info *fi;
    struct stat stbuf;

    fi = calloc(1, sizeof(struct gstfs_file_info));
    fi->filename = g_strdup(filename);
    fi->src_filename = get_source_path(filename);
    fi->passthru = !is_target_type(filename) ||
                   is_target_type(fi->src_filename);