FatFs Module Source Files R0.10a                      (C)ChaN, 2014


FILES

  ffconf.h   Configuration file for FatFs module.
  ff.h       Common include file for FatFs and application module.
  ff.c       FatFs module.
  diskio.h   Common include file for FatFs and disk I/O module.
  diskio.c   An example of glue function to attach existing disk I/O module to FatFs.
  integer.h  Integer type definitions for FatFs.
  option     Optional external functions.

  Low level disk I/O module is not included in this archive because the FatFs
  module is only a generic file system layer and not depend on any specific
  storage device. You have to provide a low level disk I/O module that written
  to control your storage device.

官方文档链接:
http://elm-chan.org/fsw/ff/00index_e.html
参考资料:
https://gitee.com/lishutong-01ketang/fatfs/tree/master

1. FATFS 的移植:

2.FATFS 多线程支持:
使用最新的版本,  ffsystem.c 中实现互斥量相关接口即可;