

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <zlib.h>

#include "libmng.h"

typedef struct user_struct {
  FILE *hFile;                 /* file handle */
  int  iIndent;                /* for nice indented formatting */
  FILE* fpzdata;             // this current file pointer to the png file
  int frame;              // the current frame
  char* fileprefix ; 
} userdata;
typedef userdata * userdatap;


/* ************************************************************************** */

mng_ptr myalloc (mng_size_t iSize)
{
  return (mng_ptr)calloc (1, iSize);   /* duh! */
}

/* ************************************************************************** */

void myfree (mng_ptr pPtr, mng_size_t iSize)
{
  free (pPtr);                         /* duh! */
  return;
}

/* ************************************************************************** */

mng_bool myopenstream (mng_handle hMNG)
{
  return MNG_TRUE;                     /* already opened in main function */
}

/* ************************************************************************** */

mng_bool myclosestream (mng_handle hMNG)
{
  return MNG_TRUE;                     /* gets closed in main function */
}

/* ************************************************************************** */

mng_bool myreaddata (mng_handle hMNG,
    mng_ptr    pBuf,
    mng_uint32 iSize,
    mng_uint32 *iRead)
{                                      /* get to my file handle */
  userdatap pMydata = (userdatap)mng_get_userdata (hMNG);
  /* read it */
  *iRead = fread (pBuf, 1, iSize, pMydata->hFile);
  return MNG_TRUE;
}


/* ************************************************************************** */

mng_bool myiterchunk (mng_handle  hMNG,
    mng_handle  hChunk,
    mng_chunkid iChunktype,
    mng_uint32  iChunkseq)
{                                      /* get to my file handle */
  userdatap pMydata = (userdatap)mng_get_userdata (hMNG);
  char aCh[4];
  char zIndent[80];
  int iX;
  /* decode the chunkname */
  aCh[0] = (char)((iChunktype >> 24) & 0xFF);
  aCh[1] = (char)((iChunktype >> 16) & 0xFF);
  aCh[2] = (char)((iChunktype >>  8) & 0xFF);
  aCh[3] = (char)((iChunktype      ) & 0xFF);
  /* indent less ? */
  if ( (iChunktype == MNG_UINT_MEND) || (iChunktype == MNG_UINT_IEND) ||
      (iChunktype == MNG_UINT_ENDL) )
    pMydata->iIndent -= 2;
  /* this looks ugly; but I haven't
     figured out how to do it prettier */
  for (iX = 0; iX < pMydata->iIndent; iX++)
    zIndent[iX] = ' ';
  zIndent[pMydata->iIndent] = '\0';
  /* print a nicely indented line */
  //printf ("%s%c%c%c%c\n", zIndent, aCh[0], aCh[1], aCh[2], aCh[3]);
  /* indent more ? */
  if ( (iChunktype == MNG_UINT_MHDR) || (iChunktype == MNG_UINT_IHDR) ||
      (iChunktype == MNG_UINT_JHDR) || (iChunktype == MNG_UINT_DHDR) ||
      (iChunktype == MNG_UINT_BASI) || (iChunktype == MNG_UINT_LOOP)    )
    pMydata->iIndent += 2;

  if(iChunktype == MNG_UINT_IHDR)
  {
    unsigned int width         =0; 
    unsigned int height        =0;
    unsigned int bitDepth      =0;
    unsigned int colorType     =0;
    unsigned int compression   =0;
    unsigned int filter        =0;
    unsigned int interlace     =0;
    mng_getchunk_ihdr(hMNG, hChunk,  &width, &height, &bitDepth, &colorType, &compression, &filter, &interlace);
    // printf("width        = %d\n", width       );
    // printf("height       = %d\n", height      );
    // printf("bitDepth     = %d\n", bitDepth    );
    // printf("colorType    = %d\n", colorType   );
    // printf("compression  = %d\n", compression );
    // printf("filter       = %d\n", filter      );
    // printf("interlace    = %d\n", interlace   );
    pMydata->frame ++ ;
    //write the 
    char fname [80];
    sprintf(fname, "%s%04d.png", pMydata->fileprefix, pMydata->frame);
    pMydata->fpzdata =  fopen(fname, "wb");
    if(pMydata->fpzdata)
    {
      char signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
      fwrite(signature, 1, 8, pMydata->fpzdata);
      int n = 13;
      unsigned char ihdr [4+4+13+4];
      *(unsigned char*)&ihdr[  0] = (n >>24)&0xff;
      *(unsigned char*)&ihdr[  1] = (n >>16)&0xff;
      *(unsigned char*)&ihdr[  2] = (n >> 8)&0xff;
      *(unsigned char*)&ihdr[  3] = (n >> 0)&0xff;
      memcpy(ihdr+4, "IHDR", 4);
      *(unsigned char*)&ihdr[4+4+ 0] = (width >>24)&0xff;
      *(unsigned char*)&ihdr[4+4+ 1] = (width >>16)&0xff;
      *(unsigned char*)&ihdr[4+4+ 2] = (width >> 8)&0xff;
      *(unsigned char*)&ihdr[4+4+ 3] = (width >> 0)&0xff;
      *(unsigned char*)&ihdr[4+4+ 4] = (height>>24)&0xff;
      *(unsigned char*)&ihdr[4+4+ 5] = (height>>16)&0xff;
      *(unsigned char*)&ihdr[4+4+ 6] = (height>> 8)&0xff;
      *(unsigned char*)&ihdr[4+4+ 7] = (height>> 0)&0xff;
      *(unsigned char*)&ihdr[4+4+ 8] = bitDepth;
      *(unsigned char*)&ihdr[4+4+ 9] = colorType;
      *(unsigned char*)&ihdr[4+4+10] = compression;
      *(unsigned char*)&ihdr[4+4+11] = filter;
      *(unsigned char*)&ihdr[4+4+12] = interlace;
      uLong crc = crc32(0L, Z_NULL, 0);
      crc = crc32(crc,ihdr+4 , 13+4);
      *(unsigned char*)&ihdr[4+4+13] = (crc >>24)&0xff;
      *(unsigned char*)&ihdr[4+4+14] = (crc >>16)&0xff;
      *(unsigned char*)&ihdr[4+4+15] = (crc >> 8)&0xff;
      *(unsigned char*)&ihdr[4+4+16] = (crc >> 0)&0xff;
      fwrite(ihdr,1, sizeof ihdr, pMydata->fpzdata);
    }
  }
  else if(iChunktype == MNG_UINT_IDAT)
  {
    int len = 0;
    char* ptr =0;
    mng_getchunk_idat(hMNG, hChunk,  &len, &ptr);
    //printf("len=%d ptr = %p\n", len, ptr);

    if(pMydata->fpzdata)
    {
      char data[4];
      *(unsigned char*)&data[0] = (len >>24)&0xff;
      *(unsigned char*)&data[1] = (len >>16)&0xff;
      *(unsigned char*)&data[2] = (len >> 8)&0xff;
      *(unsigned char*)&data[3] = (len >> 0)&0xff;
      fwrite(data, 1, 4, pMydata->fpzdata);
      fwrite("IDAT",  1, 4, pMydata->fpzdata);
      fwrite(ptr, 1, len, pMydata->fpzdata);
      uLong crc = crc32(0L, Z_NULL, 0);
      crc = crc32(crc, "IDAT", 4);
      crc = crc32(crc,ptr, len);
      *(unsigned char*)&data[0] = (crc >>24)&0xff;
      *(unsigned char*)&data[1] = (crc >>16)&0xff;
      *(unsigned char*)&data[2] = (crc >> 8)&0xff;
      *(unsigned char*)&data[3] = (crc >> 0)&0xff;
      fwrite(data, 1, 4, pMydata->fpzdata);
    }

    //save_zdata_file(fname, ptr, len);
  }
  else if(iChunktype == MNG_UINT_IEND)
  {
    if(pMydata->fpzdata)
    {
      int len = 0;
      char data[4];
      *(unsigned char*)&data[0] = (len >>24)&0xff;
      *(unsigned char*)&data[1] = (len >>16)&0xff;
      *(unsigned char*)&data[2] = (len >> 8)&0xff;
      *(unsigned char*)&data[3] = (len >> 0)&0xff;
      fwrite(data, 1, 4, pMydata->fpzdata);
      fwrite("IEND",  1, 4, pMydata->fpzdata);
      uLong crc = crc32(0L, Z_NULL, 0);
      crc = crc32(crc, "IEND", 4);
      *(unsigned char*)&data[0] = (crc >>24)&0xff;
      *(unsigned char*)&data[1] = (crc >>16)&0xff;
      *(unsigned char*)&data[2] = (crc >> 8)&0xff;
      *(unsigned char*)&data[3] = (crc >> 0)&0xff;
      fwrite(data, 1, 4, pMydata->fpzdata);
      fclose(pMydata->fpzdata);
    }
  }


  return MNG_TRUE;                     /* keep'm coming... */
}

/* ************************************************************************** */

int dumptree (char * zFilename, char* fileprefix)
{
  userdatap pMydata;
  mng_handle hMNG;
  mng_retcode iRC;
  /* get a data buffer */
  pMydata = (userdatap)calloc (1, sizeof (userdata));
  pMydata->frame = 0;
  pMydata->fileprefix = fileprefix;
  
  printf(".");

  if (pMydata == NULL)                 /* oke ? */
  {
    fprintf (stderr, "Cannot allocate a data buffer.\n");
    return 1;
  }
  /* can we open the file ? */
  if ((pMydata->hFile = fopen (zFilename, "rb")) == NULL)
  {                                    /* error out if we can't */
    fprintf (stderr, "Cannot open input file %s.\n", zFilename);
    return 1;
  }
  /* let's initialize the library */
  hMNG = mng_initialize((mng_ptr)pMydata, myalloc, myfree, MNG_NULL);
  if (!hMNG)                           /* did that work out ? */
  {
    fprintf (stderr, "Cannot initialize libmng.\n");
    iRC = 1;
  }
  else
  {                                    /* setup callbacks */
    if ( ((iRC = mng_setcb_openstream  (hMNG, myopenstream )) != 0) ||
        ((iRC = mng_setcb_closestream (hMNG, myclosestream)) != 0) ||
        ((iRC = mng_setcb_readdata    (hMNG, myreaddata   )) != 0)    )
      fprintf (stderr, "Cannot set callbacks for libmng.\n");

      {                                  /* read the file into memory */
        if ((iRC = mng_read (hMNG)) != 0)
          fprintf (stderr, "Cannot read the file.\n");
        else
        {
          pMydata->iIndent = 2;          /* start the indenting at a nice level */
          printf ("Starting dump of %s.\n\n", zFilename);
          /* run through the chunk list */
          if ((iRC = mng_iterate_chunks (hMNG, 0, myiterchunk)) != 0)
            fprintf (stderr, "Cannot iterate the chunks.\n");
          printf ("\nDone.\n");
        }
      }

      mng_cleanup (&hMNG);               /* cleanup the library */
  }

  fclose (pMydata->hFile);             /* cleanup */
  free (pMydata);

  return iRC;
}

int main(int argc, char *argv[])
{
  char* fileprefix = "pngs/_";
  if(argc >2)
  {
    fileprefix = argv[2];
  }
  if (argc > 1)                  /* need that first parameter ! */
    return dumptree (argv[1], fileprefix);
  else
    printf ("\nUsage: mngtree <file.mng>\n\n");  

  return 0;
}

