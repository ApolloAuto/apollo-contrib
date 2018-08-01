This repository contains code/binary contributed by Baidu to the Apollo Autonomous Driving Project that can't or is not suitable to be distributed under Apache license.

Directory structure:

- bin/: binary programs
- docs/: documentations
- drivers/: driver source code
- include/: header files for user space library/application development
  - include/baidu/: library headers
  - uapi/: header files for writing user space program against kernel driver(s)
- lib/: binary library files (.a, .so)
- linux/: binary release of kernel driver(s)
  - linux/drivers: binary drivers organized by kernel version
  - linux/include: header files for writing user space program against the binary driver
