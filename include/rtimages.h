#ifndef __RTIMAGE_H__
#define __RTIMAGE_H__

#include "dcmtk_includes.h"
#include "dcmtk/dcmrt/drtimage.h"
#include "dcmtk/dcmrt/drmimage.h"

#define RTIMAGE_SOP_CLASS_UID "1.2.840.10008.5.1.4.1.1.481.1"
#define CTIMAGE_SOP_CLASS_UID "1.2.840.10008.5.1.4.1.1.2"
#define MRIMAGE_SOP_CLASS_UID "1.2.840.10008.5.1.4.1.1.4"

#ifndef UINT16_MAX
#define UINT16_MAX 65535
#endif

using namespace RTC;

class RTImage
{
public:
    RTImage  ();
    ~RTImage ();

    class SLICE_DATA
    {
      public:
        std::string filename;
        std::string sop_instance_uid;
        std::string reference_frame_uid;
        int instance_number;
        float slice_location;
        rtfloat3 image_position_patient;
    };

    bool loadDicomInfo();
    int  loadRTImageData();
    void saveRTImageData ( const char *outpath, bool anonymize_switch );
    int  saveRTImageData ( const char *outpath, float *newData, bool anonymize_switch );
    bool importSOPClassUID( char *buffer );
    void importInstanceNumber( uint i );
    void importPatientInfo();
    void anonymize( DcmDataset *dataset );

    char*  getDicomDirectory()
    {
        return (char*)dicom_dir.data();
    };
    float  getArrayVoxel(int i, int j, int k)
    {
        return data_array[i + data_size.x*(j + data_size.y*k)];
    };
    uint getImageCount()
    {
        return image_count;
    };
    rtint3    getDataSize()
    {
        return data_size;
    };
    rtfloat3  getVoxelSize()
    {
        return voxel_size;
    };
    float  getRescaleSlope()
    {
        return rescale_slope;
    };
    float  getRescaleIntercept()
    {
        return rescale_intercept;
    };
    float  getDataMin()
    {
        return data_min;
    };
    float  getDataMax()
    {
        return data_max;
    };
    float* getDataArray()
    {
        return data_array;
    };
    float  getWindowCenter()
    {
        return window_center;
    };
    float  getWindowWidth()
    {
        return window_width;
    };
    int    getSliceInstanceNumber(uint i)
    {
        return slice[i].instance_number;
    };
    rtfloat3  getSliceImagePositionPatient(uint i)
    {
        return slice[i].image_position_patient;
    };
    char*  getSliceSOPInstanceUID(uint i)
    {
        return (char*)slice[i].sop_instance_uid.data();
    };
    char*  getSliceReferenceFrameUID(uint i)
    {
        return (char*)slice[i].reference_frame_uid.data();
    };
    char*  getSliceFilename(uint i)
    {
        return (char*)slice[i].filename.data();
    };

    void   setDicomDirectory( const char *buffer );
    void   setSliceSOPInstanceUID( uint i, char *buffer);
    void   setSliceFilename(uint i, char *buffer);

    void   setArrayVoxel(int x, int y, int z, float v)
    {
        data_array[x + data_size.x*(y + data_size.y*z)] = v;
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
    void   setDataOrigin(float v_x, float v_y, float v_z)
    {
        data_origin.x = v_x;
        data_origin.y = v_y;
        data_origin.z = v_z;
    };
    void   setRescaleSlope(float v)
    {
        rescale_slope = v;
    };
    void   setRescaleIntercept(float v)
    {
        rescale_intercept = v;
    };
    void   setDataMin(float v)
    {
        data_min = v;
    };
    void   setDataMax(float v)
    {
        data_max = v;
    };
    void   setWindowCenter(float v)
    {
        window_center = v;
    };
    void   setWindowWidth(float v)
    {
        window_width = v;
    };
    void   setSliceInstanceNumber(uint i, int n)
    {
        slice[i].instance_number = n;
    };
    void   setSliceImagePositionPatient(uint i, float v_x, float v_y, float v_z);

    enum { SOP_RTIMAGE = 0, SOP_CT, SOP_MR };
    unsigned int sop_type;

protected:
    std::string dicom_dir;
    std::string dicom_date;
    std::string pt_series_description;
    std::string pt_name;
    std::string pt_id;
    std::string pt_study_id;
    std::string pt_study_instance_uid;
    std::string pt_series_instance_uid;

    uint image_count;
    SLICE_DATA *slice;

    rtint3 data_size;
    rtfloat3 voxel_size;
    rtfloat3 data_origin;
    rtfloat3 orient_x;
    rtfloat3 orient_y;

    float window_center;
    float window_width;
    float rescale_slope;
    float rescale_intercept;

    float data_min;
    float data_max;
    float *data_array;
};

#endif // __RTIMAGE_H__

