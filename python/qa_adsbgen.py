#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2016 <+YOU OR YOUR COMPANY+>.
# 
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
# 
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
# 

from gnuradio import gr, gr_unittest
from gnuradio import blocks
import adsb_send_swig as adsb_send

class qa_adsbgen (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        src=adsb_send.siggen('/home/paultpt/Dropbox/Polymtl/Recherche DDS/usrp/ADS-B_1.log',0.5,500)
        #expected_result=[1,0,0,0,1,1,0,1,0,0,1,1,1,1,0,0,0,1,1,0,0,1,0,1,0,1,1,0,1,1,0,0,1,0,0,1,1,0,0,1,0,0,0,1,0,1,0,0,0,1,1,1,1,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1,0,1,0,1,0,1,1,0,0,0,0,1,0,0,0,1,1,0,0,1]
        dst = blocks.vector_sink_f()
        self.tb.connect(src,dst)
        self.tb.run ()
        result_data = dst.data()
        #expected_result=[x*0.5 for x in expected_result]
        print result_data
        #self.assertFloatTuplesAlmostEqual(result_data,expected_result)



if __name__ == '__main__':
    gr_unittest.run(qa_adsbgen, "qa_adsbgen.xml")
