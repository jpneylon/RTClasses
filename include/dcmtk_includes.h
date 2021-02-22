#ifndef __DCMTK_INCLUDES_H__
#define __DCMTK_INCLUDES_H__

#define HAVE_CONFIG_H

#include <string>
#include <dirent.h>
#include <sys/stat.h>

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmimgle/dcmimage.h"

namespace RTC 
{
    struct rtfloat3
    {
        float x;
        float y;
        float z;
    };

    struct rtint3
    {
        int x;
        int y;
        int z;
    };

    struct rtuint3
    {
        int x;
        int y;
        int z;
    };
}

#endif // __DCMTK_INCLUDES_H__
