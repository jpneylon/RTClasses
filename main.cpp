// EXAMPLE USAGE TO LOAD RT DICOM DATA
// created by: Jack Neylon, PhD
// 2021-02-22

#include <string.h>
#include <strings.h>

#include <rtclasses/rtimages.h>
#include <rtclasses/rtdose.h>
#include <rtclasses/rtstruct.h>
#include <rtclasses/rtplan.h>

RTDose *rtdose = 0;
RTStruct *rtstruct = 0;
RTPlan *rtplan = 0;
RTImage *rtimages = 0;

inline int stringRemoveDelimiter(char delimiter, const char *string) {
  int string_start = 0;
  while (string[string_start] == delimiter) {
    string_start++;
  }
  if (string_start >= static_cast<int>(strlen(string) - 1)) {
    return 0;
  }
  return string_start;
}

inline bool rt_checkCmdLineFlag(const int argc, const char **argv,
                             const char *string_ref) {
  bool bFound = false;
  if (argc >= 1) {
    for (int i = 1; i < argc; i++) {
      int string_start = stringRemoveDelimiter('-', argv[i]);
      const char *string_argv = &argv[i][string_start];
      const char *equal_pos = strchr(string_argv, '=');
      int argv_length = static_cast<int>(
          equal_pos == 0 ? strlen(string_argv) : equal_pos - string_argv);
      int length = static_cast<int>(strlen(string_ref));
      if (length == argv_length && !strncasecmp(string_argv, string_ref, length)) {
        bFound = true;
        continue;
      }
    }
  }
  return bFound;
}

inline bool rt_getCmdLineArgumentString(const int argc, const char **argv,
                                     const char *string_ref,
                                     char **string_retval) {
  bool bFound = false;
  if (argc >= 1) {
    for (int i = 1; i < argc; i++) {
      int string_start = stringRemoveDelimiter('-', argv[i]);
      char *string_argv = const_cast<char*>(&argv[i][string_start]);
      int length = static_cast<int>(strlen(string_ref));

      if (!strncasecmp(string_argv, string_ref, length)) {
        *string_retval = &string_argv[length + 1];
        bFound = true;
        continue;
      }
    }
  }
  if (!bFound) {
    *string_retval = NULL;
  }
  return bFound;
}

/////////// Gives User Choice to Load Previously Calculated Data and use Visualization/Structure Analysis Tools Or Begin a New Sequence and Load Data
int main (int argc, char *argv[])
{
    bool load_rtdose = false;
    if (rt_checkCmdLineFlag( argc, (const char**)argv, "dose" ) )
    {
        load_rtdose = true;
        rtdose = new RTDose();
    }
    bool load_rtstruct = false;
    if (rt_checkCmdLineFlag( argc, (const char**)argv, "struct" ) )
    {
        load_rtstruct = true;
        rtstruct = new RTStruct();
    }
    bool load_rtplan = false;
    if (rt_checkCmdLineFlag( argc, (const char**)argv, "plan" ) )
    {
        load_rtplan = true;
        rtplan = new RTPlan();
    }
    bool load_rtimages = false;
    if (rt_checkCmdLineFlag( argc, (const char**)argv, "images") )
    {
        load_rtimages = true;
        rtimages = new RTImage();
    }


    if (rt_checkCmdLineFlag( argc, (const char**)argv, "input" ) )
    {
      char *input_directory;
      rt_getCmdLineArgumentString( argc, (const char**)argv, "input", &input_directory );

      if ( load_rtimages )
      {
          rtimages->setDicomDirectory(input_directory);
          printf("\n RTIMAGES Directory: %s\n", rtimages->getDicomDirectory() );
          if ( rtimages->loadDicomInfo() )
          {
              rtimages->loadRTImageData();
              if (rt_checkCmdLineFlag( argc, (const char**)argv, "imageout" ) )
              {
                  char *imgout_buffer;
                  rt_getCmdLineArgumentString( argc, (const char**)argv, "imageout", &imgout_buffer );

                  rtimages->saveRTImageData( imgout_buffer, rtimages->getDataArray(), true);
              }
          }
      }


      if ( load_rtdose )
      {
        rtdose->setDicomDirectory(input_directory);
        printf("\n RTDOSE Directory: %s\n", rtdose->getDicomDirectory() );
        if ( rtdose->loadDicomInfo() )
        {
            rtdose->loadRTDoseData();

            if (rt_checkCmdLineFlag( argc, (const char**)argv, "doseout" ) )
            {
                char *doseout_buffer;
                rt_getCmdLineArgumentString( argc, (const char**)argv, "doseout", &doseout_buffer );

                rtdose->saveRTDoseData( doseout_buffer, rtdose->getDataArray(), true);
            }
        }
      } // dose flag


      if ( load_rtstruct )
      {
        rtstruct->setDicomDirectory(input_directory);
        printf("\n RTSTRUCT Directory: %s\n", rtstruct->getDicomDirectory() );
        if ( rtstruct->loadDicomInfo() )
        {
            printf("\n load rtstruct sequence... \n");
            rtstruct->loadRTStructInfo();
            rtstruct->chooseContours();
            for ( uint c=0; c < rtstruct->getNumberOfROIs(); c++)
                if ( rtstruct->getROILoadDataSwitch(c) )
                    rtstruct->loadRTStructData(c);

            if (rt_checkCmdLineFlag( argc, (const char**)argv, "structout" ) )
            {
                char *structout_buffer;
                rt_getCmdLineArgumentString( argc, (const char**)argv, "structout", &structout_buffer );
                printf("\n RTSTRUCT Output Directory: %s\n", structout_buffer );

                uint new_roi_count = 0;
                for ( uint c=0; c < rtstruct->getNumberOfROIs(); c++)
                    if ( rtstruct->getROILoadDataSwitch(c) )
                        new_roi_count++;

                printf("\n New structure set created with %d ROIs.",new_roi_count);
                RTStruct::ROI_DATA *new_roi_array;
                new_roi_array = new RTStruct::ROI_DATA[new_roi_count];

                /* create new roi data or copy old roi data */
                uint n = 0;
                for ( uint c=0; c < rtstruct->getNumberOfROIs(); c++)
                    if ( rtstruct->getROILoadDataSwitch(c) )
                    {
                        rtstruct->copyROI( c, new_roi_array, n );
                        n++;
                    }
                rtstruct->saveRTStructData( structout_buffer, new_roi_array, new_roi_count, true);
                for (uint c=0; c < new_roi_count; c++ )
                    rtstruct->freeROI( new_roi_array, c );
                delete []new_roi_array;
            }
        }
      } // struct flag


      if ( load_rtplan )
      {
        rtplan->setDicomDirectory(input_directory);
        printf("\n RTPLAN Directory: %s\n", rtplan->getDicomDirectory() );
        if ( rtplan->loadDicomInfo() )
        {
            printf("\n load rtplan sequence... \n");
            rtplan->loadRTPlanData();
        }
      } // plan flag


    } // input flag


    if (load_rtdose)
        delete rtdose;
    if (load_rtstruct)
        delete rtstruct;
    if (load_rtplan)
        delete rtplan;
    if (load_rtimages)
        delete rtimages;


    printf("\n\n");
 return 1;
}
