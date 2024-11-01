#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "ar_addon.h"
#include "ar_addon_transformation.h"
#include <iconv.h>

static void trans_hex_udt(sqlite3_context *context, int argc, sqlite3_value **argv);

AR_AO_EXPORTED int ar_addon_init(AR_ADDON_CONTEXT *context)
{
        AR_AO_TRANSFORMATION_DEF *transdef = NULL;

        AR_AO_INIT(context);

        transdef = GET_AR_AO_TRANSFORMATION_DEF();
        transdef->displayName = "hex_udt(X)";
        transdef->functionName = "hex_udt";
        transdef->description = "hex_udt decodes cp273 string X";
        transdef->func = trans_hex_udt;
        transdef->nArgs = 1;
        AR_AO_REGISRATION->register_user_defined_transformation(transdef);

        return 0;
}

// Complete lookup table for EBCDIC cp273 to ASCII conversion
unsigned char ebcdic_to_ascii[256] = {
        0x00,	0x01,	0x02,	0x03,	0x9C,	0x09,	0x86,	0x7F,	0x97,	0x8D,	0x8E,	0x0B,	0x0C,	0x0D,	0x0E,	0x0F,
        0x10,	0x11,	0x12,	0x13,	0x9D,	0x85,	0x08,	0x87,	0x18,	0x19,	0x92,	0x8F,	0x1C,	0x1D,	0x1E,	0x1F,
        0x80,	0x81,	0x82,	0x83,	0x84,	0x0A,	0x17,	0x1B,	0x88,	0x89,	0x8A,	0x8B,	0x8C,	0x05,	0x06,	0x07,
        0x90,	0x91,	0x16,	0x93,	0x94,	0x95,	0x96,	0x04,	0x98,	0x99,	0x9A,	0x9B,	0x14,	0x15,	0x9E,	0x1A,
        0x20,	0xA0,	0xE2,	0x7B,	0xE0,	0xE1,	0xE3,	0xE5,	0xE7,	0xF1,	0xC4,	0x2E,	0x3C,	0x28,	0x2B,	0x21,
        0x26,	0xE9,	0xEA,	0xEB,	0xE8,	0xED,	0xEE,	0xEF,	0xEC,	0x7E,	0xDC,	0x24,	0x2A,	0x29,	0x3B,	0x5E,
        0x2D,	0x2F,	0xC2,	0x5B,	0xC0,	0xC1,	0xC3,	0xC5,	0xC7,	0xD1,	0xF6,	0x2C,	0x25,	0x5F,	0x3E,	0x3F,
        0xF8,	0xC9,	0xCA,	0xCB,	0xC8,	0xCD,	0xCE,	0xCF,	0xCC,	0x60,	0x3A,	0x23,	0xA7,	0x27,	0x3D,	0x22,
        0xD8,	0x61,	0x62,	0x63,	0x64,	0x65,	0x66,	0x67,	0x68,	0x69,	0xAB,	0xBB,	0xF0,	0xFD,	0xFE,	0xB1,
        0xB0,	0x6A,	0x6B,	0x6C,	0x6D,	0x6E,	0x6F,	0x70,	0x71,	0x72,	0xAA,	0xBA,	0xE6,	0xB8,	0xC6,	0xA4,
        0xB5,	0xDF,	0x73,	0x74,	0x75,	0x76,	0x77,	0x78,	0x79,	0x7A,	0xA1,	0xBF,	0xD0,	0xDD,	0xDE,	0xAE,
        0xA2,	0xA3,	0xA5,	0xB7,	0xA9,	0x40,	0xB6,	0xBC,	0xBD,	0xBE,	0xAC,	0x7C,	0xAF,	0xA8,	0xB4,	0xD7,
        0xE4,	0x41,	0x42,	0x43,	0x44,	0x45,	0x46,	0x47,	0x48,	0x49,	0xAD,	0xF4,	0xA6,	0xF2,	0xF3,	0xF5,
        0xFC,	0x4A,	0x4B,	0x4C,	0x4D,	0x4E,	0x4F,	0x50,	0x51,	0x52,	0xB9,	0xFB,	0x7D,	0xF9,	0xFA,	0xFF,
        0xD6,	0xF7,	0x53,	0x54,	0x55,	0x56,	0x57,	0x58,	0x59,	0x5A,	0xB2,	0xD4,	0x5C,	0xD2,	0xD3,	0xD5,
        0x30,	0x31,	0x32,	0x33,	0x34,	0x35,	0x36,	0x37,	0x38,	0x39,	0xB3,	0xDB,	0x5D,	0xD9,	0xDA,	0x9F
};

void hex_to_bytes(const char *hex_string, unsigned char *byte_array, size_t *byte_array_len) {
    size_t len = strlen(hex_string);
    *byte_array_len = len / 2;
    for (size_t i = 0; i < *byte_array_len; i++) {
        sscanf(hex_string + 2 * i, "%2hhx", &byte_array[i]);
    }
}

void decode_ebcdic_to_ascii(const unsigned char *ebcdic_data, size_t length, char *ascii_string) {
    for (size_t i = 0; i < length; i++) {
        unsigned char ascii_char = ebcdic_to_ascii[ebcdic_data[i]];
        // Replace non-printable characters with a placeholder (e.g., a space)
        ascii_string[i] = isprint(ascii_char) ? ascii_char : ' ';
    }
    ascii_string[length] = '\0'; // Null-terminate the ASCII string
}

//Error handling
//Use sqlite3_result_error_code function to return an error.
//Three codes could have been used:
// SQLLITE_ERROR (1) - causes Replicate to skip the problematic data record
// SQLITE_CALLBACK_FATAL_ERROR (251) - causes Replicate to stop the task immediately with a fatal error
// SQLITE_CALLBACK_RECOVERABLE_ERROR (250) - causes Replicate to reattach the target endpoint or to stop the task with a recoverable error 
// Any other error code will be proceeded as SQLLITE_ERROR. 
// sqlite3_result_error function could have been used to return an error message. If this function is used without sqlite3_result_error_code, SQLLITE_ERROR is returned.   
static void trans_hex_udt(sqlite3_context *context, int argc, sqlite3_value **argv)
{
        AR_AO_LOG->log_trace("enter trans_hex_udt");
        if (argc >= 1) 
        { // you should check that all the parameters declared in the function definition are provided
                char *hexText = (char *)AR_AO_SQLITE->sqlite3_value_text(argv[0]); 
                char pRes[2000] = {0}; // Result string

                unsigned char byte_array[1024];
                size_t byte_array_len;

                // Convert hex string to byte array
                hex_to_bytes(hexText, byte_array, &byte_array_len);

                // Decode EBCDIC (cp037) to ASCII
                decode_ebcdic_to_ascii(byte_array, byte_array_len, pRes);

                AR_AO_SQLITE->sqlite3_result_text(context, pRes, -1, SQLITE_TRANSIENT);
                AR_AO_LOG->log_trace("Before %s", "return");
        }
        else
        { // should not occur but only if you do not declare the function correct
        AR_AO_LOG->log_error("The sqlite addon function trans_hex_udt received an incorrect (%d instead of at least 1) number of parameters", argc);
                AR_AO_SQLITE->sqlite3_result_error(context, "incorrect parameter list", -1); 
                AR_AO_SQLITE->sqlite3_result_error_code(context, SQLITE_CALLBACK_FATAL_ERROR); 
        }

        AR_AO_LOG->log_trace("leave trans_hex_udt");
}
