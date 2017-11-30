#!/usr/bin/env python
from __future__ import print_function
'''
====================
Author: Jesus Galaz - nov/2017, Last update: nov/2017
====================

# This software is issued under a joint BSD/GNU license. You may use the
# source code in this file under either license. However, note that the
# complete EMAN2 and SPARX software packages have some GPL dependencies,
# so you are responsible for compliance with the licenses of these packages
# if you opt to use BSD licensing. The warranty disclaimer below holds
# in either instance.
#
# This complete copyright notice must be included in any revised version of the
# source code. Additional authorship citations may be added, but existing
# author citations must be preserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  2111-1307 USA
'''

#from optparse import OptionParser
from EMAN2 import *
from EMAN2jsondb import JSTask,jsonclasses
from EMAN2_utils import *


def main():

	usage = """e2spt_translate.py file <options>.
	This program translationally aligns a stack of images to a reference.
	"""
			
	parser = EMArgumentParser(usage=usage,version=EMANVERSION)

	parser.add_argument("--input", type=str, default=None, help="""default=None. stack of image in .hdf format, presumably raw, without any alignment.""")

	parser.add_argument("--ref", type=str, default=None, help="""default=None. volume to be used as a reference.""")
	
	parser.add_argument("--path",type=str, default='spttranslate',help="""Directory to store results in. The default is a numbered series of directories containing the prefix 'spttranslate'; for example, spttranslate_02 will be the directory by default if 'spttranslate_01' already exists.""")
	
	parser.add_argument("--ppid", type=int, default=-1, help="set the PID of the parent process, used for cross platform PPID.")
	
	parser.add_argument("--intonly", action='store_true', default=False, help="default=Flase. If on, this will allow integer translations only.")

	parser.add_argument("--masked", action='store_true', default=False, help="default=False. treat zero pixels in --ref as a mask for normalization.")
	
	parser.add_argument("--maxshift", type=int, default=0, help="default=0 (not used). Maximum allowable translation in pixels.")
	
	parser.add_argument("--nozero", action='store_true', default=False, help="default=False. Zero translations not permitted (useful for CCD images)")
	
	parser.add_argument("--subset",type=int,default=0, help="""default=0 (not used). Subset of particles to align translationally.""")

	parser.add_argument("--useflcf", action='store_true', default=False, help="default=False. Use Fast Local Correlation Function rather than CCF.")

	parser.add_argument("--verbose", "-v", type=int, default=0, help="verbose level [0-9], higner number means higher level of verboseness.", dest="verbose", action="store", metavar="n")
		
	(options, args) = parser.parse_args()	

	logger = E2init(sys.argv, options.ppid)

	if options.verbose:
		print('\nlogging')

	alignerparams = {}
	
	if options.intonly:
		alignerparams.update({"intonly":1})
	
	if options.masked:
		alignerparams.update({"masked":1})
	
	if options.maxshift:
		alignerparams.update({"maxshift":options.maxshift})
	
	if options.nozero:
		alignerparams.update({"nozero":1})
	
	if options.useflcf:
		alignerparams.update({"useflcf":1})
	
	options = makepath(options,'spttranslate')

	if options.verbose:
		print('\ncreated path')

	n = EMUtil.get_image_count(options.input)
	if options.subset:
		n = options.subset

	ref = EMData(options.ref,0)
	
	jsAliParamsPath = options.path + '/sptali_trans.json'
	jsA = js_open_dict( jsAliParamsPath )
	
	basename = os.path.basename(options.input)
	stem,extension = os.path.splitext(basename)
	outfile = options.path + '/' + stem + '_translated.hdf'

	boxsize = EMData(options.input,0,True)['nx']

	for i in xrange(0,n):
		img = EMData(options.input,i)
		print('\nread iamge {}'.format(i))

		ccf = ref.calc_ccf(img)
		print('\ncalculated cccf')

		ccf.process_inplace("xform.phaseorigin.tocorner")
		print('\nto corner')

		ccf.process_inplace('normalize')
		print('\nnormalized')

		#box = ccf.get_zsize()
		#r =  Region((box/2) - options.maxshift, (box/2)-options.maxshift, (box/2)-options.maxshift, 2*options.maxshift+1, 2*options.maxshift+1, 2*options.maxshift+1)
		#sub_ccf = ccf.get_clip(r)

		if options.maxshift:
			print("\n!!!!")
			#ccf = ccf.process('mask.sharp',{'outer_radius':options.maxshift})
			masklength = boxsize - options.maxshift

			print('\nboxsize={}, options.maskshift={}, therefore masklength={}'.format(boxsize,options.maxshift,masklength))
			ccf = ccf.process('mask.zeroedge3d',{'x0':masklength,'x1':masklength,'y0':masklength,'y1':masklength,'z0':masklength,'z1':masklength})
		else:
			print('\nno options.maskshift={}'.format(options.maxshift))

		loc = ccf.calc_max_location()
		
		tx = loc[0]
		ty = loc[1]
		tz = loc[2]

		score = ccf.get_value_at(loc[0],loc[1],loc[2])

		#ali = img.align('translational',ref,alignerparams)
	
		if options.verbose:
			#print('\nali={}'.format(ali))
			print('\naligned particle {}'.format(i))

		xformslabel = 'subtomo_' + str( 0 ).zfill( len( str( n ) ) )
		#t = ali['xform.align3d']
		t = Transform({'type':'eman','tx':tx,'ty':ty,'tz':tz})
		#score = ali['score']

		jsA.setval( xformslabel, [ t , score ] )
		
		imgt = img.copy() 
		imgt.transform(t)
		imgt['xform.align3d'] = t
		imgt = origin2zero(imgt)

		#imgt.write_image(outfile,i)
		imgt.write_image(outfile,i)


	#jsA.close()
	
	E2end(logger)

	return


if __name__ == '__main__':
	
	main()
