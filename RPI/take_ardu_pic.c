/* Adopted from arducamstill.c by 65sc02
 * added further options (e.g., focus)
 * cleaned up the code
 * made options work 
 * removed preview and loop, so the program can be called in a shell script
 *
 * If you like this, please drop me a note via GIT.
 *
 * No warranties whatsoever, use at your own risk.
 */



#include "arducam_mipicamera.h"
#include <linux/v4l2-controls.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#define LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)
//#define LOG(fmt, args...) 


                   //IMAGE_ENCODING_JPEG
                   //IMAGE_ENCODING_BMP
                   //IMAGE_ENCODING_PNG
IMAGE_FORMAT fmt = {IMAGE_ENCODING_JPEG, 77};


typedef struct{
  int hflip;
  int vflip;
  int frameCnt;
  int exposureVal;
  int focusVal;
  int redGain ;
  int blueGain;
  int gain;
  int key;
  int awb;
  int autoex;
  int timeout;                        // Time taken before frame is grabbed and app then shuts down. Units are milliseconds
}GLOBAL_VAL;

GLOBAL_VAL globalParam; 


static struct
{
   char *format;
   uint32_t encoding;
} encoding_xref[] =
  {
     {"jpg", IMAGE_ENCODING_JPEG},
     {"bmp", IMAGE_ENCODING_BMP},
     {"png", IMAGE_ENCODING_PNG},
     {"raw", IMAGE_ENCODING_RAW_BAYER},
  };

typedef struct
{
   int id;
   char *command;
   char *abbrev;
   char *help;
   int num_parameters;
} COMMAND_LIST;

typedef struct
{
   int timeout;                        // Time taken before frame is grabbed and app then shuts down. Units are milliseconds
   int focus ;                         // focus value
   int quality;                        // JPEG quality setting (1-100)
   uint32_t encoding;                  // Encoding to use for the output file.
   int rgain;                          // red gain compensation
   int bgain;                          // blue gain compensation
   int gain;                           // overall gain
   int exposure;                       // exposure
   int mode;                           // sensor mode
   int awb_state;                      // auto white balance state
   int ae_state;                       // auto exposure state
   int glCapture;                      // Save the GL frame-buffer instead of camera output
   char *linkname;                     // filename of output file
   int hflip;
   int vflip;
} RASPISTILL_STATE;


enum
{
   CommandTimeout,
   CommandFocus,
   CommandQuality,
   CommandMode,
   CommandAutowhitebalance,
   CommandAutoexposure,
   CommandRgain,
   CommandBgain,
   CommandCapture,
   CommandRaw,
   CommandEncoding,
   CommandGain,
   CommandExposure,
   CommandHelp,
   CommandVFlip,
   CommandHFlip
};

static COMMAND_LIST cmdline_commands[] =
{
   { CommandTimeout, "-timeout",                      "t",      "Time (in ms) before it takes a picture and shuts down, default 5000", 1 },
   { CommandFocus, "-focus",                          "f",      "Set focus, <0 to 1000>, 0 is far away (intinity)", 1 }, /* 1/focus = distance */
   { CommandQuality, "-quality",                      "q",      "Set jpeg quality <1 to 100>", 1 },
   { CommandMode, "-mode",                            "m",      "Set sensor mode, default: 4", 1},
   { CommandAutowhitebalance, "-autowhitebalance",    "awb",    "Enable (1) or disable (0) awb", 1 },
   { CommandAutoexposure, "-autoexposure",            "ae",     "Enable (1) or disable (0) ae", 1 },
   { CommandRgain, "-awbrgain",                       "rgain",  "Set R channel gain vaue <0 to 4095>", 1 },
   { CommandBgain, "-awbbgain",                       "bgain",  "Set B channel gain vaue <0 to 4095>", 1 },
   { CommandGain, "-gain",                            "g",      "Overall gain (ISO) value <0 to 4095>", 1 },
   { CommandExposure, "-exposure",                    "x",      "Exposure time <0 to 65535>", 1 },
   { CommandCapture, "-capture",                      "o",      "file name for one frame, default: /run/mqtt/mode4.jpg ", 0},
   { CommandEncoding,"-encoding",                     "e",      "Encoding to use for output file (raw, jpg, bmp, gif, png)", 1},
   { CommandVFlip,"-vflip",                           "V",      "flip picture vertically 0 or 1", 1},
   { CommandHFlip,"-hflip",                           "H",      "flip picture horizontally 0 or 1", 1},
   { CommandHelp, "-help",                            "h",      "This help information", 0},
};



void raspicli_display_help(const COMMAND_LIST *commands, const int num_commands)
{
   int i;

   //vcos_assert(commands);

   if (!commands)
      return;
   for (i = 0; i < num_commands; i++)
   {
      fprintf(stdout, "-%s, -%s\t: %s\n", commands[i].abbrev,
              commands[i].command, commands[i].help);
   }
}

static int cmdline_commands_size = sizeof(cmdline_commands) / sizeof(cmdline_commands[0]);

static int encoding_xref_size = sizeof(encoding_xref) / sizeof(encoding_xref[0]);

void raspipreview_display_help()
{
   fprintf(stdout, "\nParameters:\n\n");
   raspicli_display_help(cmdline_commands, cmdline_commands_size);
   //fprintf(stdout, "\nEnd\n\n");
}

int raspicli_get_command_id(const COMMAND_LIST *commands, const int num_commands, const char *arg, int *num_parameters)
{
   int command_id = -1;
   int j;

   if (!commands || !num_parameters || !arg)
      return -1;
      
   for (j = 0; j < num_commands; j++)
   {
      if (!strcmp(arg, commands[j].command) ||
            !strcmp(arg, commands[j].abbrev))
      {
         // match
         command_id = commands[j].id;
         *num_parameters = commands[j].num_parameters;
         break;
      }
   }
   return command_id;
}



static int arducam_parse_cmdline(int argc, char **argv, RASPISTILL_STATE *state){
    int valid =1;
    int i ;
    for (i=1; i< argc && valid; i++){
        int command_id, num_parameters;
        if( !argv[i])
            continue;
        if(argv[i][0] != '-')
        {
            valid =0;
            continue;
        }
        valid = 1;
        
        command_id =raspicli_get_command_id(cmdline_commands, cmdline_commands_size, &argv[i][1], &num_parameters);
        if(command_id == CommandHelp){
             raspipreview_display_help(); 
             valid = 0;
        }
        
        // If we found a command but are missing a parameter, continue (and we will drop out of the loop)
        if (command_id != -1 && num_parameters > 0 && (i + 1 >= argc) )
         continue;
         
        switch (command_id)
        {
        case CommandMode:
            {
              if (sscanf(argv[i + 1], "%d", &state->mode) == 1)
               {
                  //printf("state->mode = %d\r\n",state->mode);
                  i++;
               }
              else
                  valid = 0;
              break;
            } 
            
          case CommandFocus:
            {
               if (sscanf(argv[i + 1], "%d", &(state->focus)) == 1)
               {
                  i++;
               }
               else
                  valid = 0;
               break;
            }
            
            case CommandTimeout:
              {
                 if (sscanf(argv[i + 1], "%d", &state->timeout) == 1)
                 {
                    i++;
                 }
                 else
                    valid = 0;
                 break;
              }
            
          case CommandQuality:
            {
              if (sscanf(argv[i + 1], "%u", &state->quality) == 1)
               {
                  if (state->quality > 100)
                  {
                     fprintf(stderr, "Setting quality = 100\n");
                     state->quality = 100;
                  }
                  i++;
               }
              else
                  valid = 0;
              break;
            }
            
           case CommandRgain:
            {
              if (sscanf(argv[i + 1], "%u", &state->rgain) == 1)
               {
                  if (state->rgain > 0xFFFF)
                  {
                     fprintf(stderr, "Setting max rgain = 0xFFFF\n");
                     state->rgain = 0xFFFF;                    
                  }
                  i++;
                  arducam_manual_set_awb_compensation(state->rgain,state->bgain);
               }
              else
                  valid = 0;
              break;
            }
            
           case CommandBgain:
            {
              if (sscanf(argv[i + 1], "%u", &state->bgain) == 1)
               {
                  if (state->bgain > 0xFFFF)
                  {
                     fprintf(stderr, "Setting max bgain = 0xFFFF\n");
                     state->bgain = 0xFFFF;
                  }
                  arducam_manual_set_awb_compensation(state->rgain,state->bgain);
                  i++;
               }
              else
                  valid = 0;
              break;
            }
            
           case CommandGain:
            {
              if (sscanf(argv[i + 1], "%u", &state->gain) == 1)
               {
                  if (state->gain > 4095)
                  {
                     fprintf(stderr, "Setting max gain = 4095\n");
                     state->gain = 4095;
                  }
                  i++;
               }
              else
                  valid = 0;
              break;
            }
            
           case CommandExposure:
            {
              if (sscanf(argv[i + 1], "%u", &state->exposure) == 1)
               {
                  if (state->exposure > 0xFFFF)
                  {
                     fprintf(stderr, "Setting max exposure = 0xFFFF\n");
                     state->exposure = 0xFFFF;
                  }
                  i++;
               }
              else
                  valid = 0;
              break;
            }
            
           case CommandAutowhitebalance:
            {
              if (sscanf(argv[i + 1], "%d", &state->awb_state) == 1)
               {
                  //printf("state->awb_state = %d\r\n",state->awb_state);
                  i++;
               }
              else
                  valid = 0;
              break;
            } 
            
           case CommandAutoexposure:
            {
              if (sscanf(argv[i + 1], "%d", &state->ae_state) == 1)
               {
                  //printf("state->ae_state = %d\r\n",state->ae_state);
                  i++;
               }
              else
                  valid = 0;
              break;
            } 
            
           case CommandVFlip:
            {
              if (sscanf(argv[i + 1], "%d", &(state->vflip)) == 1)
               {
                  i++;
               }
              else
                  valid = 0;
              break;
            } 
            
           case CommandHFlip:
            {
              if (sscanf(argv[i + 1], "%d", &(state->hflip)) == 1)
               {
                  i++;
               }
              else
                  valid = 0;
              break;
            } 
            
          case CommandEncoding :
          {
              int len = strlen(argv[i + 1]);
              valid = 0;

              if (len)
              {
                  int j;
                  for (j=0; j<encoding_xref_size; j++)
                  {
                  if (strcmp(encoding_xref[j].format, argv[i+1]) == 0)
                  {
                      state->encoding = encoding_xref[j].encoding;
                      valid = 1;
                      i++;
                      break;
                  }
                  }
              }
              break;
          }
          
          case  CommandCapture:
          {
              int len = strlen(argv[i+1]);
              printf("len = %d\r\n",len);
              if(len){
                  state->linkname = malloc(len + 10);
                  if (state->linkname)
                  strncpy(state->linkname, argv[i + 1], len+1);
                  i++;
                   printf("name= %s\r\n",state->linkname);
              }else{
                  valid = 0;
                  break;
              }
          }
        }
    }

   if (!valid)
   {
      fprintf(stderr, "Invalid command line option (%s)\n", argv[i-1]);
      return 1;
   }
   return 0;
}


int readGlobalParameter(CAMERA_INSTANCE camera_instance)
{
   /* Here we READ the current values from the camera! */
   #if(0)
    if (arducam_reset_control(camera_instance, V4L2_CID_FOCUS_ABSOLUTE)) // Set camera control to default value.
    {
        LOG("Failed to set focus, the camera may not support this control.");
        globalParam.focusVal = 0;
    }
    else
    #endif
    
    {
      arducam_get_control(camera_instance, V4L2_CID_FOCUS_ABSOLUTE, &(globalParam.focusVal));
      //LOG("\nFocus was %d.",globalParam.focusVal);
    }
    
    arducam_get_control(camera_instance, V4L2_CID_EXPOSURE, &globalParam.exposureVal);
    //LOG("Exposure time was %d.",globalParam.exposureVal);
    
    arducam_get_gain(camera_instance, &globalParam.redGain, &globalParam.blueGain);
    //LOG("Red  was %d.",globalParam.redGain);
    //LOG("Blue was %d.",globalParam.blueGain);
    
    arducam_get_control(camera_instance, V4L2_CID_GAIN, &globalParam.gain);
    //LOG("Gain (ISO) was %d.",globalParam.gain);
    
    arducam_get_control(camera_instance, V4L2_CID_HFLIP, &globalParam.hflip);
    //LOG("HFlip was %d.",globalParam.hflip);
    
    arducam_get_control(camera_instance, V4L2_CID_VFLIP, &globalParam.vflip);
    //LOG("VFlip was %d.",globalParam.vflip);
   
    LOG("\nFocus is now %d.",globalParam.focusVal);
    LOG("Exposure time is now %d.",globalParam.exposureVal);
    LOG("Red  is now %d.",globalParam.redGain);
    LOG("Blue is now %d.",globalParam.blueGain);
    LOG("Gain (ISO) is now %d.",globalParam.gain);
    LOG("HFlip is now %d.",globalParam.hflip);
    LOG("VFlip is now %d.",globalParam.vflip);
    LOG("Auto white balance is now %d.",globalParam.awb);
    LOG("Auto exposure is now %d.\n",globalParam.autoex);
}


int setGlobalParameter(CAMERA_INSTANCE camera_instance)
{    
    arducam_set_control(camera_instance, V4L2_CID_GAIN,           globalParam.gain);
    arducam_set_control(camera_instance, V4L2_CID_FOCUS_ABSOLUTE, globalParam.focusVal);
    arducam_set_control(camera_instance, V4L2_CID_HFLIP,          globalParam.hflip);
    arducam_set_control(camera_instance, V4L2_CID_VFLIP,          globalParam.vflip);

   /* Here we WRITE the current values to the camera! */
    arducam_software_auto_white_balance(camera_instance, globalParam.awb);
    if(!globalParam.awb)
    {
      arducam_manual_set_awb_compensation(globalParam.redGain,globalParam.blueGain);
    }
    
    arducam_software_auto_exposure(camera_instance, globalParam.autoex);
    if(!globalParam.autoex)
    {
      arducam_set_control(camera_instance, V4L2_CID_EXPOSURE, globalParam.exposureVal);
    }

    if((globalParam.autoex || globalParam.awb) && (globalParam.timeout != 0))
    {
      usleep(1000 * globalParam.timeout);
    }

    
    LOG("\nFocus set to %d.",globalParam.focusVal);
    LOG("Exposure time set to %d.",globalParam.exposureVal);
    LOG("Red  set to %d.",globalParam.redGain);
    LOG("Blue set to %d.",globalParam.blueGain);
    LOG("Gain (ISO) set to %d.",globalParam.gain);
    LOG("HFlip set to %d.",globalParam.hflip);
    LOG("VFlip set to %d.",globalParam.vflip);
    LOG("Auto white balance set to %d.",globalParam.awb);
    LOG("Auto exposure set to %d.\n",globalParam.autoex);
}


void save_image(CAMERA_INSTANCE camera_instance, const char *name) {
    // The actual width and height of the IMAGE_ENCODING_RAW_BAYER format and the IMAGE_ENCODING_I420 format are aligned, 
    // width 32 bytes aligned, and height 16 byte aligned.

    usleep(1000*10);
    
    BUFFER *buffer = arducam_capture(camera_instance, &fmt, 12000);
    if (!buffer) {
        LOG("capture timeout.");
        return;
    }
    
    FILE *file = fopen(name, "wb");
    if(file == NULL)
    {
      LOG("Can't open file %s for writing",name);
    }
    else
    {
      fwrite(buffer->data, buffer->length, 1, file);
      fclose(file);
      LOG("Saved picture to %s with quality %d\n", name, fmt.quality);
    }
    arducam_release_buffer(buffer);
}


static void default_status(RASPISTILL_STATE *state)
{
    state->mode = 4;
    state->ae_state =1;
    state->rgain =100;
    state->bgain =100;
    state->awb_state = 1;
    state->quality = 100;
    state->focus = 0;
    state->gain = 100;
    state->exposure = 270;
    state->timeout = 5000;
    state->hflip = 0;
    state->vflip = 0;
    
    state->linkname = malloc(30);
    if (state->linkname)
      strncpy(state->linkname, "/run/mqtt/mode4.jpg",21);
      
    state->encoding = IMAGE_ENCODING_JPEG;

    globalParam.frameCnt = 0;
    globalParam.key = 0;
}




int main(int argc, char **argv) {
    CAMERA_INSTANCE camera_instance;
    RASPISTILL_STATE state;
    int width = 0, height = 0;
    char file_name[100];

    default_status(&state);

    if (arducam_parse_cmdline(argc, argv, &state))
    {
     return 0;
    } 

    LOG("Open camera...");
    int res = arducam_init_camera(&camera_instance); 
    if (res) {
        LOG("init camera status = %d", res);
        return -1;
    }
           
    LOG("Setting the mode...");
    // res = arducam_set_resolution(camera_instance, &width, &height);
    //printf("choose the mode %d\r\n", mode );
    res= arducam_set_mode(camera_instance, state.mode);
    if (res) {
        LOG("set resolution = %d", res);
        return -1;
    } else {
        LOG("Current mode is %d", state.mode);
        //LOG("Notice:You can use the list_format sample program to see the resolution and control supported by the camera.");
    }
    
    readGlobalParameter(camera_instance);

    /* transfer our parameters to global parameter block */
    globalParam.focusVal = state.focus;
    globalParam.redGain = state.rgain;
    globalParam.blueGain = state.bgain;
    globalParam.autoex = state.ae_state;
    globalParam.awb = state.awb_state;
    globalParam.gain = state.gain;
    globalParam.exposureVal = state.exposure;
    globalParam.timeout = state.timeout;
    globalParam.hflip = state.hflip;
    globalParam.vflip = state.vflip;
    
    fmt.quality = state.quality; /* the jpeg quality to use */

    setGlobalParameter(camera_instance);

    readGlobalParameter(camera_instance);

    if(fmt.encoding == IMAGE_ENCODING_JPEG){
        sprintf(file_name, "/run/mqtt/mode%d.jpg", state.mode);
    }
    if(fmt.encoding == IMAGE_ENCODING_BMP){
        sprintf(file_name, "/run/mqtt/mode%d.bmp", state.mode);
    }
    if(fmt.encoding == IMAGE_ENCODING_PNG){
        sprintf(file_name, "/run/mqtt/mode%d.png", state.mode);
    } 
    LOG("Capture image %s...", file_name);
    save_image(camera_instance, file_name);

    LOG("Close camera...");
    res = arducam_close_camera(camera_instance);
    if (res) {
        LOG("close camera status = %d", res);
    }
    return 0;
}
