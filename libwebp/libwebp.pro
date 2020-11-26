TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = \
  webp.pro \
#  demux.pro # it's built in webp, if it's really needed, just copy webp.lib to webpdemux.lib
