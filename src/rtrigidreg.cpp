/*
 *       Written by John Paul Neylon, PhD
 *                  University of California Los Angeles
 *                  200 Medical Plaza, Suite B265
 *                  Los Angeles, CA 90095
 *       2018-01-24
*/

#include "rtrigidreg.h"

RTRigReg::RTRigReg()
{
    data_loaded = false;
    rtrigreg_file_found = false;
    for (uint i=0; i<16; i++)
        matrix[i] = 0;
}
RTRigReg::~RTRigReg()
{
    /*if (rtdose_file_found)
        if (data_loaded)
        {
            printf("\n Delete Dose Array \n");
            delete [] data_array;
        }*/
}

void
RTRigReg::setDicomDirectory( const char *buffer )
{
    dicom_dir.clear();
    dicom_dir = buffer;
}
void
RTRigReg::setDicomFilename( char *buffer )
{
    dicom_full_filename.clear();
    dicom_full_filename = buffer;
}


bool
RTRigReg::loadDicomInfo()
{
    char *filename;
    filename = new char[ dicom_dir.length() + 2 ];
    sprintf(filename,"%s",dicom_dir.data());

    rtrigreg_file_found = importSOPClassUID(filename);
    if ( rtrigreg_file_found )
    {
        printf(" RT RIGID REGISTRATION file found. -> %s\n", dicom_dir.data());
        fflush(stdout);
    }

    if ( rtrigreg_file_found )
        importPatientInfo();
    else
        printf("\n No RT RIGID REGISTRATION file found.");

    return rtrigreg_file_found;
}

bool
RTRigReg::importSOPClassUID( char *filename )
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

    bool series_is_rtrigreg = false;
    OFString ptSOPCLASSUID;
    if (dataset->findAndGetOFString(DCM_SOPClassUID, ptSOPCLASSUID).good())
        if(0 == ptSOPCLASSUID.compare(RTRIGREG_SOP_CLASS_UID) )
        {
            pt_sop_class_uid = ptSOPCLASSUID.data();
            series_is_rtrigreg = true;
        }

    return series_is_rtrigreg;
}

void
RTRigReg::importPatientInfo()
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
RTRigReg::loadRigRegData()
{
    DcmFileFormat format;
    OFCondition status = format.loadFile( dicom_dir.data() );
    if (status.bad())
    {
        printf("\n Error reading DICOM file:\n\t%s\n", dicom_dir.data() );
        return -1;
    }

    DcmDataset *dataset = format.getDataset();
    DcmItem *regseq = NULL;
    if (dataset->findAndGetSequenceItem(DCM_RegistrationSequence, regseq, 1).good())
    {
        DcmItem *matregseq = NULL;
        if (regseq->findAndGetSequenceItem(DCM_MatrixRegistrationSequence, matregseq, 0).good())
        {
            DcmItem *matseq;
            if (matregseq->findAndGetSequenceItem(DCM_MatrixSequence, matseq, 0).good())
            {
                OFString regTYPE;
                if (matseq->findAndGetOFString(DCM_FrameOfReferenceTransformationMatrixType, regTYPE).good())
                {
                    reg_type = regTYPE.data();
                    printf(" REGISTRATION TYPE: %s",reg_type.data());
                }

                Float64 matrixENTRY;
                for (uint i=0; i<16; i++)
                    if (matseq->findAndGetFloat64(DCM_FrameOfReferenceTransformationMatrix, matrixENTRY, i).good())
                        matrix[i] = matrixENTRY;

                printf("\n TRANSFORMATION MATRIX:");
                printf("\n\t%f\t%f\t%f\t%f",matrix[0],matrix[1],matrix[2],matrix[3]);
                printf("\n\t%f\t%f\t%f\t%f",matrix[4],matrix[5],matrix[6],matrix[7]);
                printf("\n\t%f\t%f\t%f\t%f",matrix[8],matrix[9],matrix[10],matrix[11]);
                printf("\n\t%f\t%f\t%f\t%f",matrix[12],matrix[13],matrix[14],matrix[15]);
            }
            else
            {
                printf("\n Could not open Matrix Sequence! \n");
            }
        }
        else
        {
            printf("\n Could not open Matrix Registration Sequence! \n");
        }
    }
    else
    {
        printf("\n Could not open Registration Sequence! \n");
    }
    return 0;
}
