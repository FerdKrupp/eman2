/**
 * $Id$
 */
#include "icosio.h"
#include "log.h"
#include "emutil.h"
#include "portable_fileio.h"
#include "geometry.h"

using namespace EMAN;


IcosIO::IcosIO(string file, IOMode rw)
:	filename(file), rw_mode(rw), icos_file(0), initialized(false)
{
	is_big_endian = ByteOrder::is_host_big_endian();
	is_new_file = false;
}

IcosIO::~IcosIO()
{
	if (icos_file) {
		fclose(icos_file);
		icos_file = 0;
	}
}

int IcosIO::init()
{
	ENTERFUNC;
	static int err = 0;
	if (initialized) {
		return err;
	}
	
	initialized = true;

	icos_file = sfopen(filename, rw_mode, &is_new_file);
	if (!icos_file) {
		err = 1;
		return err;
	}

	if (!is_new_file) {
		if (fread(&icosh, sizeof(IcosHeader), 1, icos_file) != 1) {
			LOGERR("cannot read ICOS file '%s'", filename.c_str());
			err = 1;
			return err;
		}

		if (!is_valid(&icosh)) {
			LOGERR("invalid ICOS file");
			err = 1;
			return err;
		}
		is_big_endian = ByteOrder::is_data_big_endian(&icosh.stamp);
		become_host_endian((int *) &icosh, sizeof(IcosHeader) / sizeof(int));
	}
	EXITFUNC;
	return 0;
}

bool IcosIO::is_valid(const void *first_block)
{
	ENTERFUNC;
	bool result = false;
	if (!first_block) {
		result = false;
	}
	else {
		const int *data = static_cast < const int *>(first_block);
		int stamp = data[0];
		int stamp1 = data[19];
		int stamp2 = data[20];
		int stamp3 = data[26];

		bool data_big_endian = ByteOrder::is_data_big_endian(&stamp);

		if (data_big_endian != ByteOrder::is_host_big_endian()) {
			ByteOrder::swap_bytes(&stamp);
			ByteOrder::swap_bytes(&stamp1);
			ByteOrder::swap_bytes(&stamp2);
			ByteOrder::swap_bytes(&stamp3);
		}

		if (stamp == STAMP && stamp1 == STAMP1 && stamp2 == STAMP2 && stamp3 == STAMP3) {
			result = true;
		}
	}
	EXITFUNC;
	return result;
}

int IcosIO::read_header(Dict & dict, int image_index, const Region * area, bool)
{
	ENTERFUNC;
	int err = 0;
	if (check_read_access(image_index) != 0) {
		err = 1;
	}
	else {
		if (check_region(area, IntSize(icosh.nx, icosh.ny, icosh.nz)) != 0) {
			err = 1;
		}
		else {
			int xlen = 0, ylen = 0, zlen = 0;
			EMUtil::get_region_dims(area, icosh.nx, &xlen, icosh.ny, &ylen, icosh.nz, &zlen);
			
			dict["nx"] = xlen;
			dict["ny"] = ylen;
			dict["nz"] = zlen;
			dict["datatype"] = EMUtil::EM_FLOAT;
			dict["minimum"] = icosh.min;
			dict["maximum"] = icosh.max;
		}
	}
	EXITFUNC;
	return err;
}

int IcosIO::write_header(const Dict & dict, int image_index, const Region* area, bool)
{
	ENTERFUNC;
	int err = 0;
	if (check_write_access(rw_mode, image_index, 1) != 0) {
		err = 1;
	}
	else {
		if (area) {
			LOGERR("region write is not supported yet");
			err = 1;
		}
		else {
			icosh.stamp = STAMP;
			icosh.stamp1 = STAMP1;
			icosh.stamp2 = STAMP2;
			icosh.stamp3 = STAMP3;

			icosh.nx = dict["nx"];
			icosh.ny = dict["ny"];
			icosh.nz = dict["nz"];

			icosh.min = dict["minimum"];
			icosh.max = dict["maximum"];
		
			rewind(icos_file);
		
			if (fwrite(&icosh, sizeof(IcosHeader), 1, icos_file) != 1) {
				LOGERR("cannot write header to file '%s'", filename.c_str());
				err = 1;
			}
		}
	}
	EXITFUNC;
	return err;
}

int IcosIO::read_data(float *data, int image_index, const Region * area, bool)
{
	ENTERFUNC;
	int err = 0;
	
	if (check_read_access(image_index, true, data) != 0) {
		err = 1;
	}
	else {
		if (check_region(area, IntSize(icosh.nx, icosh.ny, icosh.nz)) != 0) {
			err = 1;
		}
		else {
			portable_fseek(icos_file, sizeof(IcosHeader), SEEK_SET);
			
			err = EMUtil::get_region_data((unsigned char *) data, icos_file, image_index,
										  sizeof(float), icosh.nx, icosh.ny, icosh.nz, 
										  area,false, sizeof(int), sizeof(int));
			if (err) {
				err = 1;
			}
			else {
				int xlen = 0, ylen = 0, zlen = 0;
				EMUtil::get_region_dims(area, icosh.nx, &xlen, icosh.ny, &ylen, icosh.nz, &zlen);
				become_host_endian(data, xlen * ylen * zlen);
			}
		}
	}
	EXITFUNC;
	return err;
}

int IcosIO::write_data(float *data, int image_index, const Region* area, bool)
{
	ENTERFUNC;
	int err = 0;
	
	if (check_write_access(rw_mode, image_index, 1, true, data) != 0) {
		err = 1;
	}
	else {
		if (area) {
			LOGERR("region write is not supported yet");
			err = 1;
		}
		else {
			int float_size = sizeof(float);
			int nx = icosh.nx;
			float *buf = new float[nx + 2];
			buf[0] = float_size * nx;
			buf[nx + 1] = buf[0];
			int nrows = icosh.ny * icosh.nz;

			int row_size = (nx + 2) * float_size;
			portable_fseek(icos_file, sizeof(IcosHeader), SEEK_SET);

			for (int j = 0; j < nrows; j++) {
				memcpy(&buf[1], &data[nx * j], nx * float_size);
				if (fwrite(buf, row_size, 1, icos_file) != 1) {
					LOGERR("writing ICOS data out failed");
					err = 1;
					break;
				}
			}

			delete[]buf;
			buf = 0;
		}
	}
	EXITFUNC;
	return err;
}

void IcosIO::flush()
{
	fflush(icos_file);
}

bool IcosIO::is_complex_mode()
{
	return false;
}

bool IcosIO::is_image_big_endian()
{
	return is_big_endian;
}

int IcosIO::get_nimg()
{
	if (init() != 0) {
		return 0;
	}

	return 1;
}
