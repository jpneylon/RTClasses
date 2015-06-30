#include "rtplan.h"

RTPlan::RTPlan()
{
    isocenter.x = 0;
    isocenter.y = 0;
    isocenter.z = 0;
    rx_dose_levels.x = 0;
    rx_dose_levels.y = 0;
    rx_dose_levels.z = 0;
    refd_rx_dose_ref_number.x = 0;
    refd_rx_dose_ref_number.y = 0;
    refd_rx_dose_ref_number.z = 0;
    ssd = 100;
    fraction_count = 1;
    beam_count = 1;
    dose_ref_count = 0;
}

RTPlan::~RTPlan()
{
    _finalize();
}
void
RTPlan::_finalize()
{
    if (beam_data)
        delete []beam_data;

    if (dose_ref_data)
    {
        for (uint d=0; d<dose_ref_count; d++)
        {
            if (dose_ref_data[d].oar_target_switch)
                delete dose_ref_data[d].target;
            else
                delete dose_ref_data[d].oar;
        }
        delete []dose_ref_data;
    }
}

void
RTPlan::setDicomDirectory( char *buffer )
{
    dicom_dir.clear();
    dicom_dir = buffer;
}
void
RTPlan::setDicomFilename( char *buffer )
{
    dicom_full_filename.clear();
    dicom_full_filename = buffer;
}


bool
RTPlan::loadDicomInfo()
{
    struct dirent *dp = NULL;
    DIR *dfd = NULL;

    if ((dfd = opendir(dicom_dir.data()) ) == NULL)
    {
        printf("\n Error: can't open %s\n", dicom_dir.data());
        return 0;
    }

    bool rtstruct_file_found = false;
    while ((dp = readdir(dfd)) != NULL  && !rtstruct_file_found)
    {
        if ( strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0 )
            continue;
        else //if( strstr(dp->d_name,".dcm") != NULL )
        {
            rtstruct_file_found = importSOPClassUID(dp->d_name);
            if ( rtstruct_file_found )
            {
                dicom_full_filename = dicom_dir.data();
                dicom_full_filename += "/";
                dicom_full_filename += dp->d_name;
                printf(" RTPLAN file found. -> %s\n", dicom_full_filename.data());
            }
        }
    }
    closedir(dfd);

    if ( rtstruct_file_found )
        importPatientInfo();
    else
        printf("\n No RTPLAN file found.");

    return rtstruct_file_found;
}

bool
RTPlan::importSOPClassUID( char *buffer )
{
    char *filename;
    filename = new char[ dicom_dir.size() + strlen(buffer) + 2];
    sprintf(filename,"%s/%s", dicom_dir.data(), buffer);

    DcmFileFormat format;
    OFCondition status = format.loadFile( filename );
    if (status.bad())
    {
        printf("\n Error reading DICOM file:\n\t%s\n", filename );
        delete []filename;
        return 0;
    }

    DcmDataset *dataset = format.getDataset();

    bool series_is_rtstruct = false;
    OFString ptSOPCLASSUID;
    if (dataset->findAndGetOFString(DCM_SOPClassUID, ptSOPCLASSUID).good())
        if(0 == ptSOPCLASSUID.compare(RTPLAN_SOP_CLASS_UID) )
        {
            pt_sop_class_uid = ptSOPCLASSUID.data();
            series_is_rtstruct = true;
        }

    delete []filename;
    return series_is_rtstruct;
}

void
RTPlan::importPatientInfo()
{
    DcmFileFormat format;
    OFCondition status = format.loadFile( dicom_full_filename.data() );
    if (status.bad())
    {
        printf("\n Error reading DICOM file:\n\t%s\n", dicom_full_filename.data() );
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
    printf("\n");
}


void
RTPlan::loadRTPlanData()
{
    DRTPlan dcmrt_plan;
    OFCondition status = dcmrt_plan.loadFile( dicom_full_filename.data() );
    if (status.good())
    {
        OFString rxDESCRIPTION;
        status = dcmrt_plan.getPrescriptionDescription( rxDESCRIPTION, 0 );
        if (status.good())
        {
            rx_description = rxDESCRIPTION.data();
            printf("\n Rx Description: %s",rx_description.data());
        }

        Sint32 fractionCOUNT;
        if (dcmrt_plan.getFractionGroupSequence().getItem(0).getNumberOfFractionsPlanned( fractionCOUNT, 0 ).good() )
        {
            fraction_count = fractionCOUNT;
            printf("\n %d Fractions.",fraction_count);

            Sint32 beamCOUNT;
            if (dcmrt_plan.getFractionGroupSequence().getItem(0).getNumberOfBeams(beamCOUNT, 0).good() )
            {
                beam_count = beamCOUNT;
            }
            Float64 rxDOSE;
            if (dcmrt_plan.getFractionGroupSequence().getItem(0).getReferencedDoseReferenceSequence().getItem(0).getTargetPrescriptionDose(rxDOSE, 0).good() )
            {
                rx_dose_levels.x = rxDOSE;
                printf("\n Target Rx Dose - %f",rx_dose_levels.x);
            }
            Sint32 refDOSEREF;
            if (dcmrt_plan.getFractionGroupSequence().getItem(0).getReferencedDoseReferenceSequence().getItem(0).getReferencedDoseReferenceNumber(refDOSEREF, 0).good() )
            {
                refd_rx_dose_ref_number.x = refDOSEREF;
                printf("\n Referenced Dose Reference Number - %d",refd_rx_dose_ref_number.x);
            }
        }

        if (beam_count == 0)
            beam_count = dcmrt_plan.getBeamSequence().getNumberOfItems();
        printf("\n\n %d Beams.",beam_count);
        beam_data = new BEAM_DATA[beam_count];
        for (uint b=0; b<beam_count; b++)
        {
            Sint32 beamNUMBER;
            if (dcmrt_plan.getBeamSequence().getItem(b).getBeamNumber(beamNUMBER, 0).good() )
            {
                beam_data[b].number = beamNUMBER;
                printf("\n   Beam %d:",beam_data[b].number);
            }
            OFString beamDESCRIPTION;
            if (dcmrt_plan.getBeamSequence().getItem(b).getBeamDescription( beamDESCRIPTION, 0 ).good() )
            {
                beam_data[b].description = beamDESCRIPTION.data();
                printf("\n   Beam Description - %s",beam_data[b].description.data());
            }
            Float64 SAD;
            if (dcmrt_plan.getBeamSequence().getItem(b).getSourceAxisDistance( SAD, 0 ).good() )
            {
                beam_data[b].sad = SAD;
                printf("\n   Source-Axis Distance - %f",beam_data[b].sad);
            }
        }

        Float64 v;
        if (dcmrt_plan.getBeamSequence().getItem(0).getControlPointSequence().getItem(0).getIsocenterPosition( v, 0 ).good() )
            isocenter.x = v;
        if (dcmrt_plan.getBeamSequence().getItem(0).getControlPointSequence().getItem(0).getIsocenterPosition( v, 1 ).good() )
            isocenter.y = v;
        if (dcmrt_plan.getBeamSequence().getItem(0).getControlPointSequence().getItem(0).getIsocenterPosition( v, 2 ).good() )
            isocenter.z = v;
        printf("\n\n Isocenter: ( %f , %f , %f )",isocenter.x,isocenter.y,isocenter.z);

        dose_ref_count = dcmrt_plan.getDoseReferenceSequence().getNumberOfItems();
        printf("\n\n %d Dose Reference Items.",dose_ref_count);
        dose_ref_data = new DOSE_REF_DATA[dose_ref_count];
        for (uint d=0; d<dose_ref_count; d++)
        {
            Sint32 refNUMBER;
            if (dcmrt_plan.getDoseReferenceSequence().getItem(d).getDoseReferenceNumber( refNUMBER, 0 ).good() )
            {
                dose_ref_data[d].number = refNUMBER;
                printf("\n   Dose Reference Number %d:",dose_ref_data[d].number);
            }
            Sint32 roiREFNUMBER;
            if (dcmrt_plan.getDoseReferenceSequence().getItem(d).getReferencedROINumber( roiREFNUMBER, 0 ).good() )
            {
                dose_ref_data[d].ref_roi_number = roiREFNUMBER;
                printf("\n     Referenced ROI Number - %d",dose_ref_data[d].ref_roi_number);
            }
            Float64 constraintWEIGHT;
            if (dcmrt_plan.getDoseReferenceSequence().getItem(d).getConstraintWeight( constraintWEIGHT, 0 ).good() )
            {
                dose_ref_data[d].constraint_weight = constraintWEIGHT;
                printf("\n     Constraint Weight - %f",dose_ref_data[d].constraint_weight);
            }
            OFString refDESCRIPTION;
            if (dcmrt_plan.getDoseReferenceSequence().getItem(d).getDoseReferenceDescription( refDESCRIPTION, 0 ).good() )
            {
                dose_ref_data[d].description = refDESCRIPTION.data();
                printf("\n     Dose Reference Description - %s",dose_ref_data[d].description.data());
            }
            OFString refTYPE;
            if (dcmrt_plan.getDoseReferenceSequence().getItem(d).getDoseReferenceType( refTYPE, 0 ).good() )
            {
                dose_ref_data[d].type = refTYPE.data();
                printf("\n     Dose Reference Type - %s",dose_ref_data[d].type.data());

                dose_ref_data[d].oar_target_switch = false;
                if ( dose_ref_data[d].type.find("TARGET") >= 0 )
                {
                    dose_ref_data[d].oar_target_switch = true;
                    dose_ref_data[d].target = new TARGET_DATA;

                    Float64 rxDOSE;
                    if (dcmrt_plan.getDoseReferenceSequence().getItem(d).getTargetPrescriptionDose( rxDOSE, 0 ).good() )
                    {
                        dose_ref_data[d].target->rx_dose = rxDOSE;
                        printf("\n      Target Rx Dose - %f",dose_ref_data[d].target->rx_dose);
                        if (dose_ref_data[d].target->rx_dose > rx_dose_levels.z)
                        {
                            if (dose_ref_data[d].target->rx_dose > rx_dose_levels.y)
                            {
                                if (dose_ref_data[d].target->rx_dose >= rx_dose_levels.x)
                                {
                                    rx_dose_levels.x = dose_ref_data[d].target->rx_dose;
                                    refd_rx_dose_ref_number.x = dose_ref_data[d].number;
                                }
                                else
                                {
                                    rx_dose_levels.y = dose_ref_data[d].target->rx_dose;
                                    refd_rx_dose_ref_number.y = dose_ref_data[d].number;
                                }
                            }
                            else
                            {
                                rx_dose_levels.z = dose_ref_data[d].target->rx_dose;
                                refd_rx_dose_ref_number.z = dose_ref_data[d].number;
                            }
                        }
                    }
                    Float64 maxDOSE;
                    if (dcmrt_plan.getDoseReferenceSequence().getItem(d).getTargetMaximumDose( maxDOSE, 0 ).good() )
                    {
                        dose_ref_data[d].target->max_dose = maxDOSE;
                        printf("\n      Target Maximum Dose - %f",dose_ref_data[d].target->max_dose);
                    }
                    Float64 minDOSE;
                    if (dcmrt_plan.getDoseReferenceSequence().getItem(d).getTargetMinimumDose( minDOSE, 0 ).good() )
                    {
                        dose_ref_data[d].target->min_dose = minDOSE;
                        printf("\n      Target Minimum Dose - %f",dose_ref_data[d].target->min_dose);
                    }
                    Float64 underdoseVOLUME;
                    if (dcmrt_plan.getDoseReferenceSequence().getItem(d).getTargetUnderdoseVolumeFraction( underdoseVOLUME, 0 ).good() )
                    {
                        dose_ref_data[d].target->underdose_volume_fraction = underdoseVOLUME;
                        printf("\n      Target Underdose Volume Fraction - %f",dose_ref_data[d].target->underdose_volume_fraction);
                    }
                }
                else if ( dose_ref_data[d].type.find("ORGAN_AT_RISK") >= 0 )
                {
                    dose_ref_data[d].oar = new OAR_DATA;

                    Float64 fullVolDOSE;
                    if (dcmrt_plan.getDoseReferenceSequence().getItem(d).getOrganAtRiskFullVolumeDose( fullVolDOSE, 0 ).good() )
                    {
                        dose_ref_data[d].oar->full_volume_dose = fullVolDOSE;
                        printf("\n      OAR Full Volume Dose - %f",dose_ref_data[d].oar->full_volume_dose);
                    }
                    Float64 maxDOSE;
                    if (dcmrt_plan.getDoseReferenceSequence().getItem(d).getOrganAtRiskMaximumDose( maxDOSE, 0 ).good() )
                    {
                        dose_ref_data[d].oar->max_dose = maxDOSE;
                        printf("\n      OAR Maximum Dose - %f",dose_ref_data[d].oar->max_dose);
                    }
                    Float64 overdoseVOLUME;
                    if (dcmrt_plan.getDoseReferenceSequence().getItem(d).getOrganAtRiskOverdoseVolumeFraction( overdoseVOLUME, 0 ).good() )
                    {
                        dose_ref_data[d].oar->overdose_volume_fraction = overdoseVOLUME;
                        printf("\n      OAR Overdose Volume Fraction - %f",dose_ref_data[d].oar->overdose_volume_fraction);
                    }
                }
            }
        }

        printf("\n Rx Dose Levels: %f // %f // %f",rx_dose_levels.x,rx_dose_levels.y,rx_dose_levels.z);
    }
    printf("\n\n");
}
















