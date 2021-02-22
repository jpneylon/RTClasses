/*
 *       Written by John Paul Neylon, PhD
 *                  University of California Los Angeles
 *                  200 Medical Plaza, Suite B265
 *                  Los Angeles, CA 90095
 *       2018-01-24
*/

#ifndef __RTRIGREG_H__
#define __RTRIGREG_H__

#include "dcmtk_includes.h"

#define RTRIGREG_SOP_CLASS_UID "1.2.840.10008.5.1.4.1.1.66.1"

using namespace RTC;

class RTRigReg
{
public:
    RTRigReg  ();
    ~RTRigReg ();

    float matrix[16];

    bool loadDicomInfo();
    int  loadRigRegData();
    //int  saveRigRegData ( const char *outpath, float *newData, bool anonymize_switch );
    //void saveRigRegData ( const char *outpath, bool anonymize_switch );
    bool importSOPClassUID( char *buffer );
    void importPatientInfo();
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

    bool data_loaded;
    bool rtrigreg_file_found;
    std::string reg_type;
};

#endif // __RTRIGREG_H__

