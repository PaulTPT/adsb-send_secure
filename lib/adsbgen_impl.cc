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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "adsbgen_impl.h"

namespace gr {
  namespace adsb_send {

    adsbgen::sptr
    adsbgen::make(char* file, float amp, int delay)
    {
      return gnuradio::get_initial_sptr
        (new adsbgen_impl(file, amp, delay));
    }

    /*
     * The private constructor
     */
    adsbgen_impl::adsbgen_impl(char* file, float amp, int delay)
      : gr::sync_block("adsbgen",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(1, 1, sizeof(float))),
              AMP(amp), DELAY(delay), preamble_bits{amp, 0, amp, 0, 0, 0, 0, amp, 0, amp, 0, 0, 0, 0, 0, 0}, root_key{0x88, 0x08, 0x21, 0x26, 0xb1, 0x08, 0xc1, 0xc1, 0x00, 0xd4, 0x58, 0xc3, 0xa8,\
     0x25, 0x63, 0x36, 0xde, 0xb2, 0x32, 0x88, 0xf6, 0x95, 0x4e, 0xa7, 0x4e, 0xf1, 0x3c, 0x33, 0x4e, 0x70, 0x50, 0x8f}, addr{0xc0, 0x3c, 0x04}
    {   last_packet_time=0;
        last_signature_time=0;


        myfile.open(file);

        key_num=1;
        count=0;
        unsigned char last_key[32];
        Crypto::gen_keys(root_key,keys,NUMKEYS,last_key);

        time_t t;
        time(&t);
        start_day = t/86400; //86400 seconds in a day
        Crypto::gen_signature(addr,start_day %2,last_key,signature);

        last_key_time= (t/KEY_FREQ)*KEY_FREQ;

    }

    /*
     * Our virtual destructor.
     */
    adsbgen_impl::~adsbgen_impl()
    {
        myfile.close();
    }

    double adsbgen_impl::difftime(clock_t t1, clock_t t2){
        double t1_d = (double) t1;
        double t2_d = (double) t2;

        return (double) (t1-t2)/CLOCKS_PER_SEC*1000;

    }

     void adsbgen_impl::sendBits(int first_byte, unsigned char * data, int byte_size, float* out){

         for (int j=0;j<byte_size;j++ ) {
             for (int b = 0; b < 8; b++) {
                 int position=2*(first_byte*8 + 8*j+b);
                 int result = (data[j] >> (7-b)) & 1U; // Bit by bit
                 if (result == 0) {
                     out[position] = 0.0;
                     out[position+1] = AMP;
                 } else if (result == 1) {
                     out[position] = AMP;
                     out[position+1] = 0.0;
                 }
             }
         }
     }

    int
    adsbgen_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
                float *out = (float *) output_items[0];
                //GR_LOG_DEBUG(d_logger, "Work function");
                time_t now;

                for(int i =0; i < noutput_items; i++){
                out[i] = 0;
                }

                time(&now);
                key_num=(difftime(now,start_day*86400)*1000/KEY_FREQ);
                //std::cout << "key_num :" << key_num << " ,difftime : " << difftime(now,start_day*86400)*1000 <<std::endl;



                //Envoi de la signature
                if ((last_signature_time==0 || difftime(now,last_signature_time)*1000>=SIGN_FREQ) && noutput_items > 2*8*(39+SIGLENGTH)){
                    time(&last_signature_time);
                    GR_LOG_INFO(d_logger, "Sending signature ...");
                    memcpy(out,preamble_bits,16*sizeof(float));
                    sendBits(1, signature, 39+SIGLENGTH, out);
                    GR_LOG_INFO(d_logger, "Signature sent");
                }
                //Envoi de la dernière clé
                else if(difftime(now,last_key_time)*1000>=KEY_FREQ && noutput_items > 2*8*39){
                    last_key_time=(now/KEY_FREQ)*KEY_FREQ;
                    GR_LOG_INFO(d_logger, "Sending last key ...");
                    unsigned char key_message[39];
                    Crypto::publish_key(addr, keys[key_num-1],key_message);
                    memcpy(out,preamble_bits,16*sizeof(float));
                    sendBits(1, key_message, 39, out);
                    GR_LOG_INFO(d_logger, "Key sent");
                    count=0;
                }
                //Envoi du paquet ADS-B
                else if (difftime(now,last_packet_time)*1000*1000>=DELAY){

                    if (noutput_items<240+HASH_LEGNTH*2)
                        return 0;

                    unsigned char message[15]={0};

                    string line;

                    while (true) {

                        if (myfile.eof()){
                            myfile.clear();
                            myfile.seekg(0, ios::beg);
                        }

                        getline (myfile,line);

                        // Début du paquet. 112 bits, 14 bytes
                        if (line.find("0b")==0) {
                            int i=0;
                            for ( string::iterator it=(line.begin()+2); it!=line.end(); ++it){
                                unsigned char bit = *it;
                                if (bit=='1')
                                    message[i/8]= message[i/8] | (1U<<(7-i%8));
                                ++i;
                            }
                            message[14]=count;
                            // 1 byte
                            memcpy(out,preamble_bits,16*sizeof(float));
                            sendBits(1,message,15, out);

                            GR_LOG_INFO(d_logger, "Packet ready. Generating HMAC ...");
                            unsigned char* digest;
                            std::cout << "key :" ;
                            for (int m=0; m<32; m++){
                                std::cout << std::hex << std::setw(2) << std::setfill('0') << unsigned(keys[key_num][m]);
                            }
                            std::cout << std::endl;
                            digest=HMAC(EVP_sha256(),keys[key_num],32,message,15,NULL,NULL);
                            std::cout << "Digest : ";
                            for (int m=0; m<8; m++){
                                std::cout <<  std::hex << std::setw(2) << std::setfill('0') << unsigned(digest[m]);
                            }
                            std::cout << std::endl;
                            sendBits(16,digest,HASH_LEGNTH/8, out);
                            GR_LOG_INFO(d_logger, "Packet sent");
                            time(&last_packet_time);
                            count++;
                            break;
                        }
                    }
                }
                return noutput_items;

    }

  } /* namespace adsb_send */
} /* namespace gr */

