G_SLICE=always-malloc G_DEBUG=gc-friendly valgrind --tool=memcheck --leak-check=full --leak-resolution=high --num-callers=50 --gen-suppressions=all --log-file=vgdump --suppressions=/root/sp $1
