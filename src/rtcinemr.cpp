#include "rtcinemr.h"

RTCineMR::RTCineMR()
{
    image_count = 0;
    phase_count = 0;
}
RTCineMR::~RTCineMR()
{
    if (image_count > 0)
    {
        delete [] slice;

        if (phase_count > 0)
        {
            for (uint p=0; p<phase_count; p++)
            {
                delete [] phases[p].slice;
                delete [] phases[p].data_array;
            }
            delete [] phases;
        }
    }
}

void
RTCineMR::setDicomDirectory( const char *buffer )
{
    dicom_dir.clear();
    dicom_dir = buffer;
}
void
RTCineMR::setSliceImagePositionPatient(uint i, uint p, float v_x, float v_y, float v_z)
{
    phases[p].slice[i].image_position_patient.x = v_x;
    phases[p].slice[i].image_position_patient.y = v_y;
    phases[p].slice[i].image_position_patient.z = v_z;
}
void
RTCineMR::setSliceSOPInstanceUID( uint i, char *buffer )
{
    slice[i].sop_instance_uid.clear();
    slice[i].sop_instance_uid = buffer;
}
void
RTCineMR::setSliceFilename( uint i, char *buffer )
{
    slice[i].filename.clear();
    slice[i].filename = buffer;
}

void
RTCineMR::copySliceDataToPhaseSlice( uint s, uint p, uint ps)
{
    phases[p].slice[ps].filename = slice[s].filename;
    phases[p].slice[ps].sop_instance_uid = slice[s].sop_instance_uid;
    phases[p].slice[ps].reference_frame_uid = slice[s].reference_frame_uid;

    phases[p].slice[ps].instance_number = slice[s].instance_number;
    phases[p].slice[ps].slice_location = slice[s].slice_location;
    //phases[p].slice[ps].image_position_patient.x = slice[s].image_position_patient.x;
    //phases[p].slice[ps].image_position_patient.y = slice[s].image_position_patient.y;
    //phases[p].slice[ps].image_position_patient.z = slice[s].image_position_patient.z;
}

bool
RTCineMR::loadDicomInfo()
{
    struct dirent *dp = NULL;
    DIR *dfd = NULL;

    if ((dfd = opendir(dicom_dir.data()) ) == NULL)
    {
        printf("\n Error: can't open %s\n", dicom_dir.data());
        return 0;
    }

    bool rtimageset_file_found = false;
    image_count = 0;
    while ((dp = readdir(dfd)) != NULL)
    {
        if ( strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0 )
            continue;
        else //if( strstr(dp->d_name,".dcm") != NULL )
        {
            if ( importSOPClassUID(dp->d_name) )
                image_count++;
        }
    }
    closedir(dfd);

    printf("\n %d RT Images Found. Type: %d\n",image_count,sop_type);
    if (image_count > 0)
    {
        rtimageset_file_found = true;
        slice = new SLICE_DATA[image_count];

        dfd = opendir(dicom_dir.data());
        uint current_file = 0;
        while ((dp = readdir(dfd)) != NULL)
        {
            if ( strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0 )
                continue;
            else //if (strstr(dp->d_name,".dcm")!=NULL)
            {
                if ( importSOPClassUID(dp->d_name) )
                {
                    slice[current_file].filename = dicom_dir.data();
                    slice[current_file].filename += "/";
                    slice[current_file].filename += dp->d_name;
                    importInstanceNumber(current_file);
                    current_file++;
                }
            }
        }
        closedir(dfd);

        phases = new PHASE_DATA[phase_count];
        for (uint p=0; p<phase_count; p++)
        {
            phases[p].image_count = 0;
            phases[p].slice_count = 0;
        }
        images_per_phase = image_count / phase_count;
        printf("\n Expected Images per Phase = %d / %d = %d",image_count,phase_count,images_per_phase);
        // separate slices into phases
        for (uint s=0; s<image_count; s++)
        {
            uint slice_phase = slice[s].instance_number - 1;
            phases[slice_phase].image_count++;
        }
        for (uint p=0; p<phase_count; p++)
        {
            printf("\n %d slices found for phase %d", phases[p].image_count, p);
            if (phases[p].image_count > images_per_phase)
                images_per_phase = phases[p].image_count;
        }
        for (uint p=0; p<phase_count; p++)
        {
            phases[p].phase_id = p;
            phases[p].slice = new SLICE_DATA[images_per_phase];
        }
        for (uint s=0; s<image_count; s++)
        {
            uint slice_phase = slice[s].instance_number - 1;
            copySliceDataToPhaseSlice( s, slice_phase, phases[slice_phase].slice_count);
            phases[slice_phase].slice_count++;
        }

        // sort slices within phases
        for (uint p=0; p<phase_count; p++)
        {
            bool keep_sorting = false;
            uint sortCount = phases[p].slice_count;
            do
            {
                keep_sorting = false;
                for( uint j=1; j<sortCount; j++)
                {
                    if ( phases[p].slice[j-1].slice_location < phases[p].slice[j].slice_location )
                    {
                        keep_sorting = true;
                        float tempfloat = phases[p].slice[j].slice_location;
                        phases[p].slice[j].slice_location = phases[p].slice[j-1].slice_location;
                        phases[p].slice[j-1].slice_location = tempfloat;

                        char *tempchar_j, *tempchar_j1;

                        tempchar_j = new char[ strlen(phases[p].slice[j].filename.data()) + 1 ];
                        tempchar_j1 = new char[ strlen(phases[p].slice[j-1].filename.data()) + 1 ];

                        sprintf( tempchar_j, "%s", phases[p].slice[j].filename.data() );
                        sprintf( tempchar_j1, "%s", phases[p].slice[j-1].filename.data() );

                        phases[p].slice[j].filename.clear();
                        phases[p].slice[j].filename = tempchar_j1;

                        phases[p].slice[j-1].filename.clear();
                        phases[p].slice[j-1].filename = tempchar_j;

                        delete []tempchar_j;
                        delete []tempchar_j1;
                    }
                }
                sortCount--;
            }
            while (keep_sorting && sortCount > 1);

            //importPatientInfo(p);
        }

        //for ( uint i=0; i<image_count; i++)
        //    printf("\n   %d.\t%s",slice[i].instance_number,slice[i].filename.data());
    }

    return rtimageset_file_found;
}

bool
RTCineMR::importSOPClassUID( char *buffer )
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
        return false;
    }

    bool series_is_rtimage = false;
    OFString ptSOPCLASSUID;
    if (format.getDataset()->findAndGetOFString(DCM_SOPClassUID, ptSOPCLASSUID).good())
    {
        if(0 == ptSOPCLASSUID.compare(RTIMAGE_SOP_CLASS_UID))
        {
            series_is_rtimage = true;
            sop_type = SOP_RTIMAGE;
        }
        else if (0 == ptSOPCLASSUID.compare(CTIMAGE_SOP_CLASS_UID))
        {
            series_is_rtimage = true;
            sop_type = SOP_CT;
        }
        else if (0 == ptSOPCLASSUID.compare(MRIMAGE_SOP_CLASS_UID))
        {
            series_is_rtimage = true;
            sop_type = SOP_MR;
        }
        //else
        //    printf("\n %s ",ptSOPCLASSUID.data());
    }

    delete []filename;
    return series_is_rtimage;
}

void
RTCineMR::importInstanceNumber( uint i )
{
    DcmFileFormat format;
    OFCondition status = format.loadFile( slice[i].filename.data() );
    if (status.good())
    {
        Sint32 cardiacNUMBER;
        if (format.getDataset()->findAndGetSint32(DCM_CardiacNumberOfImages, cardiacNUMBER).good())
            if (cardiacNUMBER > phase_count)
            {
                printf("\n Cardiac # of Images (%d) > current phase count (%d) \n",cardiacNUMBER,phase_count);
                phase_count = cardiacNUMBER;
            }

        Sint32 instanceNUMBER;
        if (format.getDataset()->findAndGetSint32(DCM_InstanceNumber, instanceNUMBER).good())
            slice[i].instance_number = instanceNUMBER;

        Float64 imagePOSITION;
        if (format.getDataset()->findAndGetFloat64(DCM_ImagePositionPatient, imagePOSITION, 2).good())
            slice[i].slice_location = imagePOSITION;
    }
    else
        printf("\n Error reading DICOM file:\n\t%s\n", slice[i].filename.data() );
}

void
RTCineMR::importPatientInfo( uint p )
{
    DcmFileFormat format;
    OFCondition status = format.loadFile( phases[p].slice[0].filename.data() );
    if (status.bad())
    {
        printf("\n Error reading DICOM file:\n\t%s\n", phases[p].slice[0].filename.data() );
        return;
    }

    DcmDataset *dataset = format.getDataset();
    OFString ptNAME;
    if (dataset->findAndGetOFString(DCM_PatientName, ptNAME).good())
    {
        phases[p].pt_name = ptNAME.data();
        //printf("\n PT NAME: %s",pt_name.data());
    }
    OFString ptID;
    if (dataset->findAndGetOFString(DCM_PatientID, ptID).good())
    {
        phases[p].pt_id = ptID.data();
        //printf("\n PT ID: %s",pt_id.data());
    }
    OFString DICOMDATE;
    if (dataset->findAndGetOFString(DCM_StudyDate,DICOMDATE).good())
    {
        phases[p].dicom_date = DICOMDATE.data();
        //printf("\n ACQUISITION DATE: %s",dicom_date.data());
    }
    else if (dataset->findAndGetOFString(DCM_SeriesDate,DICOMDATE).good())
    {
        phases[p].dicom_date = DICOMDATE.data();
       // printf("\n ACQUISITION DATE: %s",dicom_date.data());
    }
    else if (dataset->findAndGetOFString(DCM_AcquisitionDate,DICOMDATE).good())
    {
        phases[p].dicom_date = DICOMDATE.data();
        //printf("\n ACQUISITION DATE: %s",dicom_date.data());
    }
    OFString ptSERIESDESCRIPTION;
    if (dataset->findAndGetOFString(DCM_SeriesDescription, ptSERIESDESCRIPTION).good())
    {
        phases[p].pt_series_description = ptSERIESDESCRIPTION.data();
        //printf("\n PT SERIES DESCRIPTION: %s", pt_series_description.data());
    }
    OFString ptSTUDYID;
    if (dataset->findAndGetOFString(DCM_StudyID, ptSTUDYID).good())
    {
        phases[p].pt_study_id = ptSTUDYID.data();
        //printf("\n PT STUDY ID: %s",pt_study_id.data());
    }
    OFString ptSTUDYINSTANCEUID;
    if (dataset->findAndGetOFString(DCM_StudyInstanceUID, ptSTUDYINSTANCEUID).good())
    {
        phases[p].pt_study_instance_uid = ptSTUDYINSTANCEUID.data();
        //printf("\n PT STUDY INSTANCE UID: %s",pt_study_instance_uid.data());
    }
    OFString ptSERIESINSTANCEUID;
    if (dataset->findAndGetOFString(DCM_SeriesInstanceUID, ptSERIESINSTANCEUID).good())
    {
        phases[p].pt_series_instance_uid = ptSERIESINSTANCEUID.data();
        //printf("\n PT SERIES INSTANCE UID: %s",pt_series_instance_uid.data());
    }
    printf("\n");
}

int
RTCineMR::loadRTImageData( uint p )
{
    for (uint i=0; i<phases[p].image_count; i++)
    {
        DcmFileFormat format;
        OFCondition status = format.loadFile( phases[p].slice[i].filename.data() );
        if (status.good())
        {
            if (i == 0)
            {
                phases[p].data_size.z = phases[p].image_count;

                Uint16 width,height;
                if (format.getDataset()->findAndGetUint16(DCM_Columns,width).good())
                    phases[p].data_size.x = width;
                if (format.getDataset()->findAndGetUint16(DCM_Rows,height).good())
                    phases[p].data_size.y = height;

                Float64 pixelSPACING;
                if (format.getDataset()->findAndGetFloat64(DCM_PixelSpacing, pixelSPACING, 0).good())
                    phases[p].voxel_size.x = pixelSPACING;
                if (format.getDataset()->findAndGetFloat64(DCM_PixelSpacing, pixelSPACING, 0).good())
                    phases[p].voxel_size.y = pixelSPACING;
                if (format.getDataset()->findAndGetFloat64(DCM_SliceThickness, pixelSPACING).good())
                    phases[p].voxel_size.z = pixelSPACING;

                Float64 imagePOSITION;
                if (format.getDataset()->findAndGetFloat64(DCM_ImagePositionPatient, imagePOSITION, 0).good())
                    phases[p].data_origin.x = imagePOSITION;
                if (format.getDataset()->findAndGetFloat64(DCM_ImagePositionPatient, imagePOSITION, 1).good())
                    phases[p].data_origin.y = imagePOSITION;
                if (format.getDataset()->findAndGetFloat64(DCM_ImagePositionPatient, imagePOSITION, 2).good())
                    phases[p].data_origin.z = imagePOSITION;

                Float64 imageORIENT;
                if (format.getDataset()->findAndGetFloat64(DCM_ImageOrientationPatient, imageORIENT, 0).good())
                    phases[p].orient_x.x = imageORIENT;
                if (format.getDataset()->findAndGetFloat64(DCM_ImageOrientationPatient, imageORIENT, 1).good())
                    phases[p].orient_x.y = imageORIENT;
                if (format.getDataset()->findAndGetFloat64(DCM_ImageOrientationPatient, imageORIENT, 2).good())
                    phases[p].orient_x.z = imageORIENT;
                if (format.getDataset()->findAndGetFloat64(DCM_ImageOrientationPatient, imageORIENT, 3).good())
                    phases[p].orient_y.x = imageORIENT;
                if (format.getDataset()->findAndGetFloat64(DCM_ImageOrientationPatient, imageORIENT, 4).good())
                    phases[p].orient_y.y = imageORIENT;
                if (format.getDataset()->findAndGetFloat64(DCM_ImageOrientationPatient, imageORIENT, 5).good())
                    phases[p].orient_y.z = imageORIENT;

                printf("\n Data Dimensions: %d x %d x %d \n Voxel Dimensions: %2.3f x %2.3f x %2.3f",phases[p].data_size.x,phases[p].data_size.y,phases[p].data_size.z,phases[p].voxel_size.x,phases[p].voxel_size.y,phases[p].voxel_size.z);
                printf("\n DATA Position: %2.3f x %2.3f x %2.3f ",phases[p].data_origin.x,phases[p].data_origin.y,phases[p].data_origin.z);
                printf("\n DATA Orientation X: %2.3f x %2.3f x %2.3f ",phases[p].orient_x.x,phases[p].orient_x.y,phases[p].orient_x.z);
                printf("\n DATA Orientation Y: %2.3f x %2.3f x %2.3f ",phases[p].orient_y.x,phases[p].orient_y.y,phases[p].orient_y.z);

                phases[p].rescale_intercept = 0.f;
                Float64 rescaleINTERCEPT;
                if (format.getDataset()->findAndGetFloat64(DCM_RescaleIntercept, rescaleINTERCEPT).good())
                    phases[p].rescale_intercept = rescaleINTERCEPT;

                phases[p].rescale_slope = 1.f;
                Float64 rescaleSLOPE;
                if (format.getDataset()->findAndGetFloat64(DCM_RescaleSlope, rescaleSLOPE).good())
                    phases[p].rescale_slope = rescaleSLOPE;

                Float64 windowCENTER;
                if (format.getDataset()->findAndGetFloat64(DCM_WindowCenter, windowCENTER).good())
                    phases[p].window_center = windowCENTER;

                Float64 windowWIDTH;
                if (format.getDataset()->findAndGetFloat64(DCM_WindowWidth, windowWIDTH).good())
                    phases[p].window_width = windowWIDTH;

                printf("\n Window Center: %3.3f\n Window Width: %3.3f",phases[p].window_center,phases[p].window_width);
                printf("\n Rescale Slope: %2.2f\n Rescale Intercept: %3.3f\n",phases[p].rescale_slope,phases[p].rescale_intercept);

                phases[p].data_array = new float[ phases[p].data_size.x * phases[p].data_size.y * phases[p].data_size.z ];
                memset( phases[p].data_array, 0, phases[p].data_size.x * phases[p].data_size.y * phases[p].data_size.z * sizeof(float) );

                phases[p].data_min = 9999;
                phases[p].data_max = -9999;
            }

            Float64 imagePOSITION;
            if (format.getDataset()->findAndGetFloat64(DCM_ImagePositionPatient, imagePOSITION, 0).good())
                phases[p].slice[i].image_position_patient.x = imagePOSITION;
            if (format.getDataset()->findAndGetFloat64(DCM_ImagePositionPatient, imagePOSITION, 1).good())
                phases[p].slice[i].image_position_patient.y = imagePOSITION;
            if (format.getDataset()->findAndGetFloat64(DCM_ImagePositionPatient, imagePOSITION, 2).good())
                phases[p].slice[i].image_position_patient.z = imagePOSITION;

            OFString instanceUID;
            if (format.getDataset()->findAndGetOFString(DCM_SOPInstanceUID, instanceUID).good() )
            {
                phases[p].slice[i].sop_instance_uid = instanceUID.data();
            }
            OFString frameUID;
            if (format.getDataset()->findAndGetOFString(DCM_FrameOfReferenceUID, frameUID).good() )
            {
                phases[p].slice[i].reference_frame_uid = frameUID.data();
            }

            const Uint16 *pixelData;
            if (format.getDataset()->findAndGetUint16Array(DCM_PixelData, pixelData, NULL, 0).good())
            {
                for (int y = 0; y < phases[p].data_size.y; y++)
                    for (int x = 0; x < phases[p].data_size.x; x++)
                    {
                        int p = x + phases[p].data_size.x * y;
                        float value = (float)pixelData[p];

                        value *= phases[p].rescale_slope;
                        value += phases[p].rescale_intercept;
                        setArrayVoxel( x, y, i, p, value );

                        if (value > phases[p].data_max) phases[p].data_max = value;
                        if (value < phases[p].data_min) phases[p].data_min = value;
                    }
            }
            else
                printf("\n Error: cannot read image pixel data (%s)\n", status.text() );
        }
        else
            printf("\n Error: cannot read Image object (%s)\n", status.text() );
    }
    printf("\n Phase %d - Data Min: %4.3f\n Data Max: %4.3f\n\n",p,phases[p].data_min,phases[p].data_max);

    return 1;
}

void
RTCineMR::saveRTPhaseData( const char *outpath, uint p, bool anonymize_switch )
{
    for (uint i=0; i<phases[p].image_count; i++)
    {
        DcmFileFormat format;
        OFCondition status = format.loadFile( phases[p].slice[i].filename.c_str() );
        if (status.good())
        {
            DcmDataset *dataset = format.getDataset();

            if (anonymize_switch)
                anonymize( dataset );

            time_t rawtime;
            struct tm *timeinfo;
            char buffer[32];
            time (&rawtime);
            timeinfo = localtime (&rawtime);
            strftime(buffer,32,"%G%m%d",timeinfo);
            //format.getDataset()->putAndInsertString(DCM_AcquisitionDate,buffer).good();
            format.getDataset()->putAndInsertString(DCM_SeriesDate,buffer).good();
            fflush(stdout);

            mkdir((const char *)outpath,S_IRWXU|S_IRWXG|S_IRWXO);

            char *outfilename = new char[ strlen(outpath) + 32 ];
            sprintf(outfilename,"%s/New_DICOM_Image_Data_%d.dcm",outpath,i);
            status = format.saveFile( outfilename );
            if (status.bad())
                printf("Error: cannot write DICOM file ( %s )", status.text() );
        }
        else
            printf("\n Error: cannot read Image object (%s)\n", status.text() );
    }
    return;
}


void
RTCineMR::anonymize( DcmDataset *dataset )
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











