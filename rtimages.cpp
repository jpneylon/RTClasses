#include "rtimages.h"

RTImage::RTImage() {}
RTImage::~RTImage()
{
    _finalize();
}
void
RTImage::_finalize()
{
    if (data_array)
        delete []data_array;

    if (image_count > 0)
    {
        delete []slice;
    }
}

void
RTImage::setDicomDirectory( char *buffer )
{
    dicom_dir.clear();
    dicom_dir = buffer;
}
void
RTImage::setSliceImagePositionPatient(uint i, float v_x, float v_y, float v_z)
{
    slice[i].image_position_patient.x = v_x;
    slice[i].image_position_patient.y = v_y;
    slice[i].image_position_patient.z = v_z;
}
void
RTImage::setSliceSOPInstanceUID( uint i, char *buffer )
{
    slice[i].sop_instance_uid.clear();
    slice[i].sop_instance_uid = buffer;
}
void
RTImage::setSliceFilename( uint i, char *buffer )
{
    slice[i].filename.clear();
    slice[i].filename = buffer;
}


bool
RTImage::loadDicomInfo()
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

    printf("\n %d RT Images Found.\n",image_count);
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

        bool keep_sorting = false;
        uint sortCount = image_count;
        do
        {
            keep_sorting = false;
            for( uint j=1; j<sortCount; j++)
            {
                if ( slice[j-1].instance_number > slice[j].instance_number )
                {
                    keep_sorting = true;
                    int tempint = slice[j].instance_number;
                    slice[j].instance_number = slice[j-1].instance_number;
                    slice[j-1].instance_number = tempint;

                    char *tempchar_j, *tempchar_j1;

                    tempchar_j = new char[ strlen(slice[j].filename.data()) + 1 ];
                    tempchar_j1 = new char[ strlen(slice[j-1].filename.data()) + 1 ];

                    sprintf( tempchar_j, "%s", slice[j].filename.data() );
                    sprintf( tempchar_j1, "%s", slice[j-1].filename.data() );

                    slice[j].filename.clear();
                    slice[j].filename = tempchar_j1;

                    slice[j-1].filename.clear();
                    slice[j-1].filename = tempchar_j;

                    delete []tempchar_j;
                    delete []tempchar_j1;
                }
            }
            sortCount--;
        }
        while (keep_sorting && sortCount > 1);

        importPatientInfo();

        //for ( uint i=0; i<image_count; i++)
        //    printf("\n   %d.\t%s",slice[i].instance_number,slice[i].filename.data());
    }

    return rtimageset_file_found;
}

bool
RTImage::importSOPClassUID( char *buffer )
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
        if(0 == ptSOPCLASSUID.compare(RTIMAGE_SOP_CLASS_UID) ||
                0 == ptSOPCLASSUID.compare(CTIMAGE_SOP_CLASS_UID)  )
        {
            series_is_rtimage = true;
        }

    delete []filename;
    return series_is_rtimage;
}

void
RTImage::importInstanceNumber( uint i )
{
    DcmFileFormat format;
    OFCondition status = format.loadFile( slice[i].filename.data() );
    if (status.good())
    {
        Sint32 instanceNUMBER;
        if (format.getDataset()->findAndGetSint32(DCM_InstanceNumber, instanceNUMBER).good())
        {
            slice[i].instance_number = instanceNUMBER;
        }
    }
    else
        printf("\n Error reading DICOM file:\n\t%s\n", slice[i].filename.data() );
}

void
RTImage::importPatientInfo()
{
    DcmFileFormat format;
    OFCondition status = format.loadFile( slice[0].filename.data() );
    if (status.bad())
    {
        printf("\n Error reading DICOM file:\n\t%s\n", slice[0].filename.data() );
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
       // printf("\n ACQUISITION DATE: %s",dicom_date.data());
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


int
RTImage::loadRTImageData()
{
    for (uint i=0; i<image_count; i++)
    {
        DcmFileFormat format;
        OFCondition status = format.loadFile( slice[i].filename.data() );
        if (status.good())
        {
            if (i == 0)
            {
                data_size.z = image_count;

                Uint16 width,height;
                if (format.getDataset()->findAndGetUint16(DCM_Columns,width).good())
                    data_size.x = width;
                if (format.getDataset()->findAndGetUint16(DCM_Rows,height).good())
                    data_size.y = height;

                Float64 pixelSPACING;
                if (format.getDataset()->findAndGetFloat64(DCM_PixelSpacing, pixelSPACING, 0).good())
                    voxel_size.x = pixelSPACING;
                if (format.getDataset()->findAndGetFloat64(DCM_PixelSpacing, pixelSPACING, 0).good())
                    voxel_size.y = pixelSPACING;
                if (format.getDataset()->findAndGetFloat64(DCM_SliceThickness, pixelSPACING).good())
                    voxel_size.z = pixelSPACING;

                Float64 imagePOSITION;
                if (format.getDataset()->findAndGetFloat64(DCM_ImagePositionPatient, imagePOSITION, 0).good())
                    data_origin.x = imagePOSITION;
                if (format.getDataset()->findAndGetFloat64(DCM_ImagePositionPatient, imagePOSITION, 1).good())
                    data_origin.y = imagePOSITION;
                if (format.getDataset()->findAndGetFloat64(DCM_ImagePositionPatient, imagePOSITION, 2).good())
                    data_origin.z = imagePOSITION;

                Float64 rescaleINTERCEPT;
                if (format.getDataset()->findAndGetFloat64(DCM_RescaleIntercept, rescaleINTERCEPT).good())
                    rescale_intercept = rescaleINTERCEPT;

                Float64 rescaleSLOPE;
                if (format.getDataset()->findAndGetFloat64(DCM_RescaleSlope, rescaleSLOPE).good())
                    rescale_slope = rescaleSLOPE;

                Float64 windowCENTER;
                if (format.getDataset()->findAndGetFloat64(DCM_WindowCenter, windowCENTER).good())
                    window_center = windowCENTER;

                Float64 windowWIDTH;
                if (format.getDataset()->findAndGetFloat64(DCM_WindowWidth, windowWIDTH).good())
                    window_width = windowWIDTH;

                printf("\n Data Dimensions: %d x %d x %d \n Voxel Dimensions: %2.3f x %2.3f x %2.3f",data_size.x,data_size.y,data_size.z,voxel_size.x,voxel_size.y,voxel_size.z);
                printf("\n DATA Position: %2.3f x %2.3f x %2.3f ",data_origin.x,data_origin.y,data_origin.z);
                printf("\n Window Center: %3.3f\n Window Width: %3.3f",window_center,window_width);
                printf("\n Rescale Slope: %2.2f\n Rescale Intercept: %3.3f",rescale_slope,rescale_intercept);

                data_array = new float[ data_size.x * data_size.y * data_size.z ];
                memset( data_array, 0, data_size.x * data_size.y * data_size.z * sizeof(float) );

                data_min = 9999;
                data_max = -9999;
            }

            Float64 imagePOSITION;
            if (format.getDataset()->findAndGetFloat64(DCM_ImagePositionPatient, imagePOSITION, 0).good())
                slice[i].image_position_patient.x = imagePOSITION;
            if (format.getDataset()->findAndGetFloat64(DCM_ImagePositionPatient, imagePOSITION, 1).good())
                slice[i].image_position_patient.y = imagePOSITION;
            if (format.getDataset()->findAndGetFloat64(DCM_ImagePositionPatient, imagePOSITION, 2).good())
                slice[i].image_position_patient.z = imagePOSITION;

            OFString instanceUID;
            if (format.getDataset()->findAndGetOFString(DCM_SOPInstanceUID, instanceUID).good() )
            {
                slice[i].sop_instance_uid = instanceUID.data();
            }
            OFString frameUID;
            if (format.getDataset()->findAndGetOFString(DCM_FrameOfReferenceUID, frameUID).good() )
            {
                slice[i].reference_frame_uid = frameUID.data();
            }

            const Uint16 *pixelData;
            if (format.getDataset()->findAndGetUint16Array(DCM_PixelData, pixelData, NULL, 0).good())
            {
                for (int y = 0; y < data_size.y; y++)
                    for (int x = 0; x < data_size.x; x++)
                    {
                        int p = x + data_size.x * y;
                        float value = (float)pixelData[p];
                        value *= rescale_slope;
                        value += rescale_intercept;
                        setArrayVoxel( x, y, i, value );
                        if (value > data_max) data_max = value;
                        if (value < data_min) data_min = value;
                    }
            }
            else
                printf("\n Error: cannot read image pixel data (%s)\n", status.text() );
        }
        else
            printf("\n Error: cannot read Image object (%s)\n", status.text() );
    }
    printf("\n Data Min: %4.3f\n Data Max: %4.3f\n\n",data_min,data_max);

    return 1;
}


int
RTImage::saveRTImageData( const char *outpath, float *newData, bool anonymize_switch )
{
    for (uint i=0; i<image_count; i++)
    {
        DcmFileFormat format;
        OFCondition status = format.loadFile( slice[i].filename.data() );
        if (status.good())
        {
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

            Uint16 width,height;
            width = data_size.x;
            height = data_size.y;
            dataset->putAndInsertUint16(DCM_Columns,width).good();
            dataset->putAndInsertUint16(DCM_Rows,height).good();

            Float64 vz;
            vz = voxel_size.z;
            dataset->putAndInsertFloat64(DCM_SliceThickness,vz).good();

            OFString vxy;
            char pixelSpacing[32];
            sprintf(pixelSpacing,"%3.3f\\%3.3f",voxel_size.x,voxel_size.y);
            vxy.assign( (const char*)pixelSpacing );
            dataset->putAndInsertOFStringArray(DCM_PixelSpacing,vxy,OFTrue).good();

            Sint32 instanceNUMBER;
            instanceNUMBER = slice[i].instance_number;
            dataset->putAndInsertSint32(DCM_InstanceNumber, instanceNUMBER).good();

            dataset->putAndInsertString(DCM_SOPInstanceUID, slice[i].sop_instance_uid.data());

            Float64 rescaleINTERCEPT;
            rescaleINTERCEPT = rescale_intercept;
            dataset->putAndInsertFloat64(DCM_RescaleIntercept, rescaleINTERCEPT);

            Float64 rescaleSLOPE;
            rescaleSLOPE = rescale_slope;
            dataset->putAndInsertFloat64(DCM_RescaleSlope, rescaleSLOPE);

            Uint16 *pixelDataOut;
            pixelDataOut = new Uint16[width*height];
            for (int y=0; y<data_size.y; y++)
                for (int x=0; x<data_size.x; x++)
                {
                    int p = x + data_size.x*y;
                    Uint16 temp;
                    temp = (Uint16)((getArrayVoxel(x,y,i) - rescale_intercept) / rescale_slope );
                    pixelDataOut[p] = temp;
                }

            status = dataset->putAndInsertUint16Array(DCM_PixelData, pixelDataOut, width*height, OFTrue);
            delete []pixelDataOut;

            mkdir((const char *)outpath,S_IRWXU|S_IRWXG|S_IRWXO);

            if (status.good())
            {
                char outfilename[512];
                sprintf(outfilename,"%s/NewDICOM_Image_Data_%d.dcm",outpath,i+1);
                //printf("  Saving %s ... \r",outfilename);
                status = format.saveFile( outfilename );
                if (status.bad())
                    printf("Error: cannot write DICOM file ( %s )",status.text() );
            }
        }
        else
            printf("\n Error: cannot read Image object (%s)\n", status.text() );
    }
    return 1;
}


void
RTImage::anonymize( DcmDataset *dataset )
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
    dataset->findAndDeleteElement(DCM_OtherPatientIDs, OFTrue, OFTrue);
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
    dataset->findAndDeleteElement(DCM_MedicalRecordLocator, OFTrue, OFTrue);
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











