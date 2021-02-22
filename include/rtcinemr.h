#ifndef __RTCINEMR_H__
#define __RTCINEMR_H__

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

class RTCineMR
{
public:
    RTCineMR  ();
    ~RTCineMR ();

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
    class PHASE_DATA
    {
      public:
        int phase_id;
        uint image_count;
        uint slice_count;
        SLICE_DATA *slice;

        std::string dicom_dir;
        std::string dicom_date;
        std::string pt_series_description;
        std::string pt_name;
        std::string pt_id;
        std::string pt_study_id;
        std::string pt_study_instance_uid;
        std::string pt_series_instance_uid;

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

    bool loadDicomInfo();
    int  loadRTImageData( uint p );
    void saveRTPhaseData ( const char *outpath, uint p, bool anonymize_switch );
    bool importSOPClassUID( char *buffer );
    void importInstanceNumber( uint i );
    void importPatientInfo( uint p );
    void anonymize( DcmDataset *dataset );
    void copySliceDataToPhaseSlice( uint s, uint p, uint ps );

    char*  getDicomDirectory()
    {
        return (char*)dicom_dir.data();
    };
    float  getArrayVoxel(int i, int j, int k, uint p)
    {
        return phases[p].data_array[i + phases[p].data_size.x*(j + phases[p].data_size.y*k)];
    };
    uint getImageCount()
    {
        return image_count;
    };
    uint getPhaseImageCount( uint p )
    {
        return phases[p].image_count;
    };
    rtint3    getDataSize( uint p )
    {
        return phases[p].data_size;
    };
    rtfloat3  getVoxelSize( uint p )
    {
        return phases[p].voxel_size;
    };
    float  getRescaleSlope( uint p )
    {
        return phases[p].rescale_slope;
    };
    float  getRescaleIntercept( uint p )
    {
        return phases[p].rescale_intercept;
    };
    float  getDataMin( uint p )
    {
        return phases[p].data_min;
    };
    float  getDataMax( uint p )
    {
        return phases[p].data_max;
    };
    float* getDataArray( uint p )
    {
        return phases[p].data_array;
    };
    float  getWindowCenter( uint p )
    {
        return phases[p].window_center;
    };
    float  getWindowWidth( uint p )
    {
        return phases[p].window_width;
    };
    int    getSliceInstanceNumber(uint i)
    {
        return slice[i].instance_number;
    };
    int    getPhaseSliceInstanceNumber(uint i, uint p)
    {
        return phases[p].slice[i].instance_number;
    };
    rtfloat3  getSliceImagePositionPatient(uint i)
    {
        return slice[i].image_position_patient;
    };
    rtfloat3  getPhaseSliceImagePositionPatient(uint i, uint p)
    {
        return phases[p].slice[i].image_position_patient;
    };
    char*  getSliceSOPInstanceUID(uint i)
    {
        return (char*)slice[i].sop_instance_uid.data();
    };
    char*  getSliceReferenceFrameUID(uint i)
    {
        return (char*)slice[i].reference_frame_uid.data();
    };
    char*  getPhaseSliceReferenceFrameUID(uint i, uint p)
    {
        return (char*)phases[p].slice[i].reference_frame_uid.data();
    };
    char*  getSliceFilename(uint i)
    {
        return (char*)slice[i].filename.data();
    };
    char*  getPhaseSliceFilename(uint i, uint p)
    {
        return (char*)phases[p].slice[i].filename.data();
    };

    void   setDicomDirectory( const char *buffer );
    void   setSliceSOPInstanceUID( uint i, char *buffer);
    void   setSliceFilename(uint i, char *buffer);

    void   setArrayVoxel(int x, int y, int z, uint p, float v)
    {
        phases[p].data_array[x + phases[p].data_size.x*(y + phases[p].data_size.y*z)] = v;
    };
    void   setDataSize(int x, int y, int z, uint p)
    {
        phases[p].data_size.x = x;
        phases[p].data_size.y = y;
        phases[p].data_size.z = z;
    };
    void   setVoxelSize(float v_x, float v_y, float v_z, uint p)
    {
        phases[p].voxel_size.x = v_x;
        phases[p].voxel_size.y = v_y;
        phases[p].voxel_size.z = v_z;
    };
    void   setDataOrigin(float v_x, float v_y, float v_z, uint p)
    {
        phases[p].data_origin.x = v_x;
        phases[p].data_origin.y = v_y;
        phases[p].data_origin.z = v_z;
    };
    void   setRescaleSlope(float v, uint p)
    {
        phases[p].rescale_slope = v;
    };
    void   setRescaleIntercept(float v, uint p)
    {
        phases[p].rescale_intercept = v;
    };
    void   setDataMin(float v, uint p)
    {
        phases[p].data_min = v;
    };
    void   setDataMax(float v, uint p)
    {
        phases[p].data_max = v;
    };
    void   setWindowCenter(float v, uint p)
    {
        phases[p].window_center = v;
    };
    void   setWindowWidth(float v, uint p)
    {
        phases[p].window_width = v;
    };
    void   setSliceInstanceNumber(uint i, uint p, int n)
    {
        phases[p].slice[i].instance_number = n;
    };
    void   setSliceImagePositionPatient(uint i, uint p, float v_x, float v_y, float v_z);

    enum { SOP_RTIMAGE = 0, SOP_CT, SOP_MR };
    unsigned int sop_type;

    uint get_phase_count(){ return phase_count; };

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
    uint phase_count;
    uint images_per_phase;
    SLICE_DATA *slice;
    PHASE_DATA *phases;
};

#endif // __RTCINEMR_H__

