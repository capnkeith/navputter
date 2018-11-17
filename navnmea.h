#ifndef __NAVNMEA_H__
#define __NAVNMEA_H__


void nmea_rmc_in( struct minmea_sentence_rmc *rmc );
void nmea_gga_in( struct minmea_sentence_gga *gga );
void nmea_gsa_in( struct minmea_sentence_gsa *gsa );
void nmea_gll_in( struct minmea_sentence_gll *gll );
void nmea_gst_in( struct minmea_sentence_gst *gst );
void nmea_gsv_in( struct minmea_sentence_gsv *gsv );
void nmea_vtg_in( struct minmea_sentence_vtg *vtg );
void nmea_zda_in( struct minmea_sentence_zda *zda );
void nmea_input( FILE *fp, char *input );

#endif /* __NAVNMEA_H__ */
