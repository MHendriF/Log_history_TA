// Demo Code for Heatshrink (Copyright (c) 2013-2015, Scott Vokes <vokes.s@gmail.com>)
// embedded compression library
// Craig Versek, Apr. 2016

#include <stdint.h>
#include <ctype.h>
#include <Arduino.h>
#include <String.h>

#include <heatshrink_encoder.h>
#include <heatshrink_decoder.h>
#include <greatest.h>

#define arduinoLED 13   // Arduino LED on board

/******************************************************************************/
// TEST CODE from adapted from test_heatshrink_dynamic.c
#if !HEATSHRINK_DYNAMIC_ALLOC
#error Must set HEATSHRINK_DYNAMIC_ALLOC to 1 for dynamic allocation test suite.
#endif

typedef struct {
    uint8_t log_lvl;
    uint8_t window_sz2;
    uint8_t lookahead_sz2;
    size_t decoder_input_buffer_size;
} cfg_info;

static void dump_buf(char *name, uint8_t *buf, uint16_t count) {
    for (int i=0; i<count; i++) {
        uint8_t c = (uint8_t)buf[i];
        printf("%s %d: 0x%02x ('%c')\n", name, i, c, isprint(c) ? c : '.');
    }
}

static int compress_and_expand_and_check(uint8_t *input, 
                                         uint32_t input_size, 
                                         cfg_info *cfg, 
                                         uint8_t *output,
                                         uint32_t &output_size) 
                                         {
    heatshrink_encoder *hse = heatshrink_encoder_alloc(cfg->window_sz2,
        cfg->lookahead_sz2);
    size_t count = 0;
    size_t sunk = 0;
    size_t polled = 0;
    
    if (cfg->log_lvl > 1) {
        Serial.print(F("\n^^ COMPRESSING\n"));
        dump_buf("input", input, input_size);
    }
    
    while (sunk < input_size) {
        HSE_sink_res esres = heatshrink_encoder_sink(hse, &input[sunk], input_size - sunk, &count);
        sunk += count;
        if (cfg->log_lvl > 1){
          Serial.print(F("^^ sunk "));
          Serial.print(count);
          Serial.print(F("\n"));
        }
        if (sunk == input_size) {
            heatshrink_encoder_finish(hse);
        }

        HSE_poll_res pres;
        do {                    /* "turn the crank" */
            pres = heatshrink_encoder_poll(hse,
                                           &output[polled],
                                           output_size - polled,
                                           &count);
            //ASSERT(pres >= 0);
            polled += count;
            if (cfg->log_lvl > 1){
              Serial.print(F("^^ polled "));
              Serial.print(count);
              Serial.print(F("\n"));
            }
        } while (pres == HSER_POLL_MORE);
        if (polled >= output_size) 
          Serial.print(F("FAIL: Compression should never expand that much!"));
        if (sunk == output_size) {
            heatshrink_encoder_finish(hse);
        }
    }
    if (cfg->log_lvl > 0){
      Serial.print(F("in: "));
      Serial.print(input_size);
      Serial.print(F(" compressed: "));
      Serial.print(polled);
      Serial.print(F(" \n")); 
      
      Serial.print("Compressed data: ");
      for(int i = 0; i < polled; i++){
        Serial.print(output[i]);
        Serial.print(", ");
      }Serial.println();

      Serial.print(output[0]);
      Serial.print("\n");
      for(int i = 1; i < polled; i++){
        if(i % 13 == 0){
          Serial.print(output[i]);
          Serial.print("\n");
          delay(3000);
        }else{
          Serial.print(output[i]);
          Serial.print("\n");
        }
      }
    }
    return polled;
}


/******************************************************************************/
#define BUFFER_SIZE 1647
uint8_t orig_buffer[BUFFER_SIZE];
uint8_t comp_buffer[BUFFER_SIZE];

int main(int argc, char **argv)
{
    init(); // this is needed

    pinMode(arduinoLED, OUTPUT);      // Configure the onboard LED for output
    digitalWrite(arduinoLED, LOW);    // default to LED off
    Serial.begin(9600);
    delay(1000);
    uint32_t length_data;
   
   
    //100
    //uint8_t test_data[] = {'1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1'};
    //248
    //uint8_t test_data [] = "<div class=\"panel panel-default\"><div id=\"heading_1\" class=\"panel-heading activestate\"><a href=\"#collapse_1\" data-toggle=\"collapse\" data-parent=\"#accordion_1\">1. Maksimal Upload lampiran data</a></div><div id=\"collapse_1\" class=\"panel-collapse col";
    //292
    //uint8_t test_data [] = "<div class=\"panel panel-default\"><div id=\"heading_1\" class=\"panel-heading activestate\"><a href=\"#collapse_1\" data-toggle=dsfsfds sdssdsdfsd\"collapse\" data-parent=\"#accordion_1\">1. Maksimal Upload lampiran data</a></div><div id=\"collapse_1\" class=\"panel-collapse col dasdasd a9da0fa9d8a098221";
    //uint8_t test_data [] = "e8h5888e8h5888e8h5k4u6vthak6qtwxjwwabbfn9b5tdasdafdg334adsdsad98das89dds43dpp3vug3x7z62yqbcvj78ue66kq8qzbamg data-toggle=\"collapse\" data-parent=\"#accordion_1\">1. Maksimal Upload lampiran data</a></div><div id=\"collapse_1\" class=\"panel-collapse col dasdasd a9da0fa9d8a098221";
    //350
    //uint8_t test_data [] = "<div class=\"panel panel-default\"><div id=\"heading_1\" class=\"panel-heading activestate\"><a href=\"#collapse_1\" data-toggle=\"collapse\" data-parent=\"#accordion_1\">1. Maksimal Upload lampiran data</a></div><div id=\"collapse_1\" class=\"panel-collapse col dasdasd a9da0fa9d8a098221\2><div id=\"heading_1\" class=\"panel-heading active dasasf23 <div><div> dfsfgg";
    //408
    //uint8_t test_data [] = "e8h5888e8h5888e8h5k4u6vthak6qtwxjwwabbfn9b5tdasdafdg334adsdsad98das89dds43dpp3vug3x7z62yqbcvj78ue66kq8qzbamgcollapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne oratio delenit senserit.&nbsp;</div></div><div id=\"collapse_2\" class=\"panel-collapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne";
    //467
    //char test_data [] = "e8h5888e8h5888e8h5888yyxnyyxny454yyxnqx5e7yyxntu98xge9pdgzycb7had5q3vdcfgh3333338juxcn9vdd6nm33cccop3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vug3x7z62yqbcvj78ue66kq8qzbamgcollapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne oratio delenit senserit.&nbsp;</div></div><div id=\"collapse_2\" class=\"panel-collapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne";
    //557
    //char test_data [] = "e8h5888e8h5888e8h5888yyxnyyxny454yyxnqx5e7yyxntu98xge9pdgzycb7had5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvvc828dctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vug3xcjpp5k8cxum64m4khaurf6tzqy3wvsnzb7ax5px2avreuaf5jwtv382vvhdca6n7z62yqbcvj78ue66kq8qzbamgcollapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne oratio delenit senserit.&nbsp;</div></div><div id=\"collapse_2\" class=\"panel-collapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne";
    //584
    //uint8_t test_data [] = "</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vug3xcjpp5k8cxmcx4d8cp5um64m4khaurf6tzqy3wvsnzb7ax5px2avreuaf5jwtv382vvhdca6n7z62yqbcvj78ue66kq8qzbamgcollapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne oratio delenit senserit.&nbsp;</div></div><div id=\"collapse_2\" class=\"panel-collapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed neest affert ocurreret cu3242";
    //uint8_t test_data [] = "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
    //584++
    //uint8_t test_data [] = "</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vug3xcjpp5k8cxmcx4d8cp5um64m4khaurf6tzqy3wvsnzb7ax5px2avreuaf5jwtv382vvhdca6n7z62yqbcvj78ue66kq8qzbamgcollapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne oratio delenit senserit.&nbsp;</div></div><div id=\"collapse_2\" class=\"panel-collapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed neest affert ocurreret cu324224</div></div><div id=collapse_298xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnw";

    //980
    //uint8_t test_data [] = "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
    //uint8_t test_data [] = "</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vug3xcjpp5k8cxmcx4d8cp5um64m4khaurf6tzqy3wvsnzb7ax5px2avreuaf5jwtv382vvhdca6n7z62yqbcvj78ue66kq8qzbamgcollapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne oratio delenit senserit.&nbsp;</div></div><div id=\"collapse_2\" class=\"panel-collapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed neest affert ocurreret cu3242 </div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vu</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vu</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge </div></div><di";
    //uint8_t test_data[] = "XxcRJmRGFk9r6NDj62ZVbL7ABJ8wRZb75zPmYSHwXxDvDL9cc9vUb8V5mE57Ys55MQyAc2GvxGqrjBKzhLcDZNrzN8HmqufEhg8jgPEdWU8dJ9hLJKFV2rM8nLkfJvtKzN8HmqufEhg8jgPEdWU8dJ9hLJKFV2rM8nLkfJvtK87875XxcRJmRGFk9r6NDj62ZVbL7ABJ8wRZb75zPmYSHwXxDvDL9cc9vUb8V5mE57Ys55MQyAc2GvxGqrjBKzhLcDZNrzN8HmqufEhg8jgPEdWU8dJ9hLJKFV2rM8nLkfJvtKzN8HmqufEhg8jgPEdWU8dJ9hLJKFV2rM8nLkfJvtK87875XxcRJmRGFk9r6NDj62ZVbL7ABJ8wRZb75zPmYSHwXxDvDL9cc9vUb8V5mE57Ys55MQyAc2GvxGqrjBKzhLcDZNrzN8HmqufEhg8jgPEdWU8dJ9hLJKFV2rM8nLkfJvtKzN8HmqufEhg8jgPEdWU8dJ9hLJKFV2rM8nLkfJvtK87875XxcRJmRGFk9r6NDj62ZVbL7ABJ8wRZb75zPmYSHwXxDvDL9cc9vUb8V5mE57Ys55MQyAc2GvxGqrjBKzhLcDZNrzN8HmqufEhg8jgPEdWU8dJ9hLJKFV2rM8nLkfJvtKzN8HmqufEhg8jgPEdWU8dJ9hLJKFV2rM8nLkfJvtK87875XxcRJmRGFk9r6NDj62ZVbL7ABJ8wRZb75zPmYSHwXxDvDL9cc9vUb8V5mE57Ys55MQyAc2GvxGqrjBKzhLcDZNrzN8HmqufEhg8jgPEdWU8dJ9hLJKFV2rM8nLkfJvtKzN8HmqufEhg8jgPEdWU8dJ9hLJKFV2rM8nLkfJvtK87875XxcRJmRGFk9r6NDj62ZVbL7ABJ8wRZb75zPmYSHwXxDvDL9cc9vUb8V5mE57Ys55MQyAc2GvxGqrjBKzhLcDZNrzN8HmqufEhg8jgPEdWU8dJ9hLJKFV2rM8nLkfJvtKz";
    
    //1280
    //uint8_t test_data [] = "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
    //uint8_t test_data [] = "</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthapk6qtwxjwwabbfn9b5t3vug3xcjpp5k8cxmcx4d8cp5um64m4khaurf6tzqy3wvsnzb7ax5px2avreuaf5jwtv382vvhdca6n7z62yqbcvj78ue66kq8qzbamgcollapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne oratio delenit senserit.&nbsp;</div></div><div id=\"collapse_2\" class=\"panel-collapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed neest affert ocurreret cu3242 </div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vu</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vu</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge </div></div><di</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabb</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabb333";
    
    //1345
    //uint8_t test_data [] = "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
    //uint8_t test_data [] = "</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juvdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthapk6qtwxjwwabbfn9b5t3vug3xcjpp5k8cxmcx4d8cp5um64m4khaurf6tzqy3wvsnzb7ax5px2avreuaf5jwtv382vvhdca6n7z62yqbcvj78ue66kq8qzbamgcollapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne oratio delenit senserit.&nbsp;</div></div><div id=\"collapse_2\" class=\"panel-collapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed neest affert ocurreret cu3242 </div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vu</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vu</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge </div></div><di</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabb</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabb333</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxnt4449pdhad5";

    //commpress > raw
    //uint8_t test_data [] = "h8x95WUWuKLReEkRrTQ5tdjxrC5QENqyUrxf6U4H4hLFwrh3CGUGJnrWekp0PxNckVUqRU3nFV6NBPBJM5DuGxNHmtfNqbHDuakW4ZpRSQHLCsZJaAvBqrBzFdwF7GHPmCHE46FdvF5twpWUKfPPpHA5QU6SrPSQkGZQ9jKh6WPmGYwDep3wyetjeEfrn7aeqYPu7UVaPrLD2PJ4tHZdnL2mxEFpzgFV2Z7xm9eZZjfDsaCNSbkfHUz8wJpq6xA6J7eWUrjTUNMZmhYvPbKRFHajvNB2K4rYm45aBVcK357aXDvnsUEyuChJfdLPy92pu6QaZachBnTzksyH2W4YCAGWBBx8TpQkXeCKgqq4m4neNKRA4Zj9Sw9tuzBkRhm4y98DZN3ca5HhGEPxMbYUbceNDqS8XQ3PUqY789Q3fYW5UbwfNbVwuAC6eZjb7TtT9yDy9EhB3CyLTXaSnVRtJMdkbTUJp9FxpVDTe2RcRtkE3GaXSTcgSSD3UpaZANehZ5kKNm8vrVQKNGZM9YaqWWxUFHF35RbFB6QbzypFqEyJYjdEBwYUrtY58kumMeKjhpem9SghmjBFPfkvxgJuAMUFRVbQjG4eHYVkMmuvZpYswWY8SZ2uqjwTXsD6wjmX4vbDwmtaeQ9AMrXJLbDvBy8gG4auWL8xSNg5vfu9WUuUaV6tjkDD8LWKfCuDfsXyLtfu4d5JpqzkhuwpgZUyTKZTmekzyxjPr5QwgHfSNPzY6Ddq6aBbFMBLLcS5spcSwrw2aaC2tLajJKfmB4LwxEKTPmGqLEMBBnJ9dDX8yCsUSCztj43RjkY9rvtzSMQSS8HjRXzRe87wjQACBseMfzGY9LXAfWxXyKdsZnwzRzPabxsgG4kKrkDqg7W37Hwmbh4Mg4zUZ7LEQMceMtDMwsBELPYCx5bcfFuUmPUZbCpF5jMPQdyXGE44nDqZ88k5rxERkbtjAyRzmkGt4v9v4DXnXB4XdqPysd99RHyQ8x4ENDZHAkEs3X4rLzJXZymZ8REab3RFajG2EeabGwP77CdXudDsWRUsYqc26NSZw3GvHykbe7NHBWJpFgG6A2Z7JSynPRJQCac6emSaXWFMfA7dR5k4MnhJnBB3EVNkhG74PdXPgqCzb5csVM2LhJZyQcgc9yMaQhbvPCpQq8QkqpuvScHrCzPPbRrgmxKSL3Hx4udUCqqTVusPc4Qv4Csfsnu9xjd5QbcQ5vSXpeF5BsyS5RR5BK7SBj3KuYL24drbKD7NcjQa7DKtJbtSER4kR9aQksBPV88d8LEhD84HdLyxzBUCzGxukBqRzFR6KYsTBEvmVTvPSRjYZdvUDXr8";
    
    //
    //uint8_t test_data [] = "</div></div><div id=\"collapse_2\" class=xny454yyxdvd3u2sv9qjkz5k4u6vthapk6qtwxjwwabbfn9b5t3vug3xcyy9pdhad5q3vdcfgh3333338juvdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthapk6qtwxjwwabbfn9b5t3vug3xcq3vdcfgh3333338juvdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthapk6qtwxjwwabbfn9b5t3vug3xcjpp5k8cxmcx4d8cp5um64m4khaurf6tzqy3wvsnzb7ax5px2avreuaf5jwtv382vvhdca6n7z62yqbcvj78ue66kq8qzbamgcollapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne oratio delenit senserit.&nbsp;</div></div><div id=\"collapse_2\" class=\"panel-collapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed neest affert ocurreret cu3242 </div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vu</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vu</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge </div></div><di</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabb</div></div><div id=\"collapse_2\" class=xny4";

    //1647
    //uint8_t test_data [] = "</div></div><div id=\"collapse_2\" class=xny454yyxdvd3u2sv9qjkz5k4u6vthapk6qtwxjwwabbfn9b5t3vug3xcyy9pdhad5q3vdcfgh3333338juvdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthapk6qtwxjwwabbfn9b5t3vug3xcq3vdcfgh3333338juvdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthapk6qtwxjwwabbfn9b5t3vug3xcjpp5k8cxmcx4d8cp5um64m4khaurf6tzqy3wvsnzb7ax5px2avreuaf5jwtv382vvhdca6n7z62yqbcvj78ue66kq8qzbamgcollapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne oratio delenit senserit.&nbsp;</div></div><div id=\"collapse_2\" class=\"panel-collapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed neest affert ocurreret cu3242 </div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vu</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vu</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge </div></div><di</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabb</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabb333</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxnt4449pdhad5yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juvdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthapk6qtwxjwwabbfn9b5t3vug3xcyyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juvdd6nm33cccnwdr79bn9b5t3oc";
    uint8_t test_data [] = "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
    
    //1750
    //uint8_t test_data [] = "</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhnwdr79bcvctdvd3u2sv9qjkz5k4u6vthapk6qtwxjwwabbfn9b5t3vug3xcyyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juvdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthapk6qtwxjwwabbfn9b5t3vug3xcq3vdcfgh3333338juvdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthapk6qtwxjwwabbfn9b5t3vug3xcjpp5k8cxmcx4d8cp5um64m4khaurf6tzqy3wvsnzb7ax5px2avreuaf5jwtv382vvhdca6n7z62yqbcvj78ue66kq8qzbamgcollapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne oratio delenit senserit.&nbsp;</div></div><div id=\"collapse_2\" class=\"panel-collapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed neest affert ocurreret cu3242 </div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vu</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vu</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge </div></div><di</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabb</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabb333</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxnt4449pdhad5yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juvdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthapk6qtwxjwwabbfn9b5t3vug3xcyyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juvdd6nm33cccnwdr79bn9b5t3vug3xctdvd3u2sv9qjkz5k4u6vctdvd3u2sv9qjkz5k4u6v3333333c";

   // uint8_t test_data [] = "('../fonts/fontawesome/fontawesome-webfont.eot?v=4.6.3');src:url('../fonts/fontawesome/fontawesome-webfont.eot?#iefix&v=4.6.3') format('embedded-opentype'),url('../fonts/fontawesome/fontawesome-webfont.woff2?v=4.6.3') format('woff2'),url('../fonts/fontawesome/fontawesome-webfont.woff?v=4.6.3') format('woff'),url('../fonts/fontawesome/fontawesome-webfont.ttf?v=4.6.3') format('truetype'),url('../fonts/fontawesome/fontawesome-webfont.svg?v=4.6.3#fontawesomeregular') format('svg');font-weight:normal;font-style:normal}.fa{display:inline-block;font:normal normal normal 14px/1 FontAwesome;font-size:inherit;text-rendering:auto;-webkit-font-smoothing:antialiased;-moz-osx-font-smoothing:grayscale}.fa-lg{font-size:1.33333333em;line-height:.75em;vertical-align:-15%}.fa-2x{font-size:2em}.fa-3x{font-size:3em}.fa-4x{font-size:4em}.fa-5x{font-size:5em}.fa-fw{width:1.28571429em;text-align:center}.fa-ul{padding-left:0;margin-left:2.14285714em;list-style-type:none}.fa-ul>li{position:relative}.fa-li{position:absolute;left:-2.14285714em;width:2.14285714em;top:.14285714em;text-align:center}.fa-li.fa-lg{left:-1.85714286em}.fa-border{padding:.2em .25em .15em;border:solid .08em #eee;border-radius:.1em}.fa-pull-left{float:left}.fa-pull-right{float:right}.fa.fa-pull-left{margin-right:.3em}.fa.fa-pull-right{margin-left:.3em}.pull-right{float:right}.p";
   // uint8_t test_data [] = "0deg)}100%{-webkit-transform:rotate(359deg);transform:rotate(359deg)}}@keyframes fa-spin{0%{-webkit-transform:rotate(0deg);transform:rotate(0deg)}100%{-webkit-transform:rotate(359deg);transform:rotate(359deg)}}.fa-rotate-90{-ms-filter:\"progid:DXImageTransform.Microsoft.BasicImage(rotation=1)\";-webkit-transform:rotate(90deg);-ms-transform:rotate(90deg);transform:rotate(90deg)}.fa-rotate-180{-ms-filter:\"progid:DXImageTransform.Microsoft.BasicImage(rotation=2)\";-webkit-transform:rotate(180deg);-ms-transform:rotate(180deg);transform:rotate(180deg)}.fa-rotate-270{-ms-filter:\"progid:DXImageTransform.Microsoft.BasicImage(rotation=3)\";-webkit-transform:rotate(270deg);-ms-transform:rotate(270deg);transform:rotate(270deg)}.fa-flip-horizontal{-ms-filter:\"progid:DXImageTransform.Microsoft.BasicImage(rotation=0, mirror=1)\";-webkit-transform:scale(-1, 1);-ms-transform:scale(-1, 1);transform:scale(-1, 1)}.fa-flip-vertical{-ms-filter:\"progid:DXImageTransform.Microsoft.BasicImage(rotation=2, mirror=1)\";-webkit-transform:scale(1, -1);-ms-transform:scale(1, -1);transform:scale(1, -1)}:root .fa-rotate-90,:root .fa-rotate-180,:root .fa-rotate-270,:root .fa-flip-horizontal,:root .fa-flip-vertical{filter:none}.fa-stack{position:relative;display:inline-block;width:2em;height:2em;line-height:2em;vertical-align:middle}.fa-stack-1x,.fa-stack-2ontent.fa-music:before{content:}";

    //1345
    //uint8_t test_data [] = "</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juvdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthapk6qtwxjwwabbfn9b5t3vug3xcjpp5k8cxmcx4d8cp5um64m4khaurf6tzqy3wvsnzb7ax5px2avreuaf5jwtv382vvhdca6n7z62yqbcvj78ue66kq8qzbamgcollapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed ne oratio delenit senserit.&nbsp;</div></div><div id=\"collapse_2\" class=\"panel-collapse collapse in\"><div class=\"panel-body pa-15\">Lorem ipsum dolor sit amet, est affert ocurreret cu, sed neest affert ocurreret cu3242 </div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vu</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabbfn9b5t3vu</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge </div></div><di</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabb</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxntu98xge9pdhad5q3vdcfgh3333338juxcn9vdd6nm33cccnwdr79bcvctdvd3usv9qjkz5k4u6vthak6qtwxjwwabb333</div></div><div id=\"collapse_2\" class=xny454yyxnqx5e7yyxnt4449pdhad5";

    length_data = sizeof(test_data)/sizeof(test_data[0]);

    if(length_data > 1647){
      Serial.print("Data terlalu besar, data yang muat untuk kompresi maksimal 1345 karakter");
      delay(4000);
      return 0;
    }

    uint32_t comp_size   = BUFFER_SIZE; //this will get updated by reference
    memcpy(orig_buffer, test_data, length_data);
    size_t polled = 0;
    
    cfg_info cfg;
    cfg.log_lvl = 1;
    cfg.window_sz2 = 4;
    cfg.lookahead_sz2 = 3;
     
//    if(length_data <= 248){
//      cfg.window_sz2 = 8;
//      cfg.lookahead_sz2 = 4;
//    }else if(length_data > 248 && length_data <= 517){
//      cfg.window_sz2 = 6;
//      cfg.lookahead_sz2 = 3;
//    }else if(length_data > 517 && length_data <= 561){
//      cfg.window_sz2 = 5;
//      cfg.lookahead_sz2 = 3;
//    }else if(length_data > 561 && length_data <= 584){
//      cfg.window_sz2 = 4;
//      cfg.lookahead_sz2 = 3;
//    }
    cfg.decoder_input_buffer_size = 64;
    polled = compress_and_expand_and_check(orig_buffer, length_data, &cfg, comp_buffer, comp_size);
    //Serial.print(polled);

    Serial.println("Compressed data: ");
    for(int i = 0; i < polled; i++){
      Serial.print(comp_buffer[i]);
      Serial.print(",");
    }Serial.print("\n");
    
//    Serial.print(comp_buffer[0]);
//    Serial.print("\n");
//    for(int i = 1; i < polled; i++){
//        if(i % 16 == 0){
//          Serial.print(comp_buffer[i]);
//          Serial.print("\n");
//          delay(3000);
//        }else{
//          Serial.print(comp_buffer[i]);
//          Serial.print("\n");
//        }
//    }
    delay(3000);
    
    //lenght data original
    Serial.print("a");
    Serial.print(length_data);
    Serial.print("\n");

    //config
    Serial.print("b");
    Serial.print(cfg.window_sz2);
    Serial.print("\n");
    Serial.print("c");
    Serial.print(cfg.lookahead_sz2);
    Serial.print("\n");
    Serial.print("d");
    Serial.print(cfg.decoder_input_buffer_size);
    Serial.print("\n");

    //Polled
    Serial.print("f");
    Serial.print(polled);
    Serial.print("\n");
   
    
    for ( ;; ){
      //Serial.print("a100");
      //Serial.print("\n");
      delay(3000);
    }
        
}
