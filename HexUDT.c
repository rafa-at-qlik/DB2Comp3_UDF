#include <stdio.h>
#include <string.h>
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


typedef struct {
    char code[3];  // Stores the two-character code as a string (like "00", "01", etc.)
    char *description;  // Points to the corresponding description string (like "Null", "Ü", etc.)
} CodeMap;

// CP273 Mapping
// Based on https://www.longpelaexpertise.com.au/toolsCode.php CP273
CodeMap mappings[] = {
        {"00", "00"},
        {"01", "01"},
        {"02", "02"},
        {"03", "03"},
        {"04", "04"},
        {"05", "05"},
        {"06", "06"},
        {"07", "07"},
        {"08", "08"},
        {"09", "9"},
        {"0A", ""},
        {"0B", ""},
        {"0C", ";"},
        {"0D", ""},
        {"0E", ""},
        {"0F", ";"},
        {"10", "10"},
        {"11", "11"},
        {"12", "12"},
        {"13", "13"},
        {"14", "14"},
        {"15", "15"},
        {"16", "16"},
        {"17", "17"},
        {"18", "18"},
        {"19", "19"},
        {"1A", ""},
        {"1A", ""},
        {"1B", ""},
        {"1C", ""},
        {"1D", ""},
        {"1E", ""},
        {"1F", ""},
        {"20", ""},
        {"21", ""},
        {"22", ""},
        {"23", ""},
        {"24", ""},
        {"25", ""},
        {"26", ""},
        {"27", "\n"},
        {"28", ""},
        {"29", ""},
        {"2A", ""},
        {"2B", ""},
        {"2C", ""},
        {"2D", ""},
        {"2E", ""},
        {"2F", ""},
        {"30", "30"},
        {"31", "31"},
        {"32", "32"},
        {"33", "33"},
        {"34", "34"},
        {"35", "35"},
        {"36", "36"},
        {"37", "37"},
        {"38", "38"},
        {"39", "39"},
        {"3A", ""},
        {"3B", ""},
        {"3C", ""},
        {"3D", ""},
        {"3E", ""},
        {"3F", ""},
        {"40", ""},
        {"41", "41"},
        {"42", "â"},
        {"43", "{"},
        {"44", "à"},
        {"45", "á"},
        {"46", "ã"},
        {"47", "å"},
        {"48", "ç"},
        {"49", "ñ"},
        {"4A", "Ä"},
        {"4B", "."},
        {"4C", "<"},
        {"4D", "("},
        {"4E", "+"},
        {"4F", "!"},
        {"50", "&"},
        {"51", "é"},
        {"52", "ê"},
        {"53", "ë"},
        {"54", "è"},
        {"55", "í"},
        {"56", "î"},
        {"57", "ï"},
        {"58", "ì"},
        {"59", "~"},
        {"5A", "Ü"},
        {"5B", "$"},
        {"5C", "*"},
        {"5D", ")"},
        {"5E", ";"},
        {"5F", "^"},
        {"60", "-"},
        {"61", "/"},
        {"62", "Â"},
        {"63", "["},
        {"64", "À"},
        {"65", "Á"},
        {"66", "Ã"},
        {"67", "$"},
        {"68", "Ç"},
        {"69", "Ñ"},
        {"6A", "ö"},
        {"6B", ","},
        {"6C", "%"},
        {"6D", "_"},
        {"6E", ">"},
        {"6F", "?"},
        {"70", "ø"},
        {"71", "É"},
        {"72", "Ê"},
        {"73", "Ë"},
        {"74", "È"},
        {"75", "Í"},
        {"76", "Î"},
        {"77", "Ï"},
        {"78", "Ì"},
        {"79", "`"},
        {"7A", ":"},
        {"7B", "#"},
        {"7C", "§"},
        {"7D", "'"},
        {"7E", "="},
        {"7F", "\""},
        {"80", "Ø"},
        {"81", "a"},
        {"82", "b"},
        {"83", "c"},
        {"84", "d"},
        {"85", "e"},
        {"86", "f"},
        {"87", "g"},
        {"88", "h"},
        {"89", "i"},
        {"8A", "«"},
        {"8B", "»"},
        {"8C", "ð"},
        {"8D", "ý"},
        {"8E", "Þ"},
        {"8F", "±"},
        {"90", "º"},
        {"91", "j"},
        {"92", "k"},
        {"93", "l"},
        {"94", "m"},
        {"95", "n"},
        {"96", "o"},
        {"97", "p"},
        {"98", "q"},
        {"99", "r"},
        {"9A", "ª"},
        {"9B", "º"},
        {"9C", "æ"},
        {"9D", "¸"},
        {"9E", "Æ"},
        {"9F", "¤"},
        {"A0", "µ"},
        {"A1", "ß"},
        {"A2", "s"},
        {"A3", "t"},
        {"A4", "u"},
        {"A5", "v"},
        {"A6", "w"},
        {"A7", "x"},
        {"A8", "y"},
        {"A9", "z"},
        {"AA", "¡"},
        {"AB", "¿"},
        {"AC", "Ð"},
        {"AD", "Ý"},
        {"AE", "þ"},
        {"AF", "®"},
        {"B0", "¢"},
        {"B1", "£"},
        {"B2", "¥"},
        {"B3", "·"},
        {"B4", "©"},
        {"B5", "@"},
        {"B6", "¶"},
        {"B7", "¼"},
        {"B8", "½"},
        {"B9", "¾"},
        {"BA", "¬"},
        {"BB", "|"},
        {"BC", "¯"},
        {"BD", "¨"},
        {"BE", "´"},
        {"BF", "×"},
        {"C0", "ä"},
        {"C1", "A"},
        {"C2", "B"},
        {"C3", "C"},
        {"C4", "D"},
        {"C5", "E"},
        {"C6", "F"},
        {"C7", "G"},
        {"C8", "H"},
        {"C9", "I"},
        {"CA", ""}, // Syllable Hyphen
        {"CB", "ô"},
        {"CC", "¦"},
        {"CD", "ò"},
        {"CE", "ó"},
        {"CF", "õ"},
        {"D0", "ü"},
        {"D1", "J"},
        {"D2", "K"},
        {"D3", "L"},
        {"D4", "M"},
        {"D5", "N"},
        {"D6", "O"},
        {"D7", "P"},
        {"D8", "Q"},
        {"D9", "R"},
        {"DA", "¹"},
        {"DB", "û"},
        {"DC", "}"},
        {"DD", "ù"},
        {"DE", "ú"},
        {"DF", "ÿ"},
        {"E0", "Ö"},
        {"E1", "÷"},
        {"E2", "S"},
        {"E3", "T"},
        {"E4", "U"},
        {"E5", "V"},
        {"E6", "W"},
        {"E7", "X"},
        {"E8", "Y"},
        {"E9", "Z"},
        {"EA", "²"},
        {"EB", "Ô"},
        {"EC", "\\"},
        {"ED", "Ò"},
        {"EE", "Ó"},
        {"EF", "Õ"},
        {"F0", "0"},
        {"F1", "1"},
        {"F2", "2"},
        {"F3", "3"},
        {"F4", "4"},
        {"F5", "5"},
        {"F6", "6"},
        {"F7", "7"},
        {"F8", "8"},
        {"F9", "9"},
        {"FA", "³"},
        {"FB", "Û"},
        {"FC", "]"},
        {"FD", "Ù"},
        {"FE", "Ú"},
        {"FF", "*"}
};


// Function to get the description by code
const char* getDescription(const char *code) {
    int n = sizeof(mappings) / sizeof(CodeMap);
    for (int i = 0; i < n; i++) {
        if (strcmp(mappings[i].code, code) == 0) {
            return mappings[i].description;
        }
    }
    return "Unknown";
}

// Function to translate a string of codes to the full description
void translateCodeString(const char *input, char *output) {
    char code[3] = {0};  // To hold each two-character code
    output[0] = '\0';    // Ensure output is empty to start

    while (*input) {
        strncpy(code, input, 2);  // Copy two characters
        code[2] = '\0';  // Ensure null-termination for safety
        const char* description = getDescription(code);
        strcat(output, description);  // Append description to output
        strcat(output, " ");          // Add a space between descriptions
        input += 2;  // Move to the next pair of characters
    }
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

                translateCodeString(hexText, pRes);

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
