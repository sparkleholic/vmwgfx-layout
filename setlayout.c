#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#define DRM_DRIVER_NAME "vmwgfx"

static int width = 1920;
static int height = 1080;

/** DRM version structure. */
struct DRMVERSION
{
    int cMajor;
    int cMinor;
    int cPatchLevel;
    size_t cbName;
    char *pszName;
    size_t cbDate;
    char *pszDate;
    size_t cbDescription;
    char *pszDescription;
};

struct DRMVMWUPDATELAYOUT {
        uint32_t cOutputs;
        uint32_t u32Pad;
        uint64_t ptrRects;
};

struct DRMVMWRECT
{
        int32_t x;
        int32_t y;
        uint32_t w;
        uint32_t h;
};

#define DRM_IOCTL_VERSION _IOWR('d', 0x00, struct DRMVERSION)
#define DRM_IOCTL_VMW_UPDATE_LAYOUT \
        _IOW('d', 0x40 + 20, struct DRMVMWUPDATELAYOUT)


void freeDrmVersion(struct DRMVERSION* version) {
    if (!version)
        return;
    free(version->pszDate);
    free(version->pszName);
    free(version->pszDescription);
    free(version);
}

int isSVGADriver(int fd) {
    int rc =0;
    struct DRMVERSION version = {};
    char szName[sizeof(DRM_DRIVER_NAME)];
    version.cbName = sizeof(szName);
    version.pszName = szName;
    rc = ioctl(fd, DRM_IOCTL_VERSION, &version, sizeof(version), NULL);
    if (rc) {
        return rc;
    }
    if ( version.cbName < 1 || strncmp(version.pszName, DRM_DRIVER_NAME, version.cbName) != 0 ) {
        rc = 1;
    }
    // printf("name : %s \n", version.pszName);
    return rc;
}


int openSVGADrmFd() {
    int fd;
    unsigned int i;
    for (i = 64, fd = -1; i < 192; ++i)
    {
        char szPath[64];
        int rc;

        if (i < 128)
            rc = snprintf(szPath, sizeof(szPath), "/dev/dri/controlD%u", i);
        else
            rc = snprintf(szPath, sizeof(szPath), "/dev/dri/renderD%u", i);
        fd = open(szPath, O_RDWR);
        if (fd == -1)
            continue;
        rc = isSVGADriver(fd);
        if (!rc)
            break;
    }
    return fd;
}

int setResolution(int fd, int32_t x, int32_t y, uint32_t w, uint32_t h) {
    int rc;
    struct DRMVMWUPDATELAYOUT ioctlLayout;
    struct DRMVMWRECT rect = {
        x, y, w, h
    };
    ioctlLayout.cOutputs = 1;
    ioctlLayout.ptrRects = (uint64_t)(&rect);
    rc = ioctl(fd, DRM_IOCTL_VMW_UPDATE_LAYOUT, &ioctlLayout, sizeof(ioctlLayout), NULL);
    return rc;
}

void usage(int argc, char *argv[]) {
    printf("usage: \n");
    printf("%s -w width -h height", argv[0]);
    printf("\t(e.g. %s -w 1920 -h 1080)\n", argv[0]);
    exit(1);
}

int parse_opt(int argc, char *argv[]) {
    int rc = 0;
    const struct option long_options[] = {
        {"width",   1,  0,  'w'},
        {"height",   1,  0,  'h'},
        { 0, 0, 0, 0},
    };

    for(;;) {
        int index = 0;
        int c = getopt_long(argc, argv, "w:h:", long_options, &index);
        if (c == -1)
            break;
        switch (c) {
        case 'w':
            width = atoi(optarg);
            break;
        case 'h':
            height = atoi(optarg);
            break;
        default:
            usage(argc, argv);
            break;
        }
    }
    return rc;
}

int main(int argc, char* argv[]) {
    int rc = EXIT_SUCCESS;
    int fd = -1;

    /* options */
    rc = parse_opt(argc, argv);
    if (rc) {
        printf("Parsing error about options .\n");
        return rc;
    }

    fd = openSVGADrmFd();
    if (fd == -1)  {
       printf("This is not %s DRM driver. Hence No need to set a default layout.\n", DRM_DRIVER_NAME);
       return rc;
    }
    rc = setResolution(fd, 0, 0, width, height);
    if (rc) {
       printf("Resolution Setting Error. (errno: %d)\n", errno);
       return rc;
    }

    return rc;
}
