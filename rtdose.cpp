#include "rtdose.h"

RTDose::RTDose() {}
RTDose::~RTDose()
{
    _finalize();
}
void
RTDose::_finalize()
{
    //if (data_array)
    //    delete []data_array;
}

void
RTDose::setDicomDirectory( char *buffer )
{
    dicom_dir.clear();
    dicom_dir = buffer;
}
void
RTDose::setDicomFilename( char *buffer )
{
    dicom_full_filename.clear();
    dicom_full_filename = buffer;
}


bool
RTDose::loadDicomInfo()
{
    struct dirent *dp = NULL;
    DIR *dfd = NULL;

    if ((dfd = opendir(dicom_dir.data()) ) == NULL)
    {
        printf("\n Error: can't open %s\n", dicom_dir.data());
        return 0;
    }

    bool rtdose_file_found = false;
    while ((dp = readdir(dfd)) != NULL && !rtdose_file_found)
    {
        if ( strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0 )
            continue;
        else //if( strstr(dp->d_name,".dcm") != NULL )
        {
            rtdose_file_found = importSOPClassUID(dp->d_name);
            if ( rtdose_file_found )
            {
                dicom_full_filename = dicom_dir.data();
                dicom_full_filename += "/";
                dicom_full_filename += dp->d_name;
                printf(" RTDOSE file found. -> %s\n", dicom_full_filename.data());
            }
        }
    }
    closedir(dfd);

    if ( rtdose_file_found )
        importPatientInfo();
    else
        printf("\n No RTDOSE file found.");

    return rtdose_file_found;
}

bool
RTDose::importSOPClassUID( char *buffer )
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

    bool series_is_rtdose = false;
    OFString ptSOPCLASSUID;
    if (dataset->findAndGetOFString(DCM_SOPClassUID, ptSOPCLASSUID).good())
        if(0 == ptSOPCLASSUID.compare(RTDOSE_SOP_CLASS_UID) )
        {
            pt_sop_class_uid = ptSOPCLASSUID.data();
            series_is_rtdose = true;
        }

    delete []filename;
    return series_is_rtdose;
}

void
RTDose::importPatientInfo()
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
    OFString ptREFFRAMEUID;
    if (dataset->findAndGetOFString(DCM_FrameOfReferenceUID, ptREFFRAMEUID).good())
    {
        reference_frame_uid = ptREFFRAMEUID.data();
        //printf("\n REFERENCE FRAME UID: %s",pt_series_instance_uid.data());
    }
    printf("\n");
}



int
RTDose::loadRTDoseData()
{
    DRTDose dcmrt_dose;
    OFCondition status = dcmrt_dose.loadFile( dicom_full_filename.data() );
    if (status.good())
    {
        Uint16 width,height;
        dcmrt_dose.getColumns(width,0);
        dcmrt_dose.getRows(height,0);
        data_size.x = width;
        data_size.y = height;

        Float64 v;
        dcmrt_dose.getPixelSpacing(v,0);
        voxel_size.x = v;
        dcmrt_dose.getPixelSpacing(v,1);
        voxel_size.y = v;

        dcmrt_dose.getGridFrameOffsetVector(v,1);
        float frame0 = v;
        dcmrt_dose.getGridFrameOffsetVector(v,0);
        float frame1 = v;
        voxel_size.z = fabs(frame1 - frame0);

        dcmrt_dose.getImagePositionPatient(v,0);
        data_origin.x = v;
        dcmrt_dose.getImagePositionPatient(v,1);
        data_origin.y = v;
        dcmrt_dose.getImagePositionPatient(v,2);
        data_origin.z = v;

        Float64 doseGridScaler;
        dcmrt_dose.getDoseGridScaling( doseGridScaler, 0 );
        data_scaler = doseGridScaler;
        printf("\n Dose Scalar: %f", data_scaler);

        OFString doseType, doseUnits;
        dcmrt_dose.getDoseType( doseType, 0 );
        printf("\n DOSE TYPE: %s",doseType.data());
        dcmrt_dose.getDoseUnits( doseUnits, 0 );
        printf("\n DOSE UNITS: %s",doseUnits.data());

        Sint32 frameNumber;
        dcmrt_dose.getNumberOfFrames( frameNumber, 0 );
        printf("\n NUMBER OF FRAMES: %d",frameNumber);
        int numberOfFrames = frameNumber;
        data_size.z = numberOfFrames;

        printf("\n REF FRAME UID: %s",reference_frame_uid.data() );

        Uint16 bitsStored;
        dcmrt_dose.getBitsStored( bitsStored, 0 );
        printf("\n BITS STORED: %d",bitsStored);

        Uint16 pixelRepresentation;
        dcmrt_dose.getPixelRepresentation( pixelRepresentation, 0 );
        printf("\n PIXEL REPRESENTATION: %d",pixelRepresentation);

        printf("\n Data Dimensions: %d x %d x %d \n Voxel Dimensions: %2.3f x %2.3f x %2.3f",data_size.x,data_size.y,data_size.z,voxel_size.x,voxel_size.y,voxel_size.z);
        printf("\n DATA Position: %2.3f x %2.3f x %2.3f ",data_origin.x,data_origin.y,data_origin.z);

        data_array = new float[width*height*numberOfFrames];

        float min = 0, max = 0;
        OFVector< OFVector<double> > doseImage;
        status = dcmrt_dose.getDoseImages( doseImage );
        if (status.good())
        {
            for (int z=0; z<numberOfFrames; z++)
                for (int y = 0; y < dcmrt_dose.getDoseImageHeight(); ++y)
                {
                    for (int x = 0; x < dcmrt_dose.getDoseImageWidth(); ++x)
                    {
                        int p = x + dcmrt_dose.getDoseImageWidth() * y;
                        float value = doseImage.at(z).at(p);
                        setArrayVoxel( x, y, z, value );
                        if (value > max) max = value;
                    }
                }
        }
        else
            printf("\n Error: cannot access Dose Image (%s)\n", status.text() );

        data_min = min;
        data_max = max;
        printf("\n Data Min: %4.3f\n Data Max: %4.3f\n\n",data_min,data_max);
        fflush(stdout);
    }
    else
        printf("\n Error: cannot read RT Dose object (%s)\n", status.text() );

    return 1;
}


/*
export_dicom_data is launched once for each slice to written. It creates a DICOM file in the indicated output path, load a template header from the incidated input path,
applies an anonymization to the header, writes the 3D volume parameters from the dataVol.params structure to the header file, and writes the pixel data from the newData source.
The file is then saved to the DICOM directory.
*/
int
RTDose::saveRTDoseData( const char *outpath, float *newData, bool anonymize_switch )
{
    DcmFileFormat format;
    OFCondition status = format.loadFile( dicom_full_filename.data() );
    if (status.bad())
    {
        printf("\n Error reading DICOM file:\n\t%s\n", dicom_full_filename.data() );
        return 0;
    }

    if (anonymize_switch)
        anonymize( format.getDataset() );

    Uint16 width,height;
    width = data_size.x;
    height = data_size.y;
    format.getDataset()->putAndInsertUint16(DCM_Columns,width).good();
    format.getDataset()->putAndInsertUint16(DCM_Rows,height).good();
    //printf("  Data size written...\n");
    fflush(stdout);

    Float64 vz;
    vz = voxel_size.z;
    format.getDataset()->putAndInsertFloat64(DCM_SliceThickness,vz).good();

    OFString vxy;
    char pixelSpacing[32];
    sprintf(pixelSpacing,"%3.3f\\%3.3f",voxel_size.x,voxel_size.y);
    vxy.assign( (const char*)pixelSpacing );
    format.getDataset()->putAndInsertOFStringArray(DCM_PixelSpacing,vxy,OFTrue).good();
    //printf("  Voxel size written...\n");
    fflush(stdout);

    time_t rawtime;
    struct tm * timeinfo;
    char buffer[64];
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    strftime (buffer,32,"%G%m%d",timeinfo);
    format.getDataset()->putAndInsertString(DCM_AcquisitionDate,buffer).good();
    format.getDataset()->putAndInsertString(DCM_SeriesDate,buffer).good();
    //printf("  Creation date written...\n");
    fflush(stdout);

    //sprintf(buffer,"DOSE");
    //format.getDataset()->putAndInsertString(DCM_StudyDescription,buffer).good();
    //format.getDataset()->putAndInsertString(DCM_SeriesDescription,buffer).good();

    double new_data_scaler = (double)data_max / UINT16_MAX;
    char scalerString[64];
    sprintf(scalerString,"%g",new_data_scaler);
    format.getDataset()->putAndInsertString(DCM_DoseGridScaling, scalerString).good();
    //printf("  Data scaler written (%g)...\n",new_data_scaler);
    fflush(stdout);

    uint bit_depth = 16;
    char bitsStored[8];
    sprintf(bitsStored,"%d",bit_depth);
    format.getDataset()->putAndInsertString(DCM_BitsStored, bitsStored).good();
    //printf("  Bits stored written...\n");
    fflush(stdout);

    char bitsAllocated[8];
    sprintf(bitsAllocated,"%d",bit_depth);
    format.getDataset()->putAndInsertString(DCM_BitsAllocated, bitsAllocated).good();
    //printf("  Bits allocated written...\n");
    fflush(stdout);

    char highBit[8];
    sprintf(highBit,"%d",bit_depth-1);
    format.getDataset()->putAndInsertString(DCM_HighBit, highBit).good();
    //printf("  Bit depth written...\n");
    fflush(stdout);

    OFString frameNumber;
    char frameString[32];
    sprintf(frameString,"%d",data_size.z);
    frameNumber.assign( (const char *)frameString );
    format.getDataset()->putAndInsertOFStringArray(DCM_NumberOfFrames, frameNumber, OFTrue).good();
    //printf("  Number of frames written...\n");
    fflush(stdout);

    Uint16 *pixelDataOut;
    pixelDataOut = (Uint16*)malloc(width * height * data_size.z * sizeof(Uint16));
    //printf("  Pixel data allocated...\n");
    fflush(stdout);

    for (int k=0; k<data_size.z; k++)
        for (int j=0; j<data_size.y; j++)
            for (int i=0; i<data_size.x; i++)
            {
                int p = i + data_size.x*(j + k*data_size.y);
                Uint16 temp;
                temp = (uint)abs( floor(newData[p] / new_data_scaler) );
                pixelDataOut[p] = temp;
            }
    //printf("  Pixel data recorded...\n");
    //fflush(stdout);

    status = format.getDataset()->putAndInsertUint16Array(DCM_PixelData, pixelDataOut, width*height*data_size.z, OFTrue);
    //if (status.good()) printf("  Pixel data written...\n");
    free(pixelDataOut);
    fflush(stdout);

    //printf("\n %s",outpath);
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
        sprintf(outfilename,"%s/RD.new_data_set.dcm",outpath);
        status = format.saveFile( outfilename );
        if (status.bad())
            printf("Error: cannot write DICOM file ( %s )",status.text() );
    }

    return 1;
}



/*
anonymize ensures no patient data is included when exporting
DICOM data in the export_dicom_data function
*/
void
RTDose::anonymize( DcmDataset *dataset )
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




