/*
 *       Written by John Paul Neylon, PhD
 *                  University of California Los Angeles
 *                  200 Medical Plaza, Suite B265
 *                  Los Angeles, CA 90095
 *       2018-02-05
*/

#include "rtdir.h"

RTDIR::RTDIR()
{
    data_loaded = false;
    rtdir_file_found = false;
}
RTDIR::~RTDIR()
{
    /*if (rtdir_file_found)
        if (data_loaded)
        {
            printf("\n Delete DIR Array \n");
            delete [] data_array;
        }*/
}

void
RTDIR::setDicomDirectory( const char *buffer )
{
    dicom_dir.clear();
    dicom_dir = buffer;
}
void
RTDIR::setDicomFilename( char *buffer )
{
    dicom_full_filename.clear();
    dicom_full_filename = buffer;
}


bool
RTDIR::loadDicomInfo()
{
    char *filename;
    filename = new char[ dicom_dir.length() + 2 ];
    sprintf(filename,"%s",dicom_dir.data());

    rtdir_file_found = importSOPClassUID(filename);
    if ( rtdir_file_found )
    {
        printf(" RTDIR file found. -> %s\n", dicom_dir.data());
        fflush(stdout);
    }

    if ( rtdir_file_found )
        importPatientInfo();
    else
        printf("\n No RTDIR file found.");

    return rtdir_file_found;
}

bool
RTDIR::importSOPClassUID( char *filename )
{
    DcmFileFormat format;
    OFCondition status = format.loadFile( filename );
    if (status.bad())
    {
        printf("\n Error reading DICOM file:\n\t%s\n", filename );
        delete []filename;
        return 0;
    }

    DcmDataset *dataset = format.getDataset();

    bool series_is_rtdir = false;
    OFString ptSOPCLASSUID;
    if (dataset->findAndGetOFString(DCM_SOPClassUID, ptSOPCLASSUID).good())
        if(0 == ptSOPCLASSUID.compare(RTDIR_SOP_CLASS_UID) )
        {
            pt_sop_class_uid = ptSOPCLASSUID.data();
            series_is_rtdir = true;
        }

    return series_is_rtdir;
}

void
RTDIR::importPatientInfo()
{
    DcmFileFormat format;
    OFCondition status = format.loadFile( dicom_dir.data() );
    if (status.bad())
    {
        printf("\n Error reading DICOM file:\n\t%s\n", dicom_dir.data() );
        return;
    }

    DcmDataset *dataset = format.getDataset();
    OFString ptNAME;
    if (dataset->findAndGetOFString(DCM_PatientName, ptNAME).good())
    {
        pt_name = ptNAME.data();
        //printf("\n PT NAME: %s",pt_name.data());
    }
    OFString ptID;
    if (dataset->findAndGetOFString(DCM_PatientID, ptID).good())
    {
        pt_id = ptID.data();
        //printf("\n PT ID: %s",pt_id.data());
    }
    OFString DICOMDATE;
    if (dataset->findAndGetOFString(DCM_StudyDate,DICOMDATE).good())
    {
        dicom_date = DICOMDATE.data();
        //printf("\n ACQUISITION DATE: %s",dicom_date.data());
    }
    else if (dataset->findAndGetOFString(DCM_SeriesDate,DICOMDATE).good())
    {
        dicom_date = DICOMDATE.data();
        //printf("\n ACQUISITION DATE: %s",dicom_date.data());
    }
    else if (dataset->findAndGetOFString(DCM_AcquisitionDate,DICOMDATE).good())
    {
        dicom_date = DICOMDATE.data();
        //printf("\n ACQUISITION DATE: %s",dicom_date.data());
    }
    OFString ptSERIESDESCRIPTION;
    if (dataset->findAndGetOFString(DCM_SeriesDescription, ptSERIESDESCRIPTION).good())
    {
        pt_series_description = ptSERIESDESCRIPTION.data();
        //printf("\n PT SERIES DESCRIPTION: %s", pt_series_description.data());
    }
    OFString ptSOPINSTANCEUID;
    if (dataset->findAndGetOFString(DCM_SOPInstanceUID, ptSOPINSTANCEUID).good())
    {
        pt_sop_instance_uid = ptSOPINSTANCEUID.data();
        //printf("\n PT SOP INSTANCE UID: %s",pt_sop_instance_uid.data());
    }
    OFString ptSTUDYID;
    if (dataset->findAndGetOFString(DCM_StudyID, ptSTUDYID).good())
    {
        pt_study_id = ptSTUDYID.data();
        //printf("\n PT STUDY ID: %s",pt_study_id.data());
    }
    OFString ptSTUDYINSTANCEUID;
    if (dataset->findAndGetOFString(DCM_StudyInstanceUID, ptSTUDYINSTANCEUID).good())
    {
        pt_study_instance_uid = ptSTUDYINSTANCEUID.data();
        //printf("\n PT STUDY INSTANCE UID: %s",pt_study_instance_uid.data());
    }
    OFString ptSERIESINSTANCEUID;
    if (dataset->findAndGetOFString(DCM_SeriesInstanceUID, ptSERIESINSTANCEUID).good())
    {
        pt_series_instance_uid = ptSERIESINSTANCEUID.data();
        //printf("\n PT SERIES INSTANCE UID: %s",pt_series_instance_uid.data());
    }
    OFString ptREFFRAMEUID;
    if (dataset->findAndGetOFString(DCM_FrameOfReferenceUID, ptREFFRAMEUID).good())
    {
        reference_frame_uid = ptREFFRAMEUID.data();
        //printf("\n REFERENCE FRAME UID: %s",pt_series_instance_uid.data());
    }
    OFString ptREFSOPCLASSUID;
    if (dataset->findAndGetOFString(DCM_ReferencedSOPClassUID, ptREFSOPCLASSUID).good())
    {
        reference_sop_class_uid = ptREFSOPCLASSUID.data();
        //printf("\n REFERENCE FRAME UID: %s",pt_series_instance_uid.data());
    }
    OFString ptREFSOPINSTANCEUID;
    if (dataset->findAndGetOFString(DCM_ReferencedSOPInstanceUID, ptREFSOPINSTANCEUID).good())
    {
        reference_sop_instance_uid = ptREFSOPINSTANCEUID.data();
        //printf("\n REFERENCE FRAME UID: %s",pt_series_instance_uid.data());
    }
    printf("\n");
}



int
RTDIR::loadRTDIRData()
{
    // DeformableRegistrationSequence --> item 1
        // DeformableRegistrationGridSequence --> item 0
            //(0020,0032) DS [-209.60530090332\-147.28674316406\-136.99990844726] #  50, 3 ImagePositionPatient
            //(0020,0037) DS [1.0\0.0\0.0\0.0\1.0\0.0]                #  24, 6 ImageOrientationPatient
            //(0064,0007) UL 156\119\80                               #  12, 3 GridDimensions
            //(0064,0008) FD 2.5501749515533447\2.534717321395874\3.0000002384185791015625 #  24, 3 GridResolution
            //(0064,0009) OF 4.06517029\17.331741\8.8897705\2.9293976\17.341095\8.8681946... # 17821440, 1 VectorGridData

    DcmFileFormat format;
    OFCondition status = format.loadFile( dicom_dir.data() );
    if (status.bad())
    {
        printf("\n Error reading DICOM file:\n\t%s\n", dicom_dir.data() );
        return -1;
    }

    DcmDataset *dataset = format.getDataset();
    DcmItem *dirseq = NULL;

    if (dataset->findAndGetSequenceItem(DCM_DeformableRegistrationSequence, dirseq, 1).good())
    {
        DcmItem *dirgridseq = NULL;
        if (dirseq->findAndGetSequenceItem(DCM_DeformableRegistrationGridSequence, dirgridseq, 0).good())
        {
            Float64 imagePOSITION;
            if (dirgridseq->findAndGetFloat64(DCM_ImagePositionPatient, imagePOSITION, 0).good())
                data_origin.x = imagePOSITION;
            if (dirgridseq->findAndGetFloat64(DCM_ImagePositionPatient, imagePOSITION, 1).good())
                data_origin.y = imagePOSITION;
            if (dirgridseq->findAndGetFloat64(DCM_ImagePositionPatient, imagePOSITION, 2).good())
                data_origin.z = imagePOSITION;

            Uint32 gridDIMENSION;
            if (dirgridseq->findAndGetUint32(DCM_GridDimensions, gridDIMENSION, 0).good())
                data_size.x = gridDIMENSION;
            if (dirgridseq->findAndGetUint32(DCM_GridDimensions, gridDIMENSION, 1).good())
                data_size.y = gridDIMENSION;
            if (dirgridseq->findAndGetUint32(DCM_GridDimensions, gridDIMENSION, 2).good())
                data_size.z = gridDIMENSION;

            Float64 gridRESOLUTION;
            if (dirgridseq->findAndGetFloat64(DCM_GridResolution, gridRESOLUTION, 0).good())
                voxel_size.x = gridRESOLUTION;
            if (dirgridseq->findAndGetFloat64(DCM_GridResolution, gridRESOLUTION, 1).good())
                voxel_size.y = gridRESOLUTION;
            if (dirgridseq->findAndGetFloat64(DCM_GridResolution, gridRESOLUTION, 2).good())
                voxel_size.z = gridRESOLUTION;

            printf("\n Data Dimensions: %d x %d x %d \n Voxel Dimensions: %2.3f x %2.3f x %2.3f",data_size.x,data_size.y,data_size.z,voxel_size.x,voxel_size.y,voxel_size.z);
            printf("\n DATA Position: %2.3f x %2.3f x %2.3f \n",data_origin.x,data_origin.y,data_origin.z);

            int size = data_size.x * data_size.y * data_size.z;
            if (size > 0)
            {
                u_array =  new float[size];
                v_array =  new float[size];
                w_array =  new float[size];

                data_min.x = 0;
                data_min.y = 0;
                data_min.z = 0;
                data_max.x = 0;
                data_max.y = 0;
                data_max.z = 0;

                for (int i=0; i<size; i++)
                {
                    float u_value = 0;
                    float v_value = 0;
                    float w_value = 0;

                    OFString vectorData;
                    if (dirgridseq->findAndGetOFString(DCM_VectorGridData, vectorData, 3*i+0, false).good())
                        u_value = atof( vectorData.data() );
                    if (dirgridseq->findAndGetOFString(DCM_VectorGridData, vectorData, 3*i+1, false).good())
                        v_value = atof( vectorData.data() );
                    if (dirgridseq->findAndGetOFString(DCM_VectorGridData, vectorData, 3*i+2, false).good())
                        w_value = atof( vectorData.data() );

                    u_array[i] = u_value;
                    v_array[i] = v_value;
                    w_array[i] = w_value;

                    if (u_value > data_max.x) data_max.x = u_value;
                    if (u_value < data_min.x) data_min.x = u_value;
                    if (v_value > data_max.y) data_max.y = v_value;
                    if (v_value < data_min.y) data_min.y = v_value;
                    if (w_value > data_max.z) data_max.z = w_value;
                    if (w_value < data_min.z) data_min.z = w_value;
                }
                printf("\n Data Min: %4.3f | %4.3f | %4.3f\n Data Max: %4.3f | %4.3f | %4.3f\n\n",data_min.x,data_min.y,data_min.z,data_max.x,data_max.y,data_max.z);
                data_loaded = true;
            }
        }
    }

    return 1;
}


/*
export_dicom_data is launched once for each slice to written. It creates a DICOM file in the indicated output path, load a template header from the incidated input path,
applies an anonymization to the header, writes the 3D volume parameters from the dataVol.params structure to the header file, and writes the pixel data from the newData source.
The file is then saved to the DICOM directory.
*/
int
RTDIR::saveRTDIRData( const char *outpath, float *new_u, float *new_v, float *new_w, bool anonymize_switch )
{
    DcmFileFormat format;
    OFCondition status = format.loadFile( dicom_dir.data() );
    if (status.bad())
    {
        printf("\n Error reading DICOM file:\n\t%s\n", dicom_dir.data() );
        return 0;
    }

    DcmDataset *dataset = format.getDataset();

    if (anonymize_switch)
        anonymize( dataset );

    time_t rawtime;
    struct tm * timeinfo;
    char buffer[64];
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    strftime (buffer,32,"%G%m%d",timeinfo);
    dataset->putAndInsertString(DCM_AcquisitionDate,buffer).good();
    dataset->putAndInsertString(DCM_SeriesDate,buffer).good();

    //sprintf(buffer,"NEW DIR DATA");
    //format.getDataset()->putAndInsertString(DCM_StudyDescription,buffer).good();
    //format.getDataset()->putAndInsertString(DCM_SeriesDescription,buffer).good();

    Float32 *new_vector;
    new_vector = new Float32[3*data_size.z*data_size.y*data_size.x];

    for (int p=0; p<data_size.z*data_size.y*data_size.x; p++)
    {
        printf("\r Writing Vector %d / %d",p,data_size.z*data_size.y*data_size.x);

        new_vector[3*p+0] = (Float32)new_u[p];
        new_vector[3*p+1] = (Float32)new_v[p];
        new_vector[3*p+2] = (Float32)new_w[p];
    }

    DcmItem *dirseq = NULL;
    if (dataset->findAndGetSequenceItem(DCM_DeformableRegistrationSequence, dirseq, 1).good())
    {
        DcmItem *dirgridseq = NULL;
        if (dirseq->findAndGetSequenceItem(DCM_DeformableRegistrationGridSequence, dirgridseq, 0).good())
        {
            if (dirgridseq->putAndInsertFloat32Array(DCM_VectorGridData, new_vector, 3*data_size.z*data_size.y*data_size.x, OFTrue).good())
            {
                status = format.saveFile( outpath );
                if (status.bad())
                    printf("Error: cannot write DICOM file ( %s )",status.text() );
            }
            else
                printf("\n Could not open Vector Grid Data \n");
        }
        else
            printf("\n Could not open Deformable Registration Grid Sequence \n");
    }
    else
        printf("\n Could not open Deformable Registration Sequence \n");

    //new_vector_grid.clear();
    delete [] new_vector;

    return 1;
}

/*
void
RTDose::saveRTDoseData( const char *outpath, bool anonymize_switch )
{
    DcmFileFormat format;
    OFCondition status = format.loadFile( dicom_full_filename.data() );
    if (status.bad())
    {
        printf("\n Error reading DICOM file:\n\t%s\n", dicom_full_filename.data() );
        return;
    }

    if (anonymize_switch)
        anonymize( format.getDataset() );

    time_t rawtime;
    struct tm * timeinfo;
    char buffer[64];
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    strftime (buffer,32,"%G%m%d",timeinfo);
    //format.getDataset()->putAndInsertString(DCM_AcquisitionDate,buffer).good();
    format.getDataset()->putAndInsertString(DCM_SeriesDate,buffer).good();
    printf("  Creation date written...\n");
    fflush(stdout);

    printf("\n %s",outpath);
    if (0 == mkdir((const char *)outpath,S_IRWXU|S_IRWXG|S_IRWXO) )
    {
        printf("\n ...directory created successfully.");
        fflush(stdout);
    }
    else
    {
        printf("\n Directory already exists.\n");
        fflush(stdout);
    }

    DRTDoseIOD rtdose_template;
    status = rtdose_template.read(*format.getDataset());

    DRTDoseIOD rtdose_new(rtdose_template);

    format.clear();
    status = rtdose_new.write(*format.getDataset());
    if (status.good())
    {
        char outfilename[512];
        sprintf(outfilename,"%s/RD.new_dose.dcm",outpath);
        status = format.saveFile( outfilename );
        if (status.bad())
            printf("Error: cannot write DICOM file ( %s )",status.text() );
    }
}
*/

/*
anonymize ensures no patient data is included when exporting
DICOM data in the export_dicom_data function
*/
void
RTDIR::anonymize( DcmDataset *dataset )
{
    dataset->findAndDeleteElement(DCM_AccessionNumber, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_ReferringPhysicianName, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_ReferringPhysicianAddress, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_ReferringPhysicianTelephoneNumbers, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_ReferringPhysicianIdentificationSequence, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_PhysiciansOfRecord, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_InstitutionName, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_InstitutionAddress, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_InstitutionCodeSequence, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_PatientName, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_PatientID, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_IssuerOfPatientID, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_TypeOfPatientID, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_IssuerOfPatientIDQualifiersSequence, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_PatientBirthDate, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_PatientBirthTime, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_PatientSex, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_PatientInsurancePlanCodeSequence, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_PatientPrimaryLanguageCodeSequence, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_PatientPrimaryLanguageModifierCodeSequence, OFTrue, OFTrue);
//    dataset->findAndDeleteElement(DCM_OtherPatientIDs, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_OtherPatientNames, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_OtherPatientIDsSequence, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_PatientBirthName, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_PatientAge, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_PatientSize, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_PatientSizeCodeSequence, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_PatientWeight, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_PatientAddress, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_PatientMotherBirthName, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_MilitaryRank, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_BranchOfService, OFTrue, OFTrue);
//    dataset->findAndDeleteElement(DCM_MedicalRecordLocator, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_MedicalAlerts, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_Allergies, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_CountryOfResidence, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_RegionOfResidence, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_PatientTelephoneNumbers, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_EthnicGroup, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_Occupation, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_SmokingStatus, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_AdditionalPatientHistory, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_PregnancyStatus, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_LastMenstrualDate, OFTrue, OFTrue);
    dataset->findAndDeleteElement(DCM_PatientReligiousPreference, OFTrue, OFTrue);
}



