/* -*- c++ -*- */

#define ADSB_SEND_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "adsb_send_swig_doc.i"

%{
#include "adsb_send/adsbgen.h"
%}


%include "adsb_send/adsbgen.h"
GR_SWIG_BLOCK_MAGIC2(adsb_send, adsbgen);
