#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"

int main(int argc , char*argv[])
{
    EncodeInfo encInfo;
    DecodeInfo decInfo;
     if(argc<2)
     {
        printf("Error:INvalid input \n for encoding: ./a.out -e beautiful.bmp secret.txt [stego.bmp]\n for decoding: ./a.out -d beautiful.bmp secret.txt [stego.bmp] \n");
        return 1;
     }
     if(check_operation_type(&argv[1][1])==e_encode)
     {
         if(argc<4 || argc>5)
        {
            printf("Invalid input\n");
            printf("For encoding: ./a.out -e beautiful.bmp secret.txt [stego.bmp]\n");
            return 1;
        }
       if(read_and_validate_encode_args(argv, &encInfo)==e_success)
        {
           if( do_encoding(&encInfo)==e_success)
           {
            printf("Encoding is done\n");
            return 0;
           }
           else
           {
            printf("Encoding is failed \n");
            return 1;
           }

        }
       
     }
     else if(check_operation_type(&argv[1][1])==e_decode)
     {
        if(argc<3 || argc>4)
        {
            printf("Invalid input\n");
            printf("For decoding: ./a.out -d stego.bmp [decoded.txt]\n");
            return 1;
        }
        if(read_and_validate_decode_args(argv, &decInfo)==e_success)
        {
           if(do_decoding(&decInfo)==e_success)
           {
           printf("Decoding is done\n");
            return 0;
           }
           else
           {
            printf("decoding failed\n");
            return 1;
           }

        }
     }
     else if(check_operation_type(&argv[1][1])==e_unsupported)
     {
        printf("INvalid input \n for encoding: ./a.out -e beautiful.bmp secret.txt [stego.bmp]\n for decoding: ./a.out -d beautiful.bmp secret.txt [stego.bmp] \n");
        return 1;
     }
     

    return 0;
}
OperationType check_operation_type(char *opt)
{
    if(strcmp(opt,"e")==0)
    {
        return e_encode;
    }
    else if(strcmp(opt,"d")==0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}
