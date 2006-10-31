/**
 * $Id$
 */
 
/*
 * Author: Steven Ludtke, 04/10/2003 (sludtke@bcm.edu)
 * Copyright (c) 2000-2006 Baylor College of Medicine
 * 
 * This software is issued under a joint BSD/GNU license. You may use the
 * source code in this file under either license. However, note that the
 * complete EMAN2 and SPARX software packages have some GPL dependencies,
 * so you are responsible for compliance with the licenses of these packages
 * if you opt to use BSD licensing. The warranty disclaimer below holds
 * in either instance.
 * 
 * This complete copyright notice must be included in any revised version of the
 * source code. Additional authorship citations may be added, but existing
 * author citations must be preserved.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * 
 * */
 
#ifndef eman__emdata_h__
#define eman__emdata_h__ 1

#include <cfloat>
#include <fstream>

#include "sparx/fundamentals.h"
#include "emutil.h"
#include "util.h"
#include "sparx/emarray.h"
#include "geometry.h"
#include "transform.h"

using std::string;
using std::vector;
using std::map;
//using std::complex;	//comment this out for conflict with ACML
using std::ostream;

namespace EMAN
{
	class ImageIO;
	class Ctf;
	class XYData;
	class Transform3D;
	
	typedef boost::multi_array_ref<float, 2> MArray2D;
	typedef boost::multi_array_ref<float, 3> MArray3D;
	typedef boost::multi_array_ref<std::complex<float>, 2> MCArray2D;
	typedef boost::multi_array_ref<std::complex<float>, 3> MCArray3D;
	typedef boost::multi_array<int, 2> MIArray2D;
	typedef boost::multi_array<int, 3> MIArray3D;
	
	/** @ingroup tested3c */
	/** EMData stores an image's data and defines core image processing routines.
     * The image is 1D, 2D or 3D, in real space or fourier space (complex image).
	 *
	 * Data are ordered with x increasing fastest, then y, then z.
	*/
	class EMData
	{
		/** For all image I/O */
		#include "emdata_io.h"
		
		/** For anything read/set image's information */
		#include "emdata_metadata.h"
	
		/** For modular class functions, process, align, etc. */
		#include "emdata_modular.h"
	
		/** For fft, wavelet, insert data */
		#include "emdata_transform.h"
	
		/** For get/set values, basic math operations, operators */
		#include "emdata_core.h"
	
		/** This is the header of EMData stay in sparx directory */
		#include "sparx/emdata_sparx.h"
	
		/** This is the header of EMData stay in sparx directory */
		#include "sparx/normpadft.h"
		
	static int totalalloc;
	public:
	
		
		
		enum FFTPLACE { FFT_OUT_OF_PLACE, FFT_IN_PLACE };
		enum WINDOWPLACE { WINDOW_OUT_OF_PLACE, WINDOW_IN_PLACE };
		
		/** Construct an empty EMData instance. It has no image data. */
		EMData();
		virtual ~ EMData();  

		
		/**  Do the Fourier Harmonic Transform  PRB
		 * Takes a real image, returns the FH
		 * Sets the EMDATA_FH switch to indicate that it is an FH image
		 * @exception ImageFormatException If the image is not a square real odd image.
		 * @return the FH image.
		 */
//		EMData* do_FH();
		
		/**   Do the Inverse Fourier Harmonic Transform   PRB
		 * Takes an FH image, returns a square  complex image with odd sides
		 * @exception ImageFormatException If the image is not the FH of something
		 * @return a square complex image with odd sides
		 */
//		EMData* do_FH2F();

		

		/** Caclulates normalization and phase residual for a slice in
		 * an already existing volume. phase residual is calculated
		 * using only the inner 1/2 of the fourier sphere. Both the
		 * slice image and this image must be in complex image format.
		 *
		 * @param slice An slice image to be normalized.
		 * @param orient Orientation of the slice.
		 * @exception ImageFormatException If the images are not complex.
		 * @exception ImageDimensionException If the image is 3D.
		 * @return A float number pair (result, phase-residual).
		 */
//		FloatPoint normalize_slice(EMData * slice, const Transform3D & orient);

		/** Caclulates normalization and phase residual for a slice in
		 * an already existing volume. phase residual is calculated
		 * using only the inner 1/2 of the fourier sphere. Both the
		 * slice image and this image must be in complex image format.
		 *
		 * @param slice An slice image to be normalized.
		 * @param alt Orientation euler angle alt (in EMAN convention).
		 * @param az  Orientation euler angle az  (in EMAN convention).
		 * @param phi Orientation euler angle phi (in EMAN convention).
		 * @exception ImageFormatException If the images are not complex.
		 * @exception ImageDimensionException If the image is 3D.
		 * @return A float number pair (result, phase-residual).
		 */
//		FloatPoint normalize_slice(EMData * slice, float az, float alt, float phi);


		/** Get an inclusive clip. Pads 0 if larger than this image.
		 * area can be 2D/3D.
		 * @param area The clip area.
		 * @return The clip image.
		 */
		EMData *get_clip(const Region & area);
		
		
		/** Get the top half of this 3D image.
		 * @exception ImageDimensionException If this image is not 3D.
		 * @return The top half of this image.
		 */
		EMData *get_top_half() const;
		
		
		/** This will extract an arbitrarily oriented and sized region from the
		 *  image. 
		 *
		 *  @param xform The transformation of the region.
		 *  @param size Size of the clip.
		 *  @param scale Scaling put on the returned image.
		 *  @return The clip image.
		 */ 
		EMData *get_rotated_clip(const Transform3D & xform, const IntSize &size, float scale=1.0);

		/** Window the center of an image.
		 *  Often an image is padded with zeros for fourier interpolation.  In
		 *  that case the desired lxlxl volume (or lxl area) lies in the center
		 *  of a larger volume (or area).  This routine creates a new object
		 *  that contains only the desired window.  (This routine is a thin
		 *  wrapper around get_clip.)
		 *
		 * @param l Length of the window.
		 * @return An image object that has been windowed.
		 */
		EMData* window_center(int l);


		/** Set up for fftslice operations.
		 * When interpolating in fourier space there is a little
		 * problem when we get close to x=0, since f(-x,-y,-z) = f(x,y,z)* .
		 * So this makes a supplementary array that allows for up to +-2
		 * point interpolation all the way to the origin in x.
		 *
		 * 3D only; complex image only
		 *
		 * @param redo If true,  recalculate the supplementary array.
		 * @exception ImageFormatException If the image is not a
		 * complex image.
		 * @exception ImageDimensionException If the image is not 3D.
		 * @return The supplementary array.
		 */
		float *setup4slice(bool redo = true);
		
		
		/** scale the image by a factor.
		 * @param scale_factor scale factor.
		 */
		void scale(float scale_factor);
		
		
		/** Translate this image.
		 * @param dx Translation distance in x direction.
		 * @param dy Translation distance in y direction.
		 * @param dz Translation distance in z direction.
		 */
		void translate(float dx, float dy, float dz);
		
		
		/** Translate this image.
		 * @param translation The translation distance vector.
		 */
		void translate(const Vec3f &translation);
		
				
		/** Translate this image. integer only translation
		 *  could be done faster, without interpolation.
		 * @param dx Translation distance in x direction.
		 * @param dy Translation distance in y direction.
		 * @param dz Translation distance in z direction.
		 */
		void translate(int dx, int dy, int dz);
		
				
		/** Translate this image. integer only translation 
		 *  could be done faster, without interpolation.
		 * @param translation The translation distance vector.
		 */
		void translate(const Vec3i &translation);
		
		
		/** Rotate this image.
		 * @param t Transformation rotation.
		 */
		void rotate(const Transform3D & t);
		
		
		/** Rotate this image.
		 * @param az  Rotation euler angle az  in EMAN convention.
		 * @param alt Rotation euler angle alt in EMAN convention.		 
		 * @param phi Rotation euler angle phi in EMAN convention.
		 */
		void rotate(float az, float alt, float phi);
		
		
		/** Rotate then translate the image.
		 * @param t The rotation and translation transformation to be done.
		 */
		void rotate_translate(const Transform3D & t);
		
		
		/** Rotate then translate the image.
		 * @param alt Rotation euler angle alt in EMAN convention.
		 * @param az  Rotation euler angle az  in EMAN convention.
		 * @param phi Rotation euler angle phi in EMAN convention.
		 * @param dx Translation distance in x direction.
		 * @param dy Translation distance in y direction.
		 * @param dz Translation distance in z direction.
		 */
		void rotate_translate(float az, float alt, float phi, float dx, float dy, float dz);
		
				
		/** Rotate then translate the image.
		 * @param alt Rotation euler angle alt in EMAN convention.
		 * @param az  Rotation euler angle az  in EMAN convention.
		 * @param phi Rotation euler angle phi in EMAN convention.
		 * @param dx Translation distance in x direction.
		 * @param dy Translation distance in y direction.
		 * @param dz Translation distance in z direction.
		 * @param pdx Pretranslation distance in x direction.
		 * @param pdy Pretranslation distance in y direction.
		 * @param pdz Pretranslation distance in z direction.
		 */
		void rotate_translate(float az, float alt, float phi, float dx, float dy,
							  float dz, float pdx, float pdy, float pdz);
		
			
		/** This performs a translation of each line along x with wraparound.
		 *  This is equivalent to a rotation when performed on 'unwrapped' maps.
		 *  @param dx Translation distance align x direction.
		 *  @exception ImageDimensionException If the image is 3D.
		 */
		void rotate_x(int dx);
		
		
		/** Fast rotation by 180 degrees. Square 2D image only.
		 *  @exception ImageFormatException If the image is not square.
		 *  @exception ImageDimensionException If the image is not 2D.
		 */
		void rotate_180();
		
		
		/** dot product of 2 images. Then 'this' image is rotated/translated.
		 * It is much faster than Rotate/Translate then dot product. 
		 * 2D images only.
		 *
		 * @param with The image used to do the dot product.
		 * @param dx Translation distance in x direction.
		 * @param dy Translation distance in y direction.
		 * @param da Rotation euler angle.
		 * @exception ImageFormatException If the 2 images are not the
		 * same size.
		 * @exception ImageDimensionException If the image is 3D.
		 * @return
		 */
		double dot_rotate_translate(EMData * with, float dx, float dy, float da);
		
		
		/** This does a normalized dot product of a little image with a big image 
		 * using real-space methods. The result is the same size as 'this', 
		 * but a border 1/2 the size of 'little_img' will be zero. 
		 * This routine is only efficient when 'little_img' is fairly small.
		 * 
		 * @param little_img A small image.
		 * @param do_sigma Calculate sigma or not.
		 * @exception ImageDimensionException If the image is not 1D/2D.
		 * @return normalized dot product image.
		 */
		EMData *little_big_dot(EMData * little_img, bool do_sigma = false);
		
		
		/** Radon Transform: an algorithm that transforms an original
		 * image into a series of equiangular projections. When
		 * applied to a 2D object, the output of the Radon transform is a
		 * series of 1D lines.
		 *
		 * Do radon transformation on this image. This image must be
		 * 2D square.
		 
		 * @exception ImageFormatException If the image is not square.
		 * @exception ImageDimensionException If the image is not 2D.
		 * @return Radon transform image in square.
		 */
		EMData *do_radon();
		
		
		/** Calculate Cross-Correlation Function (CCF).
		 *
		 * Calculate the correlation of two 1-, 2-, or 3-dimensional
		 * images.  Note: this method internally just calls the
		 * correlation function from fundamentals.h.
		 *
		 * @param[in] with The image used to calculate the CCF. If 'with' is
		 * NULL, the autocorrelation function is computed instead.
		 * @param[in] fpflag Specify how periodicity (or normalization) should
		 * be handled.  See fundamentals.h  The default is "CIRCULANT".  for
		 * specific flags.
		 * @return Real-space image.
		 */
		EMData *calc_ccf(EMData * with, fp_flag fpflag = CIRCULANT);
		
		
		/** Calculate Cross-Correlation Function (CCF) in the x-direction 
		 * and adds them up, result in 1D.
		 * WARNING: this routine will modify the 'this' and 'with' to contain
		 * 1D fft's without setting some flags. This is an optimization
		 * for rotational alignment.
		 *
		 * @param with The image used to calculate CCF.
		 * @param y0 Starting position in x-direction.
		 * @param y1 Ending position in x-direction. '-1' means the
		 *        end of the row.
		 * @param nosum If true, returns an image y1-y0+1 pixels high.
		 * @see #calc_ccf()
		 * @exception NullPointerException If input image 'with' is NULL.
		 * @exception ImageFormatException If 'with' and 'this' are
		 * not same size.
		 * @exception ImageDimensionException If 'this' image is 3D.
		 * @return The result image containing the CCF.
		 */
		EMData *calc_ccfx(EMData * with, int y0 = 0, int y1 = -1, bool nosum = false);
		
		
		/** Makes a 'rotational footprint', which is an 'unwound'
		 * autocorrelation function. generally the image should be
		 * edge-normalized and masked before using this.
		 *
		 * @param unwrap RFP undergoes polar->cartesian x-form
		 * @param premasked Is the image pre-masked?
		 * @exception ImageFormatException If image size is not even.
		 * @return The rotaional footprint image.
		 */
		EMData *make_rotational_footprint(bool premasked = false, bool unwrap = true);
		
		
		/** Makes a 'footprint' for the current image. This is another
		 * image constructed from the 'rotational footprint' to produce
		 * a rotationally and translationally invariant image.
		 *
		 * @exception ImageFormatException If image size is not even.
		 * @return The footprint image.
		 */
		EMData *make_footprint();
		
		
		/** Calculates mutual correlation function (MCF) between 2 images.
		 * If 'with' is NULL, this does mirror ACF.
		 *
		 * @param with The image used to calculate MCF.
		 * @param tocorner Set whether to translate the result image
		 *        to the corner.
		 * @param filter The filter image used in calculating MCF.
		 * @exception ImageFormatException If 'with' is not NULL and
		 * it doesn't have the same size to 'this' image.
		 * @exception NullPointerException If FFT returns NULL image.
		 * @return Mutual correlation function image.
		 */
		EMData *calc_mutual_correlation(EMData * with, bool tocorner = false, EMData * filter = 0);
		
		
		/** maps polar coordinates to Cartesian coordinates. radially weighted.
		 * When used with RFP, this provides 1 pixel accuracy at 75% radius.
		 * 2D only.
		 *
		 * @param r1
		 * @param r2
		 * @param xs
		 * @param dx
		 * @param dy
		 * @param do360  If true, do 0-360 degree mapping. Otherwise,
		 * do 0-180 degree mapping.
		 * @exception ImageDimensionException If 'this' image is not 2D.
		 * @return The image in Cartesian coordinates.
		 */		 
		EMData *unwrap(int r1 = -1, int r2 = -1, int xs = -1, int dx = 0,
							   int dy = 0, bool do360 = false);
							   
		
		/** Reduces the size of the image by a factor 
		 * using the average value of the pixels in a block.
		 * @param shrink_factor Image shrink factor.
		 * @exception InvalidValueException If shrink factor is invalid.
		 */
		void mean_shrink(float shrink_factor);
		
				
		/* Reduces the size of the image by a factor using a local median processor.
		 *
		 * @param shrink_factor Image shrink factor.
		 * @exception InvalidValueException If shrink factor is invalid.
		 */
		void median_shrink(int shrink_factor);
		
		
		/** multiplies by a radial function in fourier space.
		 *
		 * @param x0  starting point x coordinate.
		 * @param dx  step of x.
		 * @param array radial function data array.
		 * @param interp Do the interpolation or not.
		 */
		void apply_radial_func(float x0, float dx, vector < float >array, bool interp = true);
		
		
		/** calculates radial distribution. works for real and imaginary images.
		 * Returns mean radial amplitude, or intensity if inten is set. Note that the complex
		 * origin is at (0,0), with periodic boundaries. Note that the inten option is NOT
		 * equivalent to returning amplitude and squaring the result.
		 * 
		 * @param n number of points.
		 * @param x0 starting point x coordinate.
		 * @param dx step of x.
		 * @param inten returns intensity (amp^2) rather than amplitude if set
		 * @return The radial distribution in an array.
		 */					
		vector < float >calc_radial_dist(int n, float x0, float dx,bool inten);
		
		
		/** calculates radial distribution subdivided by angle. works for real and imaginary images.
		 * 2-D only. The first returns a single vector of n*nwedge points, with radius varying first.
		 * That is, the first n points represent the radial profile in the first wedge.
		 * @param n number of points.
		 * @param x0 starting x coordinate.
		 * @param dx step of x.
		 * @param nwedge int number of wedges to divide the circle into
		 * @param inten returns intensity (amp^2) rather than amplitude if set
		 * @exception ImageDimensionException If 'this' image is not 2D.
		 * @return nwedge radial distributions packed into a single vector<float>
		 */
		vector < float >calc_radial_dist(int n, float x0, float dx, int nwedge,bool inten);
		

		/** Replace the image its complex conjugate.
		 * @exception ImageFormatException Image must be complex (and RI)
		 */
		void cconj();


		/** Adds 'obj' to 'this' incoherently. 'obj' and 'this' should
		 * be same size. Both images should be complex images.
		 *
		 * @param obj The image added to 'this' image.
		 * @exception ImageFormatException If the 2 images are not
		 * same size; or if the 2 images are not complex images.
		 */
		void add_incoherent(EMData * obj);


		/** Calculates the histogram of 'this' image. The result is
		 * stored in float array 'hist'. If hist_min = hist_max, use 
		 * image data min as hist_min; use image data max as hist_max.
		 *
		 * @param hist_size Histogram array's size.
		 * @param hist_min Minimum histogram value.
		 * @param hist_max Maximum histogram value.
		 * @return histogram array of this image.
		 */
		vector <float> calc_hist(int hist_size = 256, float hist_min = 0, 
								   float hist_max = 0);


		/** Caculates the azimuthal distributions.
		 * works for real or complex images, 2D only.
		 *
		 * @param n  Number of elements.
		 * @param a0 Starting angle.
		 * @param da Angle step.
		 * @param rmin Minimum radius.
		 * @param rmax  Maximum radius.
		 * @exception ImageDimensionException If image is 3D.
		 * @return Float array to store the data.
		 */
		vector<float> calc_az_dist(int n, float a0, float da, float rmin, 
								   float rmax);
								   
#if 0
		void calc_rcf(EMData * with, vector < float >&sum_array);
#endif
		/** Calculates the distance between 2 vectors. 'this' image is
		 * 1D, which contains a vector; 'second_img' may be nD. One of
		 * its row is used as the second vector. 'second_img' and
		 * 'this' must have the same x size. 
		 *
		 * @param second_img The image used to caculate the distance.
		 * @param y_index Specifies which row in 'second_img' is used to do
		 * the caculation.
		 * @exception ImageDimensionException If 'this' image is not 1D.
		 * @exception ImageFormatException If the 2 images don't have
		 * same xsize.
		 * @return The distance between 2 vectors.
		 */
		float calc_dist(EMData * second_img, int y_index = 0) const;

		/** Calculates the cross correlation with local normalization
		 * between 2 images. This is a fater version of local correlation.
		 *
		 * This is the fast local correlation program based upon local
		 * real space correlation. The traditional real-space technies
		 * are know to be senisitve for finding small objects in a large
		 * field due to local optimization of numerical
		 * scaling. However, they are slow to compute. This technique is
		 * based upon Fourier transform and claimed to be two times
		 * faster than the explicit real-space formula.
		 * 
		 * Well, it was published much earlier by Marin van Heel...
		 * The technique is published in Ultramicroscopy by Alan M. Roseman, 
		 * MRC Cambridge.
		 *
		 * Imlemented by htet khant, 02-2003
		 *
		 * @param with The image used to calculate cross correlation.
		 * @param radius
		 * @param maskfilter
		 * @return the cross correlation image.
		 */
		EMData *calc_flcf(EMData * with, int radius = 50,
						  const string & maskfilter = "eman1.mask.sharp");

		/** Convolutes 2 data sets. The 2 images must be of the same size.
		 * @param with One data set. 'this' image is the other data set.
		 * @exception NullPointerException If FFT resturns NULL image.
		 * @return The result image.
		 */
		EMData *convolute(EMData * with);

		

#if 0
		void create_ctf_map(CtfMapType type, XYData * sf = 0);
#endif

		

		/** @ingroup tested2 */
		/** Finds common lines between 2 complex images.
		 * 
		 * This function does not assume any symmetry, just blindly
		 * compute the "sinogram" and the user has to take care how
		 * to interpret the returned "sinogram". it only considers
		 * inplane rotation and assumes prefect centering and identical
		 * scale.
		 *
		 * @param image1 The first complex image.
		 * @param image2 The second complex image.
		 * @param mode Either 0 or 1 or 2. mode 0 is a summed
		 *   dot-product, larger value means better match; mode 1 is
		 *   weighted phase residual, lower value means better match.		 
		 * @param steps: 1/2 of the resolution of the map.
		 * @param horizontal In horizontal way or not.
		 * @exception NullPointerException If 'image1' or 'image2' is NULL.
		 * @exception OutofRangeException If 'mode' is invalid.
		 * @exception ImageFormatException If 'image1' 'image2' are
		 * not same size.
		 */
		void common_lines(EMData * image1, EMData * image2, int mode = 0,
						  int steps = 180, bool horizontal = false);

		/** @ingroup tested2 */
		/** Finds common lines between 2 real images.
		 *
		 * @param image1 The first image.
		 * @param image2 The second image.
		 * @param steps 1/2 of the resolution of the map.
		 * @param horizontal In horizontal way or not.
		 * @exception NullPointerException If 'image1' or 'image2' is NULL.
		 * @exception ImageFormatException If 'image1' 'image2' are not same size.
		 */
		void common_lines_real(EMData * image1, EMData * image2,
							   int steps = 180, bool horizontal = false);

		/** cut a 2D slice out of a real 3D map. Put slice into 'this' image.
		 *
		 * @param map The real 3D map.
		 * @param dz 
		 * @param orientation Orientation of the slice.
		 * @param interpolate Do interpolation or not.
		 * @param dx
		 * @param dy
		 * @exception NullPointerException If map is NULL.
		 */
		void cut_slice(const EMData * map, float dz, Transform3D * orientation = 0,
					   bool interpolate = true, float dx = 0, float dy = 0);

		/** Opposite of the cut_slice(). It will take a slice and insert
		 * the data into a real 3D map. It does not interpolate, it uses
		 * the nearest neighbor.
		 *
		 * @param map  The real 3D map.
		 * @param dz
		 * @param orientation Orientation of the slice.
		 * @param dx
		 * @param dy
		 */		 
		void uncut_slice(EMData * map, float dz, Transform3D * orientation = 0,
						 float dx = 0, float dy = 0);



	private:
		enum EMDataFlags {
//			EMDATA_COMPLEX = 1 << 1,
//			EMDATA_RI = 1 << 2,	       // real/imaginary or amp/phase
			EMDATA_BUSY = 1 << 3,	   // someone is modifying data, NO LONGER USED
			EMDATA_HASCTFF = 1 << 4,   // has CTF info in the image file
			EMDATA_NEEDUPD = 1 << 5,   // needs a real update			
//			EMDATA_COMPLEXX = 1 << 6,  // 1D fft's in X
			EMDATA_FLIP = 1 << 7,	   // is the image flipped
			EMDATA_PAD = 1 << 8,       // is the image fft padded 
			EMDATA_FFTODD = 1 << 9,	   // is the (real-space) nx odd
			EMDATA_SHUFFLE = 1 << 10,  // fft been shuffled? (so O is centered) PRB
			EMDATA_FH = 1 << 11        // is the complex image a FH image
		};

		void update_stat() const;
		void set_xyz_origin(float origin_x, float origin_y, float origin_z);
		void scale_pixel(float scale_factor) const;
		void save_byteorder_to_dict(ImageIO * imageio);
		
	private:
		/** to store all image header info */
		mutable Dict attr_dict; 
		/** image real data */
		float *rdata;	 
		/** supplementary data array */       
		float *supp;    
		
		/** CTF data 
		 * All CTF data become attribute ctf(vector<float>) in attr_dict  --Grant Tang*/        
		//Ctf *ctf;		   
		
		/** flags */  
		mutable int flags;       
		// Incremented every time the image changes
		int changecount;
		/** image size */       
		int nx, ny, nz;	        
		/** array index offsets */
		int xoff, yoff, zoff;

		/** translation from the original location */
		Vec3f all_translation; 
//		Vec3f all_rotation;    /** rotation (az, alt, phi) from the original locaton*/

		string path;
		int pathnum;
	};


	EMData * operator+(const EMData & em, float n);
	EMData * operator-(const EMData & em, float n);
	EMData * operator*(const EMData & em, float n);
	EMData * operator/(const EMData & em, float n);

	EMData * operator+(float n, const EMData & em);
	EMData * operator-(float n, const EMData & em);
	EMData * operator*(float n, const EMData & em);
	EMData * operator/(float n, const EMData & em);

	EMData * operator+(const EMData & a, const EMData & b);
	EMData * operator-(const EMData & a, const EMData & b);
	EMData * operator*(const EMData & a, const EMData & b);
	EMData * operator/(const EMData & a, const EMData & b);


	
/*   Next  is Modified by PRB      Transform3D::EMAN,
	inline Transform3D EMData::get_transform() const 
	{
		return Transform3D((float)attr_dict["euler_alt"],
				   (float)attr_dict["euler_az"],
				   (float)attr_dict["euler_phi"]);
	}
*/
	
	
	inline void EMData::scale_pixel(float scale) const
	{
		attr_dict["apix_x"] = ((float) attr_dict["apix_x"]) * scale;
		attr_dict["apix_y"] = ((float) attr_dict["apix_y"]) * scale;
		attr_dict["apix_z"] = ((float) attr_dict["apix_z"]) * scale;
	}

	
			 
	
}

			
#endif

/* vim: set ts=4 noet nospell: */
