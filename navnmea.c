


#include "navputter.h"

#define NMEA_LIST\
    NMEA( MINMEA_SENTENCE_RMC, minmea_sentence_rmc, minmea_parse_rmc, nmea_rmc_in ) \
    NMEA( MINMEA_SENTENCE_GGA, minmea_sentence_gga, minmea_parse_gga, nmea_gga_in ) \
    NMEA( MINMEA_SENTENCE_GSA, minmea_sentence_gsa, minmea_parse_gsa, nmea_gsa_in ) \
    NMEA( MINMEA_SENTENCE_GLL, minmea_sentence_gll, minmea_parse_gll, nmea_gll_in ) \
    NMEA( MINMEA_SENTENCE_GST, minmea_sentence_gst, minmea_parse_gst, nmea_gst_in ) \
    NMEA( MINMEA_SENTENCE_VTG, minmea_sentence_vtg, minmea_parse_vtg, nmea_vtg_in ) \
    NMEA( MINMEA_SENTENCE_ZDA, minmea_sentence_zda, minmea_parse_zda, nmea_zda_in )

extern FILE *gfp;

void deg_to_dms( float dec, uint8_t lat, int16_t *deg, int16_t *min, float *sec, char *dir );

void deg_to_dms( float dec, uint8_t lat, int16_t *deg, int16_t *min, float *sec, char *dir )
{
    if ( lat )
    {
        if ( dec > 0 ) *dir = 'N';
        else *dir = 'S';
    }
    else
    {
        if ( dec > 0 ) *dir = 'E';
        else *dir = 'W';
    }
    dec = (dec < 0)?dec * -1 : dec;
    float d = trunc( dec );
    float rem = dec - d;
    rem = (rem < 0)?rem * -1 : rem;
    rem *= 60;
    float m = trunc( rem );
    rem -= m;

    rem *= 60;

    *deg = (int16_t)d;
    *min = (int16_t)m;
    *sec = (float)rem;

}    
 

void nmea_rmc_in( struct minmea_sentence_rmc *rmc )
{
    int16_t deg=0;
    int16_t min=0;
    float sec=0;
    char dir=0;
    printtext(0,0, "TIME:  %d-%d-%d", rmc->time.hours, rmc->time.minutes, rmc->time.seconds );
    printtext(0,10, "DATE: %d-%d-%d", rmc->date.day, rmc->date.month, rmc->date.year );
    deg_to_dms( minmea_tocoord( &rmc->latitude ), 1, &deg, &min, &sec, &dir );
    printtext(0,20, "LAT: %d*%d'%2.4f %c\n", deg, min, sec, dir );
    deg_to_dms( minmea_tocoord( &rmc->latitude ), 0, &deg, &min, &sec, &dir );
    printtext(0,30, "LON: %d*%d'%2.4f %c\n", deg, min, sec, dir );
    printtext(0,40, "SPEED: %2.1f knots", minmea_tofloat( &rmc->speed ) );
    printtext(0,50, "COURSE: %3.2f deg", minmea_tofloat( &rmc->course ));
}    

void nmea_gga_in( struct minmea_sentence_gga *gga )
{
    int16_t deg=0;
    int16_t min=0;
    float sec=0;
    char dir=0;
    printtext(0,0, "TIME:  %d-%d-%d", gga->time.hours, gga->time.minutes, gga->time.seconds );
    deg_to_dms( minmea_tocoord( &gga->latitude ), 1, &deg, &min, &sec, &dir );
    printtext(0,10, "LAT: %d*%d'%2.4f %c\n", deg, min, sec, dir );
    deg_to_dms( minmea_tocoord( &gga->longitude), 0, &deg, &min, &sec, &dir );
    printtext(0,20, "LON: %d*%d'%2.4f %c\n", deg, min, sec, dir );
    printtext(0,30, "FIX: %d\n", gga->fix_quality );
    printtext(0,40, "SATS: %d\n", gga->satellites_tracked);
}    

void nmea_gsa_in( struct minmea_sentence_gsa *gsa )
{
    dbgprint("I got gsa\n");
}    

void nmea_gll_in( struct minmea_sentence_gsa *gll )
{
    dbgprint("I got gll\n");
}    

void nmea_gst_in( struct minmea_sentence_gst *gst )
{
    dbgprint("I got gst\n");
}    
void nmea_gsv_in( struct minmea_sentence_gst *gsv )
{
    dbgprint("I got gsv\n");
}    
void nmea_vtg_in( struct minmea_sentence_vtg *vtg )
{
    printtext(0,0, "TRUE: %2.2f deg\n", minmea_tofloat(&vtg->true_track_degrees ));
    printtext(0,10, "MAG : %2.2f deg\n", minmea_tofloat(&vtg->magnetic_track_degrees));
    printtext(0,20, "SPEED: %2.2f knots\n", minmea_tofloat(&vtg->speed_knots ));
    printtext(0,30, "SPEED: %2.2f kph\n", minmea_tofloat(&vtg->speed_kph));
};

void nmea_zda_in( struct minmea_sentence_zda *vzda )
{
    dbgprint("I got zda\n");
}    


void nmea_input( FILE *fp, char *input )
{
    uint8_t id = minmea_sentence_id(input, 0);
    fprintf(fp, "setence id =%d\n\r",id);
#define NMEA(e,t,p,f)\
    {\
        struct t parse;\
        if ( e == id )\
        {\
            if ( p(&parse,input) )\
            {\
                fprintf(fp, "nmea valid\n\r");\
                f(&parse);\
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

