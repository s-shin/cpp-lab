# First Non-Blocking I/O

```
$ ../bin/why-uv-001
# wait stdin

$ ../bin/why-uv-001 --non_blocking
read(2) error: [35] Resource temporarily unavailable

$ grep 35 /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk/usr/include/sys/errno.h
#define	EAGAIN		35		/* Resource temporarily unavailable */
```
