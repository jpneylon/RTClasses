#ifndef __RTDOSE_H__
#define __RTDOSE_H__

#include "dcmtk_includes.h"
#include "dcmtk/dcmrt/drtdose.h"
#include "dcmtk/dcmrt/drmdose.h"

#define RTDOSE_SOP_CLASS_UID "1.2.840.10008.5.1.4.1.1.481.2"

#ifndef UINT16_MAX
#define UINT16_MAX 65535
#endif

class RTDose
{
public:
    RTDose  ();
    ~RTDose ();

    bool loadDicomInfo();
    int  loadRTDoseData();
    int  saveRTDoseData ( const char *outpath, float *newData, bool anonymize_switch );
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
    void   setDicomDirectory( char *buffer );

    float  getArrayVoxel(int i, int j, int k)
    {
        return data_array[i + data_size.x*(j + data_size.y*k)];
    };
    int3    getDataSize()
    {
        return data_size;
    };
    float3  getVoxelSize()
    {
        return voxel_size;
    };
    float3  getDataOrigin()
    {
        return data_origin;
    };
    float  getDataScaler()
    {
        return data_scaler;
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
    void   setDataScaler(float v)
    {
        data_scaler = v;
    };
    void   setDataMin(float v)
    {
        data_min = v;
    };
    void   setDataMax(float v)
    {
        data_max = v;
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

    int3 data_size;
    float3 voxel_size;
    float3 data_origin;
    float data_min;
    float data_max;

    float data_scaler;
    float *data_array;
};

#endif // __RTDOSE_H__
