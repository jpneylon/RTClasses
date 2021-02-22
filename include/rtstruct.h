#ifndef __RTSTRUCT_H__
#define __RTSTRUCT_H__

#include "dcmtk_includes.h"
#include "dcmtk/dcmrt/drtstrct.h"
#include "dcmtk/dcmrt/drmstrct.h"

#define RTSTRUCT_SOP_CLASS_UID "1.2.840.10008.5.1.4.1.1.481.3"

using namespace RTC;

class RTStruct
{
public:

    RTStruct  ();
    ~RTStruct ();

    class CNTR_DATA
    {
      public:
        float *points;
    };

    class ROI_DATA
    {
      public:
        int roi_number;
        std::string roi_name;
        rtint3 roi_rgb_color;
        uint sub_cntr_count;
        bool load_data;
        uint *sub_cntr_points_count;
        uint total_points_count;
        rtfloat3 range_min;
        rtfloat3 range_max;
        CNTR_DATA *sub_cntr_data;
    };

    bool loadDicomInfo();
    void loadRTStructInfo();
    int  loadRTStructData( int r );
    void chooseContours();
    bool importSOPClassUID( char *buffer );
    void importPatientInfo();

    void copyROI( int r, ROI_DATA *roi_copy, int c );
    void freeROI( ROI_DATA *roi_copy, int c );
    void anonymize( DcmDataset *dataset );
    void saveRTStructData ( const char *outpath, ROI_DATA *new_roi_data, uint new_roi_count, bool anonymize_switch );
    void saveRTStructData ( const char *outpath, bool anonymize_switch );

    uint    getNumberOfROIs()
    {
        return roi_count;
    };
    char*  getDicomDirectory()
    {
        return (char*)dicom_dir.data();
    };
    char*  getDicomFilename()
    {
        return (char*)dicom_full_filename.data();
    };

    rtfloat3 getSubCntrPoint( uint r, uint s, uint p );
    int    getROINumber( uint r )
    {
        return roi_array[r].roi_number;
    };
    char*  getROIName( uint r )
    {
        return (char*)roi_array[r].roi_name.data();
    };
    rtint3    getROIColor( uint r )
    {
        return roi_array[r].roi_rgb_color;
    };
    uint    getROISubCntrCount( uint r )
    {
        return roi_array[r].sub_cntr_count;
    };
    uint    getROISubCntrPointCount( uint r, uint s )
    {
        return roi_array[r].sub_cntr_points_count[s];
    };
    bool   getROILoadDataSwitch( uint c )
    {
        return roi_array[c].load_data;
    };
    float* getROISubCntrPoints( uint r, uint s )
    {
        return roi_array[r].sub_cntr_data[s].points;
    };
    uint    getROITotalPointsCount( uint r )
    {
        return roi_array[r].total_points_count;
    };

    void   setDicomFilename( char *buffer );
    void   setDicomDirectory( const char *buffer );
    void   setSubCntrPoint( uint r, uint s, uint p, float v_x, float v_y, float v_z );

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

    bool rtstruct_file_found;

    uint roi_count;
    ROI_DATA *roi_array;
};

#endif // __RTSTRUCT_H__
