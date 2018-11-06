


#include "navputter.h"

#define NMEA_LIST\
    NMEA( MINMEA_SENTENCE_RMC, minmea_sentence_rmc, minmea_parse_rmc, nmea_rmc_in ) \
    NMEA( MINMEA_SENTENCE_GGA, minmea_sentence_gga, minmea_parse_gga, nmea_gga_in ) \
    NMEA( MINMEA_SENTENCE_GSA, minmea_sentence_gsa, minmea_parse_gsa, nmea_gsa_in ) \
    NMEA( MINMEA_SENTENCE_GLL, minmea_sentence_gll, minmea_parse_gll, nmea_gll_in ) \
    NMEA( MINMEA_SENTENCE_GST, minmea_sentence_gst, minmea_parse_gst, nmea_gst_in ) \
    NMEA( MINMEA_SENTENCE_VTG, minmea_sentence_vtg, minmea_parse_vtg, nmea_vtg_in ) \
    NMEA( MINMEA_SENTENCE_ZDA, minmea_sentence_zda, minmea_parse_zda, nmea_zda_in )


void nmea_rmc_in( struct minmea_parse_rmc *rmc )
{
    dbgprint("I got rmc\n");
}    

void nmea_gga_in( struct minmea_parse_gga *gga )
{
    dbgprint("I got gga\n");
}    

void nmea_gsa_in( struct minmea_parse_gsa *gsa )
{
    dbgprint("I got gsa\n");
}    

void nmea_gll_in( struct minmea_parse_gsa *gll )
{
    dbgprint("I got gll\n");
}    

void nmea_gst_in( struct minmea_parse_gst *gst )
{
    dbgprint("I got gst\n");
}    
void nmea_gsv_in( struct minmea_parse_gst *gsv )
{
    dbgprint("I got gsv\n");
}    
void nmea_vtg_in( struct minmea_parse_vtg *vtg )
{
    dbgprint("I got vtg\n");
}    
void nmea_zda_in( struct minmea_parse_zda *vzda )
{
    dbgprint("I got zda\n");
}    


void nmea_input( FILE *fp, char *input )
{
    uint8_t id = minmea_sentence_id minmea_sentence_id(input, 1);
#define NMEA(e,f,p,t)
    {\
        t parse;\
        if ( e == id )\
        {\
            if ( f(&parse,input) )\
            {\
                fprintf(fp, "nmea valid\n\r");\
            }\
            else\
            {\
                fprintf(fp, "nmea invalid: %s\n\r", input );\
            }\
        }\
    }

NMEA_LIST
#undef NMEA
}

