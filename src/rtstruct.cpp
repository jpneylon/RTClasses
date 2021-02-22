#include "rtstruct.h"

RTStruct::RTStruct()
{
    roi_count = 0;
    rtstruct_file_found = false;
}
RTStruct::~RTStruct()
{
    if (rtstruct_file_found)
    {
        if (roi_count > 0)
        {
            for (uint r=0; r < roi_count; r++ )
            {
                if ( roi_array[r].load_data && roi_array[r].sub_cntr_count > 0)
                {
                    for (uint s=0; s < roi_array[r].sub_cntr_count; s++)
                        delete []roi_array[r].sub_cntr_data[s].points;
                    delete []roi_array[r].sub_cntr_data;
                    delete []roi_array[r].sub_cntr_points_count;
                }
            }
            delete []roi_array;
        }
    }
}


void
RTStruct::setDicomDirectory( const char *buffer )
{
    dicom_dir.clear();
    dicom_dir = buffer;
}
void
RTStruct::setDicomFilename( char *buffer )
{
    dicom_full_filename.clear();
    dicom_full_filename = buffer;
}

void
RTStruct::setSubCntrPoint( uint r, uint s, uint p, float v_x, float v_y, float v_z )
{
    if (p == 0 && s == 0)
    {
        roi_array[r].range_min.x = v_x;
        roi_array[r].range_min.y = v_y;
        roi_array[r].range_min.z = v_z;

        roi_array[r].range_max.x = v_x;
        roi_array[r].range_max.y = v_y;
        roi_array[r].range_max.z = v_z;
    }
    else
    {
        if (v_x < roi_array[r].range_min.x)
            roi_array[r].range_min.x = v_x;

        if (v_y < roi_array[r].range_min.y)
            roi_array[r].range_min.y = v_y;

        if (v_z < roi_array[r].range_min.z)
            roi_array[r].range_min.z = v_z;

        if (v_x > roi_array[r].range_max.x)
            roi_array[r].range_max.x = v_x;

        if (v_y > roi_array[r].range_max.y)
            roi_array[r].range_max.y = v_y;

        if (v_z > roi_array[r].range_max.z)
            roi_array[r].range_max.z = v_z;
    }

    roi_array[r].sub_cntr_data[s].points[3*p] = v_x;
    roi_array[r].sub_cntr_data[s].points[3*p+1] = v_y;
    roi_array[r].sub_cntr_data[s].points[3*p+2] = v_z;
}
rtfloat3
RTStruct::getSubCntrPoint( uint r, uint s, uint p )
{
    rtfloat3 point;
    point.x = roi_array[r].sub_cntr_data[s].points[3*p];
    point.y = roi_array[r].sub_cntr_data[s].points[3*p+1];
    point.z = roi_array[r].sub_cntr_data[s].points[3*p+2];
    return point;
}

bool
RTStruct::loadDicomInfo()
{
    struct dirent *dp = NULL;
    DIR *dfd = NULL;

    if ((dfd = opendir(dicom_dir.data()) ) == NULL)
    {
        printf("\n Error: can't open %s\n", dicom_dir.data());
        return 0;
    }

    while ((dp = readdir(dfd)) != NULL  && !rtstruct_file_found)
    {
        if ( strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0 )
            continue;
        else //if (strstr(dp->d_name,".dcm")!=NULL)
        {
            rtstruct_file_found = importSOPClassUID(dp->d_name);
            if ( rtstruct_file_found )
            {
                dicom_full_filename = dicom_dir.data();
                dicom_full_filename += "/";
                dicom_full_filename += dp->d_name;
                printf(" RTSTRUCT file found. -> %s\n", dicom_full_filename.data());
            }
        }
    }
    closedir(dfd);

    if ( rtstruct_file_found )
        importPatientInfo();
    else
        printf("\n No RTSTRUCT file found.");

    return rtstruct_file_found;
}

bool
RTStruct::importSOPClassUID( char *buffer )
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
        if(0 == ptSOPCLASSUID.compare(RTSTRUCT_SOP_CLASS_UID) )
        {
            pt_sop_class_uid = ptSOPCLASSUID.data();
            series_is_rtstruct = true;
        }

    delete []filename;
    return series_is_rtstruct;
}
void
RTStruct::importPatientInfo()
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
RTStruct::loadRTStructInfo()
{
    DRTStructureSet dcmrt_struct;
    OFCondition status = dcmrt_struct.loadFile( dicom_full_filename.data() );
    if (status.good())
    {
        DRTStructureSetROISequence ROIsequence = dcmrt_struct.getStructureSetROISequence();
        roi_count = ROIsequence.getNumberOfItems();
        printf("\n %d Contours.",roi_count);
        roi_array = new ROI_DATA[roi_count];

        DRTROIContourSequence roiContourSequence = dcmrt_struct.getROIContourSequence();

        for (uint r=0; r < roi_count; r++)
        {
            DRTStructureSetROISequence::Item roi_seq_item = ROIsequence.getItem(r);

            Sint32 roiNUMBER;
            roi_seq_item.getROINumber( roiNUMBER, 0);
            roi_array[r].roi_number = roiNUMBER;
            roi_array[r].load_data = false;

            OFString roiNAME;
            roi_seq_item.getROIName( roiNAME, 0 );
            roi_array[r].roi_name = roiNAME.data();
            printf("\n %d. %s:", roi_array[r].roi_number, roi_array[r].roi_name.data() );

            DRTROIContourSequence::Item roiContourSequenceItem = roiContourSequence.getItem(r);

            Sint32 roiCOLOR;
            roiContourSequenceItem.getROIDisplayColor( roiCOLOR, 0);
            roi_array[r].roi_rgb_color.x = roiCOLOR;
            roiContourSequenceItem.getROIDisplayColor( roiCOLOR, 1);
            roi_array[r].roi_rgb_color.y = roiCOLOR;
            roiContourSequenceItem.getROIDisplayColor( roiCOLOR, 2);
            roi_array[r].roi_rgb_color.z = roiCOLOR;
            printf(" (%d,%d,%d) ", roi_array[r].roi_rgb_color.x, roi_array[r].roi_rgb_color.y, roi_array[r].roi_rgb_color.z);

            DRTContourSequence contourSequence = roiContourSequenceItem.getContourSequence();

            if ( contourSequence.isValid() )
            {
                roi_array[r].sub_cntr_count = contourSequence.getNumberOfItems();

                if (roi_array[r].sub_cntr_count > 0)
                {
                    roi_array[r].total_points_count = 0;
                    roi_array[r].sub_cntr_points_count = new uint[ roi_array[r].sub_cntr_count ];

                    for (uint sc = 0; sc < roi_array[r].sub_cntr_count; sc++)
                    {
                        DRTContourSequence::Item contourSequenceItem = contourSequence.getItem(sc);

                        Sint32 contourPointsCount;
                        contourSequenceItem.getNumberOfContourPoints( contourPointsCount, 0 );

                        roi_array[r].sub_cntr_points_count[sc] = contourPointsCount;
                        roi_array[r].total_points_count += contourPointsCount;
                    }
                    printf(" %d data points...", roi_array[r].total_points_count);
                }
                else
                {
                    printf(" aborting.\n");
                }
            }
            else
            {
                printf(" failed to load ROI sequence. Aborting.\n");
            }
        }
    }
    printf("\n\n");
}

void
RTStruct::chooseContours()
{
    for (uint c=0; c<roi_count; c++)
        printf("\n %d - %s", roi_array[c].roi_number, roi_array[c].roi_name.data() );

    printf("\n\nEnter the number of the first ROI to load: ");

    char temp[8];
    scanf("%s",temp);
    while( strcmp(temp,"x")!=0 )
    {
        int i;
        sscanf(temp,"%d",&i);
        bool selection_found = false;
        for (uint c = 0; c < roi_count; c++)
            if ( roi_array[c].roi_number == i )
            {
                selection_found = true;
                roi_array[c].load_data = true;
                printf("Loading %s. Enter the next ROI, or enter 'x' to proceed: ",roi_array[c].roi_name.data());
            }
        if (!selection_found) printf("No ROI corresponds with that number. Enter another ROI, or enter 'x' to proceed: ");
        scanf("%s",temp);
    }
    printf("\n");
}

int
RTStruct::loadRTStructData( int r )
{
    DRTStructureSet dcmrt_struct;
    OFCondition status = dcmrt_struct.loadFile( dicom_full_filename.data() );
    if (status.good())
    {
        uint roi = r;

        printf("\n %d. Loading %s...", roi_array[roi].roi_number, roi_array[roi].roi_name.data() );

        DRTROIContourSequence roiContourSequence = dcmrt_struct.getROIContourSequence();

        DRTROIContourSequence::Item roiContourSequenceItem = roiContourSequence.getItem(roi);

        DRTContourSequence contourSequence = roiContourSequenceItem.getContourSequence();

        if ( contourSequence.isValid() )
        {
            printf("%d sub-cntrs,",roi_array[roi].sub_cntr_count);
            if (roi_array[roi].sub_cntr_count > 0)
            {
                roi_array[roi].sub_cntr_data = new CNTR_DATA[ roi_array[roi].sub_cntr_count ];

                for (uint sc = 0; sc < roi_array[roi].sub_cntr_count; sc++)
                {
                    DRTContourSequence::Item contourSequenceItem = contourSequence.getItem(sc);

                    Sint32 contourPointsCount;
                    contourSequenceItem.getNumberOfContourPoints( contourPointsCount, 0 );
                    roi_array[roi].sub_cntr_data[sc].points = new float[ 3 * contourPointsCount ];

                    OFVector<double> cntrData;
                    status = contourSequenceItem.getContourData( cntrData );
                    if (status.good())
                        for (uint scpc = 0; scpc < roi_array[roi].sub_cntr_points_count[sc]; scpc++)
                            setSubCntrPoint( roi, sc, scpc, cntrData[3*scpc], cntrData[3*scpc+1], cntrData[3*scpc+2] );
                }
                printf(" %d data points...", roi_array[roi].total_points_count);
                printf(" range [ %f, %f ], [ %f, %f ], [ %f, %f ] ...", roi_array[roi].range_min.x,
                                                                        roi_array[roi].range_max.x,
                                                                        roi_array[roi].range_min.y,
                                                                        roi_array[roi].range_max.y,
                                                                        roi_array[roi].range_min.z,
                                                                        roi_array[roi].range_max.z);
            }
            else
            {
                printf(" aborting.\n");
                return -1;
            }
        }
        else
        {
            printf(" failed to load ROI sequence. Aborting.\n");
            return 0;
        }

        printf(" done.\n");
        return 1;
    }
    else
        return 0;
}

void
RTStruct::copyROI( int r, ROI_DATA *roi_copy, int c )
{
    roi_copy[c].roi_number = c;
    roi_copy[c].roi_name = roi_array[r].roi_name.data();
    roi_copy[c].load_data = true;
    roi_copy[c].roi_rgb_color.x = roi_array[r].roi_rgb_color.x;
    roi_copy[c].roi_rgb_color.y = roi_array[r].roi_rgb_color.y;
    roi_copy[c].roi_rgb_color.z = roi_array[r].roi_rgb_color.z;
    roi_copy[c].total_points_count = roi_array[r].total_points_count;
    roi_copy[c].sub_cntr_count = roi_array[r].sub_cntr_count;
    roi_copy[c].sub_cntr_points_count = new uint[ roi_copy[c].sub_cntr_count ];
    roi_copy[c].sub_cntr_data = new RTStruct::CNTR_DATA[ roi_copy[c].sub_cntr_count ];

    for (uint s=0; s < roi_copy[c].sub_cntr_count; s++ )
    {
        roi_copy[c].sub_cntr_points_count[s] = roi_array[r].sub_cntr_points_count[s];
        roi_copy[c].sub_cntr_data[s].points = new float[ 3*roi_copy[c].sub_cntr_points_count[s] ];
        memcpy( roi_copy[c].sub_cntr_data[s].points, roi_array[r].sub_cntr_data[s].points, 3 * roi_copy[c].sub_cntr_points_count[s] * sizeof(float) );
    }
}

void
RTStruct::freeROI( ROI_DATA *roi_copy, int c )
{
    for (uint s=0; s < roi_copy[c].sub_cntr_count; s++ )
        delete []roi_copy[c].sub_cntr_data[s].points;
    delete []roi_copy[c].sub_cntr_points_count;
    delete []roi_copy[c].sub_cntr_data;
}


void
RTStruct::saveRTStructData( const char *outpath, ROI_DATA *new_roi_data, uint new_roi_count, bool anonymize_switch )
{
    DcmFileFormat format;
    OFCondition status = format.loadFile( dicom_full_filename.data() );
    if (status.bad())
    {
        printf("\n Error reading DICOM file:\n\t%s\n", dicom_full_filename.data() );
    }

    if (anonymize_switch)
        anonymize( format.getDataset() );

    time_t rawtime;
    struct tm * timeinfo;
    char buffer[64];
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    strftime (buffer,32,"%G%m%d",timeinfo);
    format.getDataset()->putAndInsertString(DCM_AcquisitionDate,buffer).good();
    format.getDataset()->putAndInsertString(DCM_SeriesDate,buffer).good();
    printf("  Creation date written...\n");
    fflush(stdout);

    for (uint i=0; i<roi_count-new_roi_count; i++)
    {
        printf("\n deleting %s (%d)...", roi_array[roi_count-1-i].roi_name.data(), roi_array[roi_count-1-i].roi_number);
        if (format.getDataset()->findAndDeleteSequenceItem(DCM_StructureSetROISequence, -1).good() )
            printf("Structure Set ROI Sequence deleted...");
        if (format.getDataset()->findAndDeleteSequenceItem(DCM_ROIContourSequence, -1).good() )
            printf("ROI Contour Sequence deleted...");
        if (format.getDataset()->findAndDeleteSequenceItem(DCM_RTROIObservationsSequence, -1).good() )
            printf("RT ROI Observation Sequence deleted...");
    }

    printf("\n\n");
    for (uint i=0; i<new_roi_count; i++)
    {
        printf(" %d. %s: Color(%d,%d,%d) | %d Sub Cntrs | %d Total Points\n", new_roi_data[i].roi_number, new_roi_data[i].roi_name.data(),
               new_roi_data[i].roi_rgb_color.x, new_roi_data[i].roi_rgb_color.y, new_roi_data[i].roi_rgb_color.z, new_roi_data[i].sub_cntr_count, new_roi_data[i].total_points_count);

        char newNumber[8];
        sprintf(newNumber,"%d",new_roi_data[i].roi_number);

        OFString newCOLOR;
        char newColor[32];
        sprintf(newColor,"%d//%d//%d",new_roi_data[i].roi_rgb_color.x,new_roi_data[i].roi_rgb_color.y,new_roi_data[i].roi_rgb_color.z);
        newCOLOR.assign( (const char*)newColor );

        DcmItem *roi_contourSequenceItem;
        format.getDataset()->findAndGetSequenceItem(DCM_ROIContourSequence, roi_contourSequenceItem, i );
        roi_contourSequenceItem->putAndInsertOFStringArray(DCM_ROIDisplayColor, newCOLOR, OFTrue );
        roi_contourSequenceItem->putAndInsertString(DCM_ReferencedROINumber, newNumber, OFTrue );

        DcmSequenceOfItems *contourSequence;
        roi_contourSequenceItem->findAndGetSequence(DCM_ContourSequence, contourSequence);
        uint cntrSeqLength = contourSequence->card();
        uint loop_size = cntrSeqLength < new_roi_data[i].sub_cntr_count ? cntrSeqLength : new_roi_data[i].sub_cntr_count;
        printf("\n %d : %d ? %d\n\n", cntrSeqLength, new_roi_data[i].sub_cntr_count, loop_size );
        for (uint s=0; s<loop_size; s++)
        {
            DcmItem *contourSequenceItem;
            contourSequenceItem = contourSequence->getItem(s);

            char cntrPoints[16];
            sprintf(cntrPoints,"%d",new_roi_data[i].sub_cntr_points_count[s]);
            contourSequenceItem->putAndInsertString(DCM_NumberOfContourPoints, cntrPoints );

            Float32 *POINTS;
            POINTS = new Float32[ 3*new_roi_data[i].sub_cntr_points_count[s] ];
            for (uint p=0; p < 3*new_roi_data[i].sub_cntr_points_count[s]; p++)
                POINTS[p] = new_roi_data[i].sub_cntr_data[s].points[p];
            contourSequenceItem->putAndInsertFloat32Array(DCM_ContourData, POINTS, 3*new_roi_data[i].sub_cntr_points_count[s], OFTrue );
            // doesnt seem to be working...
            delete []POINTS;
        }

        DcmItem *structureSetSequenceItem;
        format.getDataset()->findAndGetSequenceItem(DCM_StructureSetROISequence, structureSetSequenceItem, i );
        structureSetSequenceItem->putAndInsertString(DCM_ROIName, new_roi_data[i].roi_name.data() );
        structureSetSequenceItem->putAndInsertString(DCM_ROINumber, newNumber );

        DcmItem *observationSequenceItem;
        format.getDataset()->findAndGetSequenceItem(DCM_RTROIObservationsSequence, observationSequenceItem, i );
        observationSequenceItem->putAndInsertString(DCM_ROIObservationLabel, new_roi_data[i].roi_name.data() );
        observationSequenceItem->putAndInsertString(DCM_ObservationNumber, newNumber );
        observationSequenceItem->putAndInsertString(DCM_ReferencedROINumber, newNumber );
    }

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

    DRTStructureSetIOD rtstruct_template;
    status = rtstruct_template.read(*format.getDataset());

    DRTStructureSetIOD rtstruct_new(rtstruct_template);

    format.clear();
    status = rtstruct_new.write(*format.getDataset());
    if (status.good())
    {
        char outfilename[512];
        sprintf(outfilename,"%s/RS.new_structure_set.dcm",outpath);
        status = format.saveFile( outfilename );
        if (status.bad())
            printf("Error: cannot write DICOM file ( %s )",status.text() );
    }
}


void
RTStruct::saveRTStructData( const char *outpath, bool anonymize_switch )
{
    DcmFileFormat format;
    OFCondition status = format.loadFile( dicom_full_filename.data() );
    if (status.bad())
    {
        printf("\n Error reading DICOM file:\n\t%s\n", dicom_full_filename.data() );
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

    DRTStructureSetIOD rtstruct_template;
    status = rtstruct_template.read(*format.getDataset());

    DRTStructureSetIOD rtstruct_new(rtstruct_template);

    format.clear();
    status = rtstruct_new.write(*format.getDataset());
    if (status.good())
    {
        char outfilename[512];
        sprintf(outfilename,"%s/RS.new_structure_set.dcm",outpath);
        status = format.saveFile( outfilename );
        if (status.bad())
            printf("Error: cannot write DICOM file ( %s )",status.text() );
    }
}


void
RTStruct::anonymize( DcmDataset *dataset )
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
