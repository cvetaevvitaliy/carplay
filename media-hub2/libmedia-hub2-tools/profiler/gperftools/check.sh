rm /tmp/mh-sample.* -f
rm /var/mediahub.db -f
G_SLICE=always-malloc G_DEBUG=gc-friendly PPROF_PATH=/usr/bin/pprof LD_PRELOAD=/usr/lib/libtcmalloc.so HEAPCHECK=normal $1
