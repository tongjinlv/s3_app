#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <time.h>
#include "vdecoder.h"
#include "memoryAdapter.h"
#include "FormatConvert.h"

#define DEMO_FILE_NAME_LEN 256
#define ALIGN_16B(x) (((x) + (15)) & ~(15))

void loge(char* fmt,...)
{
	printf(fmt);
}
void logv(char* fmt,...)
{
	printf(fmt);
}
void logw(char* fmt,...)
{
        printf(fmt);
}
extern int gettimeofday(struct timeval *tv, struct timezone *tz);
static long long GetNowUs()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return now.tv_sec * 1000000 + now.tv_usec;
}

long long time1=0;
long long time2=0;
long long time3=0;

typedef struct {
    char             intput_file[256];
    char             output_file[256];

    unsigned int  decode_frame_num;
    unsigned int  decode_format;

    unsigned int src_size;

    unsigned int src_width;
    unsigned int src_height;

    int bit_rate;
    int frame_rate;
    int maxKeyFrame;
}decode_param_t;

typedef enum {
    INPUT,
    HELP,
    DECODE_FORMAT,
    OUTPUT,
    SRC_SIZE,
    INVALID
}ARGUMENT_T;

typedef struct {
    char Short[8];
    char Name[128];
    ARGUMENT_T argument;
    char Description[512];
}argument_t;

static const argument_t ArgumentMapping[] =
{
    { "-h",  "--help",    HELP,
        "Print this help" },
    { "-i",  "--input",   INPUT,
        "Input file path" },
    { "-f",  "--decode_format",  DECODE_FORMAT,
        "0:h264 decoder, 1:jpeg decoder" },
    { "-o",  "--output",  OUTPUT,
        "output file path" },
    { "-s",  "--srcsize",  SRC_SIZE,
        "src_size,can be 1440,1080,720,480" },
};
void logd(char *fmt,...)
{
	printf(fmt);
}
int yu12_nv12(unsigned int width, unsigned int height, unsigned char *addr_uv,
          unsigned char *addr_tmp_uv)
{
    unsigned int i, chroma_bytes;
    unsigned char *u_addr = NULL;
    unsigned char *v_addr = NULL;
    unsigned char *tmp_addr = NULL;

    chroma_bytes = width*height/4;

    u_addr = addr_uv;
    v_addr = addr_uv + chroma_bytes;
    tmp_addr = addr_tmp_uv;

    for(i=0; i<chroma_bytes; i++)
    {
        *(tmp_addr++) = *(u_addr++);
        *(tmp_addr++) = *(v_addr++);
    }

    memcpy(addr_uv, addr_tmp_uv, chroma_bytes*2);

    return 0;
}

ARGUMENT_T GetArgument(char *name)
{
    int i = 0;
    int num = sizeof(ArgumentMapping) / sizeof(argument_t);
    while(i < num)
    {
        if((0 == strcmp(ArgumentMapping[i].Name, name)) ||
            ((0 == strcmp(ArgumentMapping[i].Short, name)) &&
             (0 != strcmp(ArgumentMapping[i].Short, "--"))))
        {
            return ArgumentMapping[i].argument;
        }
        i++;
    }
    return INVALID;
}

static void PrintDemoUsage(void)
{
    int i = 0;
    int num = sizeof(ArgumentMapping) / sizeof(argument_t);
    printf("Usage:");
    while(i < num)
    {
        printf("%s %-32s %s", ArgumentMapping[i].Short, ArgumentMapping[i].Name,
                ArgumentMapping[i].Description);
        printf("\n");
        i++;
    }
}

void ParseArgument(decode_param_t *decode_param, char *argument, char *value)
{
    ARGUMENT_T arg;

    arg = GetArgument(argument);

    switch(arg)
    {
        case HELP:
            PrintDemoUsage();
            exit(-1);
        case INPUT:
            memset(decode_param->intput_file, 0, sizeof(decode_param->intput_file));
            sscanf(value, "%255s", decode_param->intput_file);
            logd(" get input file: %s ", decode_param->intput_file);
            break;
        case DECODE_FORMAT:
            sscanf(value, "%u", &decode_param->decode_format);
            break;
        case OUTPUT:
            memset(decode_param->output_file, 0, sizeof(decode_param->output_file));
            sscanf(value, "%255s", decode_param->output_file);
            logd(" get output file: %s ", decode_param->output_file);
            break;
        case SRC_SIZE:
            sscanf(value, "%u", &decode_param->src_size);
            logd(" get src_size: %dp ", decode_param->src_size);
            if(decode_param->src_size == 1440)
            {
                decode_param->src_width = 2560;
                decode_param->src_height = 1440;
            }
            else if(decode_param->src_size == 1080)
            {
                decode_param->src_width = 1920;
                decode_param->src_height = 1080;
            }
            else if(decode_param->src_size == 720)
            {
                decode_param->src_width = 1280;
                decode_param->src_height = 720;
            }
            else if(decode_param->src_size == 480)
            {
                decode_param->src_width = 640;
                decode_param->src_height = 480;
            }
            else
            {
                decode_param->src_width = 1280;
                decode_param->src_height = 720;
                logw("decoder demo only support the size 1440p,1080p,720p,480p, \
                 now use the default size 720p\n");
            }
            break;
        case INVALID:
        default:
            logd("unknowed argument :  %s", argument);
            break;
    }
}

size_t get_file_size( const char *pFileName )
{
    struct stat sb;
    if( stat( pFileName, &sb ) != 0 )
    {
        printf( "stat failed...\n" );
        return -1;
    }
    return( sb.st_size );
}

int main(int argc, char** argv)
{
    VConfig             videoConf;
    VideoStreamInfo     streamInfo;
    VideoStreamDataInfo streamDataInfo;
    VideoDecoder        *pVideoDec = NULL;
    decode_param_t    decode_param;
    VideoPicture    *pOutPic    = NULL;
    VideoPicture    *pTmpOutPic = NULL;

    int file_size       = 0;
    char *buf0          = NULL;
    char *buf1          = NULL;
    int size0           = 0;
    int size1           = 0;
    unsigned int    wait_frame_count = 0;

    int i = 0;
    int ret = 0;

    FILE *in_file = NULL;
    FILE *out_file = NULL;

    char *input_path = NULL;
    char *output_path = NULL;

    //set the default decode param
    memset(&decode_param, 0, sizeof(decode_param));

    decode_param.src_width = 1280;
    decode_param.src_height = 720;

    decode_param.frame_rate = 30;

    strcpy((char*)decode_param.intput_file,        "/data/camera/720p-30zhen.yuv");
    strcpy((char*)decode_param.output_file,        "/data/camera/720p.264");

    //parse the config paramter
    if(argc >= 2)
    {
        for(i = 1; i < (int)argc; i += 2)
        {
            ParseArgument(&decode_param, argv[i], argv[i + 1]);
        }
    }
    else
    {
        printf(" we need more arguments ");
        PrintDemoUsage();
        return 0;
    }

    if( decode_param.decode_format == 1 )
    {
        decode_param.decode_format = VIDEO_CODEC_FORMAT_MJPEG;
    }
    else
    {
        decode_param.decode_format = VIDEO_CODEC_FORMAT_H264;
    }


    input_path = decode_param.intput_file;
    output_path = decode_param.output_file;

    file_size = get_file_size( input_path );
    printf( "input file size : %d\n", file_size );

    in_file = fopen(input_path, "r");
    if(in_file == NULL)
    {
        loge("open in_file fail\n");
        return -1;
    }

    out_file = fopen(output_path, "wb");
    if(out_file == NULL)
    {
        loge("open out_file fail\n");
        fclose(in_file);
        return -1;
    }

    memset(&videoConf,      0 ,sizeof(VConfig));
    memset(&streamInfo,     0 ,sizeof(VideoStreamInfo));
    memset(&streamDataInfo, 0 ,sizeof(VideoStreamDataInfo));

    pVideoDec = CreateVideoDecoder( );
    if( pVideoDec == NULL )
    {
        printf( "CreateVideoDecoder() failed...\n" );
        goto exit_1;
    }

    streamInfo.eCodecFormat             = decode_param.decode_format;
    streamInfo.nWidth                   = decode_param.src_width;
    streamInfo.nHeight                  = decode_param.src_height;
    streamInfo.nFrameRate               = decode_param.frame_rate * 1000;
    streamInfo.nFrameDuration           = (1000*1000*1000)/streamInfo.nFrameRate;
    streamInfo.bIs3DStream              = 0;
    streamInfo.nCodecSpecificDataLen    = 0;
    streamInfo.pCodecSpecificData       = NULL;
    streamInfo.nAspectRatio             = 0;

    videoConf.memops = MemAdapterGetOpsS();
    if (videoConf.memops == NULL)
    {
        printf("MemAdapterGetOpsS failed\n");
        fclose(in_file);
        fclose(out_file);
        return -1;
    }
    CdcMemOpen(videoConf.memops);

    videoConf.eOutputPixelFormat        = PIXEL_FORMAT_YUV_PLANER_420;
    videoConf.bDisable3D                = 1;
    videoConf.bScaleDownEn              = 0;
    videoConf.nFrameBufferNum           = 0;
    videoConf.bThumbnailMode            = 0;
    //videoConf.nVbvBufferSize            = ALIGN_1K(streamInfo.nWidth * streamInfo.nHeight * 2);


    if( InitializeVideoDecoder( pVideoDec, &streamInfo, &videoConf ) )
    {
        printf( "InitializeVideoDecoder() failed...\n" );
        goto exit_2;
    }

    if( (VideoStreamBufferSize( pVideoDec, 0 ) - VideoStreamDataSize( pVideoDec, 0 ) ) < file_size )
    {
        printf( "Not enough buffer...\n" );
        goto exit_2;
    }

    if( RequestVideoStreamBuffer( pVideoDec, file_size, &buf0, &size0, &buf1, &size1, 0 ) != 0 )
    {
        printf( "RequestVideoStreamBuffer() failed...\n" );
        goto exit_2;
    }

    if( file_size <= size0 )
    {
        if( fread( buf0, 1, file_size, in_file ) != file_size )
        {
            printf( "file read failed...\n" );
            goto exit_3;
        }
    }
    else if( file_size <= (size0 + size1) )
    {
        if( fread( buf0, 1, size0, in_file ) != size0 )
        {
            printf( "file read failed...\n" );
            goto exit_3;
        }
        if( fread( buf1, 1, file_size-size0, in_file ) != (file_size-size0) )
        {
            printf( "file read failed...\n" );
            goto exit_3;
        }
    }
    else
    {
        printf( "decoder buffer is too small...\n" );
        goto exit_3;
    }

    streamDataInfo.nLength         = file_size;
    streamDataInfo.nPts            = -1;
    streamDataInfo.nPcr            = -1;
    streamDataInfo.bIsFirstPart    = 1;
    streamDataInfo.bIsLastPart     = 1;
    streamDataInfo.pData           = buf0;
    streamDataInfo.nStreamIndex    = 0;

    if( SubmitVideoStreamData( pVideoDec, &streamDataInfo, streamDataInfo.nStreamIndex ) != 0 )
    {
        printf( "SubmitVideoStreamData() failed...\n" );
        goto exit_3;
    }
    time1 = GetNowUs();
wait_frame:
    ret = DecodeVideoStream( pVideoDec, 1, 0, 0, 0 );
    if( ret != VDECODE_RESULT_OK 
            && ret != VDECODE_RESULT_CONTINUE 
            && ret != VDECODE_RESULT_FRAME_DECODED 
            && ret != VDECODE_RESULT_KEYFRAME_DECODED )
    {
        if( ret == VDECODE_RESULT_NO_FRAME_BUFFER )
        {
            if( wait_frame_count ++ > 5 )
            {
                goto exit_3;
            }
            usleep( 1000 );
            goto wait_frame;
        }
        goto exit_3;
    }
    time2 = GetNowUs();
    logv("decode frame use time is %lldus..\n",time2-time1);

    pOutPic = RequestPicture( pVideoDec, 0 );
    if( pOutPic == NULL )
    {
        printf( "No picture...\n" );
        goto exit_3;
    }
    else
    {
        VideoPicture *pResultPic = pOutPic;
        if( pOutPic->ePixelFormat == PIXEL_FORMAT_YUV_MB32_420 )
        {
            pTmpOutPic = AllocatePictureBuffer( videoConf.memops,
                                                pOutPic->nWidth,
                                                pOutPic->nHeight,
                                                pOutPic->nLineStride,
                                                PIXEL_FORMAT_YUV_MB32_420 );
            if( pTmpOutPic == NULL )
            {
                printf( "AllocatePictureBuffer() failed...\n" );
                goto exit_3;
            }

            if( DataFormatSoftwareConvert( pOutPic,
                                           (void *)pTmpOutPic->pData0,
                                           pOutPic->nWidth,
                                           pOutPic->nHeight ) == 0 )
            {
                pTmpOutPic->ePixelFormat    = PIXEL_FORMAT_YV12;
                pTmpOutPic->nLeftOffset     = pOutPic->nLeftOffset;
                pTmpOutPic->nRightOffset    = pOutPic->nRightOffset;
                pTmpOutPic->nBottomOffset   = pOutPic->nBottomOffset;
                pTmpOutPic->nTopOffset      = pOutPic->nTopOffset;
                pResultPic = pTmpOutPic;
            }
        }

        int align_w = ALIGN_16B(pResultPic->nWidth);
        int align_h = ALIGN_16B(pResultPic->nHeight);
        int width   = pResultPic->nRightOffset - pResultPic->nLeftOffset;
        int height  = pResultPic->nBottomOffset - pResultPic->nTopOffset;
        int sizeY   = align_w * align_h;
        int ylen    = sizeY;
        int ulen    = sizeY >> 2;
        int vlen    = sizeY >> 2;
        int outY   = width * height;
        int ylen_out    = outY;
        int ulen_out    = outY >> 2;
        int vlen_out    = outY >> 2;

        printf( "Picture : size(%dx%d), offset(%d, %d, %d, %d), ePixelFormat(%d)\n", 
                pResultPic->nWidth,
                pResultPic->nHeight,
                pResultPic->nLeftOffset,
                pResultPic->nRightOffset,
                pResultPic->nBottomOffset,
                pResultPic->nTopOffset,
                pResultPic->ePixelFormat );

        if( pResultPic->ePixelFormat == PIXEL_FORMAT_YUV_PLANER_420 )
        {
            fwrite( pResultPic->pData0,            1, ylen_out,    out_file );
            fwrite( pResultPic->pData0+ylen,       1, ulen_out,    out_file );
            fwrite( pResultPic->pData0+ylen+ulen,  1, vlen_out,    out_file );
        }
        else
        {
            fwrite( pResultPic->pData0,            1, ylen_out,    out_file );
            fwrite( pResultPic->pData0+ylen+ulen,  1, vlen_out,    out_file );
            fwrite( pResultPic->pData0+ylen,       1, ulen_out,    out_file );
        }
    }
          

out:
    printf("output file is saved:%s\n",decode_param.output_file);
exit_3:
    if( pTmpOutPic )
    {
        FreePictureBuffer( videoConf.memops, pTmpOutPic );
    }
    if( pOutPic )
    {
        ReturnPicture( pVideoDec, pOutPic );
    }
exit_2:
    if(pVideoDec)
    {
        DestroyVideoDecoder(pVideoDec);
    }
    pVideoDec = NULL;

exit_1:
    fclose(out_file);
    fclose(in_file);

    if(videoConf.memops)
    {
        CdcMemClose(videoConf.memops);
    }

    return 0;
}
