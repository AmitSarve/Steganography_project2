#include <stdio.h>
#include "encode.h"
#include "types.h"
#include<string.h>
#include"common.h"
/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char *extension;


    extension = strrchr(argv[2], '.');

    if (extension == NULL || strcmp(extension, ".bmp") != 0)
    {
        printf("ERROR: Source image should be a .bmp file\n");
        return e_failure;
    }

    encInfo->src_image_fname = argv[2];
    encInfo->secret_fname=argv[3];

    if(argv[4]==NULL)
    {
        encInfo->stego_image_fname="output.bmp";

    }
    else
    {
        // validate o/p file will contain .bmp or not
        char *extn;
        extn=strrchr(argv[4],'.');

        if(extn==NULL || strcmp(extn,".bmp")!=0)
        {
            printf("Error : output image shoud be a .bmp file\n");
            return e_failure;
        }
        else
        {
            encInfo->stego_image_fname=argv[4];
        }
    }
    if(open_files(encInfo)== e_failure)
    {
        return e_failure;
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    if(check_capacity(encInfo)==e_failure)
    {
        printf("Error file capacity is not enough\n");
        return e_failure;
    }

    if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image)==e_failure)
    {
        printf("Error ");
        return e_failure;
    }

    if(encode_magic_string(MAGIC_STRING,encInfo)== e_failure)
    {
        printf("Error in magic string\n");
        return e_failure;
    }
    /*----->*/
    if( encode_secret_file_extn_size( encInfo)== e_failure)
    {
        printf("Error in file extension\n");
        return e_failure;
    }

    if(encode_secret_file_extn( encInfo->extn_secret_file,encInfo)== e_failure)
    {
        printf("Error in file extension\n");
        return e_failure;
    }

    long file_size=get_file_size(encInfo->fptr_secret);
    fseek(encInfo->fptr_secret, 0, SEEK_SET);
    if(encode_secret_file_size(  file_size,encInfo)== e_failure)
    {
        printf("Error in encode secret file size\n");
        return e_failure;
    }

    if(encode_secret_file_data(encInfo)==e_failure)
        {
            printf("Error in encode secret file data\n");
            return e_failure;
        }
        

    if( copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image)==e_failure)
        {
            printf("Error in copy remaninig img data\n");
            return e_failure;
        }
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    get_image_size_for_bmp(encInfo->fptr_src_image);
    unsigned int image_capacity=get_file_size(encInfo->fptr_src_image);
    unsigned int size_secret_file = get_file_size(encInfo->fptr_secret);

    if((( 14 + size_secret_file) * 8) > image_capacity)
    {
        return e_failure;
    }
    return e_success;

}
uint get_file_size(FILE *fptr)
{
    
    fseek(fptr,0,SEEK_END);
    return ftell(fptr);

}
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    fseek(fptr_src_image,0,SEEK_SET);
    char buffer[54];

    fread(buffer,6,9,fptr_src_image);
    fwrite(buffer,6,9,fptr_dest_image);
    return e_success;


}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char buff[8];
    for(int i=0;magic_string[i]!=0;i++)
    {
        fread(buff,1,8,encInfo->fptr_src_image);

        encode_byte_to_lsb(magic_string[i], buff);

        fwrite(buff,1,8,encInfo->fptr_stego_image);
    }
    return e_success;
}
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i=7;i>=0;i--)
    {
        int get=(data>>i)&1;
    
            image_buffer[7-i]=(image_buffer[7-i]) & (~1) | (get) ;


    }
    return e_success;
}
Status encode_secret_file_extn_size( EncodeInfo *encInfo)
{
    char*dot=strrchr(encInfo->secret_fname,'.');

    if(dot==NULL)
    {
        return e_failure;
    }
    strcpy(encInfo->extn_secret_file,dot);

    char buff[32];

    fread(buff,1,32,encInfo->fptr_src_image);

    if(encode_size_to_lsb(strlen(encInfo->extn_secret_file),buff)==e_success)
    {
        fwrite(buff,1,32,encInfo->fptr_stego_image);

    }
    return e_success;
}
Status encode_size_to_lsb(int size,char *Image_buffer)
{
    for(int i=31;i>=0;i--)
    {
            int get=(size>>i)&1;

            Image_buffer[31-i]=(Image_buffer[31-i]) & (~1) | (get) ;
    }
    return e_success;
}
Status encode_secret_file_extn(const char* file_extn ,EncodeInfo*encInfo)   
    {
        char buff[8];
    for(int i=0;file_extn[i]!=0;i++)
    {
        fread(buff,1,8,encInfo->fptr_src_image);
        if( encode_byte_to_lsb(  file_extn[i],buff)==e_failure)
        {
            return e_failure;
        }
        
        fwrite(buff,1,8,encInfo->fptr_stego_image);
            
    }
    return e_success;
}
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
    {
        char buff[32];
        fread(buff,1,32,encInfo->fptr_src_image);
        if(encode_size_to_lsb(file_size,buff)==e_failure)
        {
            return e_failure;
        }
        fwrite(buff,1,32,encInfo->fptr_stego_image);

        return e_success;

    }
    Status encode_secret_file_data(EncodeInfo *encInfo)
    {
        char buff[8];
        char data;
        
        while(fread(&data,1,1,encInfo->fptr_secret)==1)
        {
            fread(buff,1,8,encInfo->fptr_src_image);
            encode_byte_to_lsb(data,buff);
            fwrite(buff, 1, 8, encInfo->fptr_stego_image);
        }
        return e_success;

    }

    Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
    {
        char data;
        while(fread(&data,1,1,fptr_src)==1)
        {
           
            fwrite(&data,1,1,fptr_dest);
        }
        return e_success;
   
    }

//for decoding
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    
    char *extension;
    extension = strrchr(argv[2], '.');
    if (extension == NULL || strcmp(extension, ".bmp") != 0)
    {
        printf("ERROR: stego image should be a .bmp file\n");
        return e_failure;
    }
    decInfo->stego_image_fname = argv[2];

    if(argv[3]==NULL)
    {
        decInfo->output_fname="decoded.txt";
    }
    else
    {
        char *extension1;
        extension1 = strrchr(argv[3], '.');
        if (extension1 == NULL || strcmp(extension1, ".txt") != 0)
        {
            printf("ERROR: Output file should be a .txt file\n");
            return e_failure;
        }
        decInfo->output_fname=argv[3];
    }
    

    return e_success;
}



Status do_decoding(DecodeInfo *decInfo)
{
    if (open_files_decode(decInfo) == e_failure)
    {
        printf("ERROR: File opening failed\n");
        return e_failure;
    }

    printf("INFO: Files opened successfully\n");

    if (decode_magic_string(decInfo) == e_failure)
    {
        printf("ERROR: Magic string mismatch\n");
        return e_failure;
    }

    printf("INFO: Magic string matched\n");

    if (decode_secret_file_extn_size(decInfo) == e_failure)
        return e_failure;

    if (decode_secret_file_extn(decInfo) == e_failure)
        return e_failure;

    if (decode_secret_file_size(decInfo) == e_failure)
        return e_failure;

    if (decode_secret_file_data(decInfo) == e_failure)
        return e_failure;

    printf("INFO: Decoding completed successfully\n");

    return e_success;
}

Status open_files_decode(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image=fopen(decInfo->stego_image_fname,"r");
    if(decInfo->fptr_stego_image==NULL)
    {
        printf("Error!file not opened");
        return e_failure;
    }
    decInfo->fptr_output =fopen(decInfo->output_fname, "w");

    if(decInfo->fptr_output == NULL)
    {
        printf("Error!file not opened");
        return e_failure;
        
    }
    return e_success;
}

Status decode_magic_string(DecodeInfo *decInfo)
{
    char decoded_magic[3];
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    decoded_magic[0] =
        decode_byte_from_lsb(decInfo->fptr_stego_image);

    decoded_magic[1] =
        decode_byte_from_lsb(decInfo->fptr_stego_image);

    decoded_magic[2] = '\0';

    printf("Decoded magic string: %s\n", decoded_magic);

    if (strcmp(decoded_magic, "#*") != 0)
    {
        return e_failure;
    }

    return e_success;
}

char decode_byte_from_lsb(FILE*fptr_stego_image)
{
    char buff[8];
    char ch=0;

    fread(buff,1,8,fptr_stego_image);

    for(int i=0;i<8;i++)
    {
        ch=ch|((buff[i]&1)<<(7-i));
    }
    return ch;
}
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char buff[32];
    int size = 0;

    fread(buff, 1, 32, decInfo->fptr_stego_image);

    for(int i = 0; i < 32; i++)
    {
        size = size | ((buff[i] & 1) << (31 - i));
    }

    decInfo->size_secret_file_extn = size;

    printf("Decoded extension size = %d\n", size);

    return e_success;
}
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    int size = decInfo->size_secret_file_extn;
    if(size<=0 || size>=sizeof(decInfo->secret_file_extn))
    {
        printf("Error Invalid secret file extension \n");
        return e_failure;
    }

    for(int i=0;i<size; i++)
    {
        decInfo->secret_file_extn[i] = decode_byte_from_lsb(decInfo->fptr_stego_image);
    }

    decInfo->secret_file_extn[size] = '\0';

    printf("Decoded extension = %s\n",
           decInfo->secret_file_extn);

    return e_success;
}
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buff[32];
    long size = 0;

    fread(buff, 1, 32, decInfo->fptr_stego_image);

    for(int i = 0; i < 32; i++)
    {
        size = size | ((buff[i] & 1) << (31 - i));
    }

    decInfo->size_secret_file = size;

    if(size<0)
    {
        printf("Error invalid secret file size\n");
        return e_failure;
    }

    printf("Decoded secret file size = %ld\n", size);

    return e_success;
}
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char data;

    for(long i = 0; i < decInfo->size_secret_file; i++)
    {
        data = decode_byte_from_lsb(decInfo->fptr_stego_image);

        fwrite(&data, 1, 1, decInfo->fptr_output);
    }

    return e_success;
}
