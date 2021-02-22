/*
 *       Written by John Paul Neylon, PhD
 *                  University of California Los Angeles
 *                  200 Medical Plaza, Suite B265
 *                  Los Angeles, CA 90095
 *       2018-02-05
*/

#ifndef __RTDIR_H__
#define __RTDIR_H__

#include "dcmtk_includes.h"

#define RTDIR_SOP_CLASS_UID "1.2.840.10008.5.1.4.1.1.66.3"

using namespace RTC;

class RTDIR
{
public:
    RTDIR  ();
    ~RTDIR ();

    bool loadDicomInfo();
    int  loadRTDIRData();
    int  saveRTDIRData ( const char *outpath, float *new_u, float *new_v, float *new_w, bool anonymize_switch );
    //void saveRTDIRData ( const char *outpath, bool anonymize_switch );
    bool importSOPClassUID( char *buffer );
    void importPatientInfo();
    void anonymize( DcmDataset *dataset );
    void _finalize();

    char*  getDicomDirectory()
    {
        return (char*)dicom_dir.data();
    };
    char*  getDicomFilename()
    {
        return (char*)dicom_full_filename.data();
    };
    char*  getReferenceFrameUID()
    {
        return (char*)reference_frame_uid.data();
    };

    void   setDicomFilename( char *buffer );
    void   setDicomDirectory( const char *buffer );

    float  getUArrayVoxel(int i, int j, int k)
    {
        return u_array[i + data_size.x*(j + data_size.y*k)];
    };
    float  getVArrayVoxel(int i, int j, int k)
    {
        return v_array[i + data_size.x*(j + data_size.y*k)];
    };
    float  getWArrayVoxel(int i, int j, int k)
    {
        return w_array[i + data_size.x*(j + data_size.y*k)];
    };
    rtint3    getDataSize()
    {
        return data_size;
    };
    rtfloat3  getVoxelSize()
    {
        return voxel_size;
    };
    rtfloat3  getDataOrigin()
    {
        return data_origin;
    };

    float  getUDataMin()
    {
        return data_min.x;
    };
    float  getUDataMax()
    {
        return data_max.x;
    };
    float  getVDataMin()
    {
        return data_min.y;
    };
    float  getVDataMax()
    {
        return data_max.y;
    };
    float  getWDataMin()
    {
        return data_min.z;
    };
    float  getWDataMax()
    {
        return data_max.z;
    };
    float* getUDataArray()
    {
        return u_array;
    };
    float* getVataArray()
    {
        return v_array;
    };
    float* getWDataArray()
    {
        return w_array;
    };

    void   setUArrayVoxel(int x, int y, int z, float value)
    {
        u_array[x + data_size.x*(y + data_size.y*z)] = value;
    };
    void   setVArrayVoxel(int x, int y, int z, float value)
    {
        v_array[x + data_size.x*(y + data_size.y*z)] = value;
    };
    void   setWArrayVoxel(int x, int y, int z, float value)
    {
        w_array[x + data_size.x*(y + data_size.y*z)] = value;
    };
    void   setDataSize(int x, int y, int z)
    {
        data_size.x = x;
        data_size.y = y;
        data_size.z = z;
    };
    void   setVoxelSize(float v_x, float v_y, float v_z)
    {
        voxel_size.x = v_x;
        voxel_size.y = v_y;
        voxel_size.z = v_z;
    };
    void   setUDataMin(float v)
    {
        data_min.x = v;
    };
    void   setUDataMax(float v)
    {
        data_max.x = v;
    };
    void   setVDataMin(float v)
    {
        data_min.y = v;
    };
    void   setVDataMax(float v)
    {
        data_max.y = v;
    };
    void   setWDataMin(float v)
    {
        data_min.z = v;
    };
    void   setWDataMax(float v)
    {
        data_max.z = v;
    };

protected:
    std::string dicom_dir;
    std::string dicom_date;
    std::string dicom_full_filename;
    std::string pt_series_description;
    std::string pt_name;
    std::string pt_id;
    std::string pt_study_id;
    std::string pt_sop_class_uid;
    std::string pt_sop_instance_uid;
    std::string pt_study_instance_uid;
    std::string pt_series_instance_uid;

    std::string reference_frame_uid;
    std::string reference_sop_class_uid;
    std::string reference_sop_instance_uid;

    rtint3 data_size;
    rtfloat3 voxel_size;
    rtfloat3 data_origin;
    rtfloat3 data_min;
    rtfloat3 data_max;
    bool  data_loaded;
    bool  rtdir_file_found;

    float *u_array;
    float *v_array;
    float *w_array;

    std::string stringify_float( float value )
    {
        char *text;
        text = new char[sizeof(float)];
        sprintf(text,"%f",value);
        //printf("\n %f =? %s \n",value,text);
        std::string str = text;
        delete [] text;
        return str;
    };
};

#endif // __RTDIR_H__
