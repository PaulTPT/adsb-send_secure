#!/bin/sh
export VOLK_GENERIC=1
export GR_DONT_LOAD_PREFS=1
export srcdir=/home/paultpt/Documents/gnuradio/gr-adsb_send/python
export GR_CONF_CONTROLPORT_ON=False
export PATH=/home/paultpt/Documents/gnuradio/gr-adsb_send/build/python:$PATH
export LD_LIBRARY_PATH=/home/paultpt/Documents/gnuradio/gr-adsb_send/build/lib:$LD_LIBRARY_PATH
export PYTHONPATH=/home/paultpt/Documents/gnuradio/gr-adsb_send/build/swig:$PYTHONPATH
/usr/bin/python2 /home/paultpt/Documents/gnuradio/gr-adsb_send/python/qa_adsbgen.py 
