/* -*- c++ -*- */
/* 
 * Copyright 2016 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_ADSB_SEND_ADSBGEN_IMPL_H
#define INCLUDED_ADSB_SEND_ADSBGEN_IMPL_H

#include <adsb_send/adsbgen.h>

#include <openssl/hmac.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <algorithm>
#include "crypto.h"


using namespace std;


namespace gr {
  namespace adsb_send {

    class adsbgen_impl : public adsbgen
    {
     private:
      time_t last_packet_time;
      time_t last_signature_time;
      time_t last_key_time;
      std::ifstream myfile;
      double difftime(clock_t t1, clock_t t2);
      void sendBits(int first_byte, unsigned char * data, int byte_size, float* out);
      float AMP;
      int DELAY;
      float preamble_bits[16];
      unsigned char root_key[32];
      unsigned char addr[3];
      unsigned char keys[NUMKEYS][32];
      unsigned char signature[39+SIGLENGTH];
      int key_num;
      unsigned char count;
      int start_day;


     public:
      adsbgen_impl(char* file, float amp, int delay);
      ~adsbgen_impl();

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace adsb_send
} // namespace gr

#endif /* INCLUDED_ADSB_SEND_ADSBGEN_IMPL_H */

