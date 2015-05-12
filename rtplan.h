#ifndef __RTPLAN_H__
#define __RTPLAN_H__

#include "dcmtk_includes.h"
#include "dcmtk/dcmrt/drtplan.h"
#include "dcmtk/dcmrt/drmplan.h"

#define RTPLAN_SOP_CLASS_UID "1.2.840.10008.5.1.4.1.1.481.5"

class RTPlan
{
public:

    RTPlan  ();
    ~RTPlan ();

    class OAR_DATA
    {
      public:
        float full_volume_dose;
        float max_dose;
        float overdose_volume_fraction;
    };

    class TARGET_DATA
    {
      public:
        float rx_dose;
        float max_dose;
        float min_dose;
        float underdose_volume_fraction;
    };

    class DOSE_REF_DATA
    {
      public:
        uint number;
        uint ref_roi_number;
        float constraint_weight;
        std::string description;
        std::string type;
        bool oar_target_switch; // 0 = oar, 1 = target
        OAR_DATA *oar;
        TARGET_DATA *target;
    };

    class BEAM_DATA
    {
      public:
        uint   number;
        std::string description;
        float sad;
    };

    bool loadDicomInfo();
    void loadRTPlanData();
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

    float3  getIsocenter()
    {
        return isocenter;
    };
    float3  getRXDoseLevels()
    {
        return rx_dose_levels;
    };
    uint    getFractionCount()
    {
        return fraction_count;
    };

    void   setDicomFilename( char *buffer );
    void   setDicomDirectory( char *buffer );

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

    std::string rx_description;
    float3 isocenter;
    uint3 refd_rx_dose_ref_number;
    float3 rx_dose_levels;
    float ssd;
    uint fraction_count;
    uint beam_count;
    uint dose_ref_count;
    BEAM_DATA *beam_data;
    DOSE_REF_DATA *dose_ref_data;
};

#endif // __RTPLAN_H__
