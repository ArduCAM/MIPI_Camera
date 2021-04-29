#include "arducam_mipicamera.h"
#include <linux/v4l2-controls.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#define LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)
#define SET_CONTROL 0
#ifndef vcos_assert
#define vcos_assert(cond) \
   ( (cond) ? (void)0 : (VCOS_ASSERT_BKPT, VCOS_ASSERT_MSG("%s", #cond)) )
#endif
#define UP    1
#define DOWN  2
#define LEFT  3
#define RIGHT 4
#define W     5
#define S     6
#define A     7
#define D     8
#define T     9
#define C     10

#define exposureStep  50
#define focusStep     5
#define rgainStep     10
#define bgainStep     10

int is_stop = 0;
typedef struct
{
   int id;
   char *command;
   char *abbrev;
   char *help;
   int num_parameters;
} COMMAND_LIST;

enum
{
   CommandTimeout,
   CommandQuality,
   CommandMode,
   CommandAutowhitebalance,
   CommandAutoexposure,
   CommandRgain,
   CommandBgain,
   CommandCapture,
   CommandRaw,
   CommandEncoding,
   CommandEnableCS,
   CommandHelp,
};

static COMMAND_LIST cmdline_commands[] =
{
   { CommandTimeout, "-timeout",    "t",  "Time (in ms) before takes picture and shuts down (if not specified, loop)", 1 },
   { CommandQuality, "-quality",    "q",  "Set jpeg quality <0 to 100>", 1 },
   { CommandMode, "-mode",    "m",    "Set sensor mode", 1},
   { CommandAutowhitebalance, "-autowhitebalance",    "awb",    "Enable or disable awb", 1 },
   { CommandAutoexposure, "-autoexposure",    "ae",    "Enable or disable ae", 1 },
   { CommandRgain, "-awbrgain",    "rgain",  "Set R channel gain value <0 to 65535>", 1 },
   { CommandBgain, "-awbbgain",    "bgain",  "Set B channel gain value <0 to 65535>", 1 },
   { CommandCapture, "-capture",    "o",    "used to get one frame", 0},
   { CommandRaw,     "-raw",        "r",  "Add raw bayer data to jpeg metadata", 0 },
   { CommandEncoding,"-encoding",   "e",  "Encoding to use for output file (jpg, bmp, gif, png)", 1},
   {CommandEnableCS, "-cs",    "cs",    "Set camera cs", 1},
   { CommandHelp, "-help",    "?",    "This help information", 0},
};
typedef struct
{
   int timeout;                        // Time taken before frame is grabbed and app then shuts down. Units are milliseconds
   int quality;                        // JPEG quality setting (1-100)
   uint32_t encoding;                  // Encoding to use for the output file.
   int rgain;                          // red gain compensation
   int bgain;                          // blue gain compensation
   int mode;                           // sensor mode
   int cs;                              // sensor cs
   int awb_state;                      // auto white balance state
   int ae_state;                       // auto exposure state
   int glCapture;                      // Save the GL frame-buffer instead of camera output
   char *linkname;                     // filename of output file
} RASPISTILL_STATE;

typedef struct{
int frameCnt;
int exposureVal;
int focusVal;
int  redGain ;
int  blueGain;
int  key;
int trigger;    // external trigger
}GLOBAL_VAL;
typedef struct {
    CAMERA_INSTANCE camera_instance;
    RASPISTILL_STATE state;
}PROCESS_STRUCT;
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
static int encoding_xref_size = sizeof(encoding_xref) / sizeof(encoding_xref[0]);

static int arducam_parse_cmdline(int argc, char **argv, RASPISTILL_STATE *state);
int raspicli_get_command_id(const COMMAND_LIST *commands, const int num_commands, const char *arg, int *num_parameters);
static int cmdline_commands_size = sizeof(cmdline_commands) / sizeof(cmdline_commands[0]);
static void default_status(RASPISTILL_STATE *state);
void raspicli_display_help(const COMMAND_LIST *commands, const int num_commands);
void raspipreview_display_help(); 
void printCurrentMode(CAMERA_INSTANCE camera_instance);
void save_image(CAMERA_INSTANCE camera_instance, const char *name, uint32_t encoding, int quality);
time_t begin = 0;
GLOBAL_VAL globalParam; 
pthread_t processCmd_pt;
_Bool isrunning  = 1;

char* itoa(int num,char* str,int radix)
{
    char index[]="0123456789ABCDEF";
    unsigned unum;
    int i=0,j,k;
    if(radix==10&&num<0)
    {
        unum=(unsigned)-num;
        str[i++]='-';
    }
    else unum=(unsigned)num;
    do{
        str[i++]=index[unum%(unsigned)radix];
        unum/=radix;
       }while(unum);
    str[i]='\0';
    if(str[0]=='-')
        k=1;
    else
        k=0;
     
    for(j=k;j<=(i-1)/2;j++)
    {       char temp;
        temp=str[j];
        str[j]=str[i-1+k-j];
        str[i-1+k-j]=temp;
    }
    return str;
}

int resetGlobalParameter(CAMERA_INSTANCE camera_instance, GLOBAL_VAL* globalParam){
    if (arducam_reset_control(camera_instance, V4L2_CID_FOCUS_ABSOLUTE)) {
        LOG("Failed to set focus, the camera may not support this control.");
    }
    arducam_get_control(camera_instance, V4L2_CID_EXPOSURE, &globalParam->exposureVal);
    arducam_get_control(camera_instance, V4L2_CID_FOCUS_ABSOLUTE, &globalParam->focusVal);
    arducam_get_gain(camera_instance, &globalParam ->redGain, &globalParam ->blueGain);
    globalParam -> frameCnt = 0;
    globalParam -> key = 0;
    
}

void stop(){
  is_stop = 1;
usleep(1000*100);
arducam_mipi_camera_reset();
}

static struct termios initial_settings, new_settings;
static int peek_character = -1;
void init_keyboard(void);
void close_keyboard(void);
int kbhit(void);
int readch(void);

void init_keyboard()
{
    tcgetattr(0,&initial_settings);
    new_settings = initial_settings;
    new_settings.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &new_settings);
 
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}
 
void close_keyboard()
{
    tcsetattr(0, TCSANOW, &initial_settings);
}
 
int kbhit()
{
    unsigned char ch;
    int nread;
 
    if (peek_character != -1) return 1;
    new_settings.c_cc[VMIN]=0;
    tcsetattr(0, TCSANOW, &new_settings);
    nread = read(0,&ch,1);	
    tcsetattr(0, TCSANOW, &new_settings);
    if(nread == 1)
    {
        peek_character = ch;
        return 1;
    }
    return 0;
}
 
int readch()
{
    char ch;
 
    if(peek_character != -1)
    {
        ch = peek_character;
        peek_character = -1;
        return ch;
    }
    read(0,&ch,1);
    return ch;
}





int get_key_board_from_termios()
{
     
     kbhit();
    return readch();;
}
 
void processKeyboardEvent(CAMERA_INSTANCE camera_instance,GLOBAL_VAL* globalParam){
    int keyVal = 0;
    init_keyboard();
    while(1){
         if(is_stop){
         close_keyboard();
           break;
         }
         keyVal= get_key_board_from_termios();
         if(keyVal == 27){  
             keyVal= get_key_board_from_termios();
             if(keyVal == 91){
                keyVal= get_key_board_from_termios();
                if(keyVal == 65){
                   globalParam->key = UP;// up 
                   globalParam->exposureVal += exposureStep;
                   if(globalParam->exposureVal > 0xFFFF){
                      globalParam->exposureVal = 0xFFFF;
                   }
                }
                if(keyVal == 66 ){
                     globalParam->key = DOWN;// down 
                     globalParam->exposureVal -= exposureStep;
                     if(globalParam->exposureVal< 0){
                        globalParam->exposureVal = 0;
                     }
                     
                }
                if(keyVal == 68 ){
                     globalParam->key = LEFT;// left
                     globalParam->focusVal += focusStep;
                     if( globalParam->focusVal > 1024){
                         globalParam->focusVal = 1024;
                     }
                }
                if(keyVal == 67 ){
                     globalParam->key = RIGHT;// right
                     globalParam->focusVal -= focusStep;
                     if(globalParam->focusVal <0){
                         globalParam->focusVal = 0;
                     }
                }
             }
         }
         if(keyVal == 111){
            static int k = 0;
            char str[8];
            k++;
            itoa(k, str, 10);
            strcat(str, ".jpg");
             save_image(camera_instance, str, \
                  IMAGE_ENCODING_JPEG, 80);
            printf("Image save OK\r\n");
         }
          if(keyVal == 119){
             globalParam->key = W;// W
             globalParam->redGain += rgainStep; 
          }
          if(keyVal == 115){
             globalParam->key = S;// S
             globalParam->redGain -= rgainStep; 
          }
          if(keyVal == 97){
             globalParam->key = A;// A
             globalParam->blueGain += bgainStep; 
          }
            if(keyVal == 100){
             globalParam->key = D;// D
             globalParam->blueGain -= bgainStep; 
          }
            if(keyVal == 116){  //T
             globalParam->key = T;// T
             globalParam->trigger = 1; 
             //Enter external trigger mode 
             arducam_set_control(camera_instance, V4L2_CID_ARDUCAM_EXT_TRI,globalParam->trigger);
          }
           if(keyVal == 99){  //C
             globalParam->key = C;// C
             globalParam->trigger = 0; 
             //Exit external trigger mode 
             arducam_set_control(camera_instance, V4L2_CID_ARDUCAM_EXT_TRI,globalParam->trigger);
          }
         //if(!isrunning){
         //   // LOG("Please click 'Ctrl'+'C' to exit!");
         //    break;
         //}
         switch (globalParam->key){
            case UP:   
            case DOWN:
            if (arducam_set_control(camera_instance, V4L2_CID_EXPOSURE, globalParam->exposureVal)) {
            LOG("Failed to set exposure, the camera may not support this control.");
            }
            break;
            case LEFT:
            case RIGHT:
            if (arducam_set_control(camera_instance, V4L2_CID_FOCUS_ABSOLUTE,globalParam->focusVal)) {
             LOG("Failed to set focus, the camera may not support this control.");
            }
	    break;
            case W:
            case S:
            case A:
            case D:
            arducam_manual_set_awb_compensation(globalParam->redGain,globalParam->blueGain);
            break;
         }
	 globalParam->key = 0;
         //LOG("Keyval:%d",keyVal);
    }
   
   // return 0;
}
int raw_callback(BUFFER *buffer) {
       globalParam.frameCnt++;
         if(time(NULL) - begin >= 1){
             printf("\r[Framerate]: %02d pfs, [Exposure]: %04d, [Focus]: %04d,[Rgain]: %04d, [Bgain]: %04d", 
                    globalParam.frameCnt,globalParam.exposureVal,globalParam.focusVal,\
                    globalParam.redGain, globalParam.blueGain);
             fflush(stdout); 
            globalParam.frameCnt = 0;
            begin = time(NULL);
         }
    return 0;
}
int printSupportFormat(CAMERA_INSTANCE camera_instance){
    struct format support_fmt;
       int index = 0;
       char fourcc[5];
       fourcc[4] = '\0';
       while (!arducam_get_support_formats(camera_instance, &support_fmt, index++)) {
           strncpy(fourcc, (char *)&support_fmt.pixelformat, 4);
           LOG("mode: %d, width: %d, height: %d, pixelformat: %s, desc: %s", 
               support_fmt.mode, support_fmt.width, support_fmt.height, fourcc, 
               support_fmt.description);
       }
       index = 0;
       struct camera_ctrl support_cam_ctrl;
       while (!arducam_get_support_controls(camera_instance, &support_cam_ctrl, index++)) {
           int value = 0;
           if (arducam_get_control(camera_instance, support_cam_ctrl.id, &value)) {
               LOG("Get ctrl %s fail.", support_cam_ctrl.desc);
           }
           LOG("index: %d, CID: 0x%08X, desc: %s, min: %d, max: %d, default: %d, current: %d",
               index - 1, support_cam_ctrl.id, support_cam_ctrl.desc, support_cam_ctrl.min_value,
               support_cam_ctrl.max_value, support_cam_ctrl.default_value, value);
       }
}

void prcessCmd(PROCESS_STRUCT *processData){
    int res;
    if(processData->state.ae_state){
        if (arducam_software_auto_exposure(processData->camera_instance, 1)) {
        LOG("Mono camera does not support automatic white balance.");
        }
    }
    if(processData->state.awb_state){
        if (arducam_software_auto_white_balance(processData->camera_instance, 1)) {
        LOG("Mono camera does not support automatic white balance.");
        }
    }
    if(processData->state.timeout == 0){
        while(1){
            usleep(1000);
            }
        }else{
            usleep(1000 * processData->state.timeout);
        }
    if(processData->state.linkname){
        save_image(processData->camera_instance, processData->state.linkname, \
                   processData->state.encoding, processData->state.quality);
        free (processData->state.linkname);
    }
     isrunning = 0;
     stop();
    LOG("Stop preview...");
    res = arducam_stop_preview(processData->camera_instance);
    if (res) {
        LOG("stop preview status = %d", res);
    }
 
    LOG("Close camera...");
    res = arducam_close_camera(processData->camera_instance);
    if (res) {
        LOG("close camera status = %d", res);
    }
   
    pthread_join(processCmd_pt,NULL);  // wait thread finish
}

int main(int argc, char **argv) {
  CAMERA_INSTANCE camera_instance; 
  CAMERA_INSTANCE camera_instance2;
  RASPISTILL_STATE state;
  PROCESS_STRUCT  processData;
  int res;
  PREVIEW_PARAMS preview_params;
  signal(SIGINT, stop);
  default_status(&state);
    if (arducam_parse_cmdline(argc, argv, &state))
    {
        return 0;
    } 
    LOG("Open camera...");
    if(state.cs == 0){
        res = arducam_init_camera(&camera_instance);
        if (res) {
            LOG("init camera status = %d", res);
            return -1;
        }
        PREVIEW_PARAMS preview_params1 = {
            .fullscreen = 0,             // 0 is use previewRect, non-zero to use full screen
            .opacity = 255,              // Opacity of window - 0 = transparent, 255 = opaque
            .window = {0, 0, 1920, 1080}, // Destination rectangle for the preview window.
        };
    preview_params = preview_params1;
    }else if(state.cs == 1){
        struct camera_interface cam_interface = {
        .i2c_bus = 0,           // /dev/i2c-0  or /dev/i2c-1   
        .camera_num = 1,        // mipi interface num
        .sda_pins = {44, 0},    // enable sda_pins[camera_num], disable sda_pins[camera_num ? 0 : 1]
        .scl_pins = {45, 1},    // enable scl_pins[camera_num], disable scl_pins[camera_num ? 0 : 1]
        .shutdown_pins ={133, 133},
        };
        res = arducam_init_camera2(&camera_instance,cam_interface);
        if (res) {
            LOG("init camera status = %d", res);
            return -1;
        }
        PREVIEW_PARAMS preview_params2 = {
            .fullscreen = 0,             // 0 is use previewRect, non-zero to use full screen
            .opacity = 255,              // Opacity of window - 0 = transparent, 255 = opaque
            .window = {640, 480, 640, 480}, // Destination rectangle for the preview window.
        };
        preview_params = preview_params2;
    }
    char path = NULL;//"./lens_shading_table/imx230/2672x2004.h";
    arducam_set_lens_table(camera_instance,path );
    printSupportFormat(camera_instance);
    res = arducam_set_mode(camera_instance, state.mode);
    if (res) {
        LOG("set resolution status = %d", res);
        return -1;
    } 
    resetGlobalParameter(camera_instance, &globalParam);
    if (arducam_set_control(camera_instance, V4L2_CID_FOCUS_ABSOLUTE,globalParam.focusVal)) {
        LOG("Failed to set focus, the camera may not support this control.");
    }
    if (arducam_set_control(camera_instance, V4L2_CID_EXPOSURE,globalParam.exposureVal)) {
        LOG("Failed to set exposure, the camera may not support this control.");
    }
    arducam_manual_set_awb_compensation(globalParam.redGain,globalParam.blueGain);      
    LOG("Start preview...");
    
    res = arducam_start_preview(camera_instance, &preview_params);
    if (res) {
        LOG("start preview status = %d", res);
        return -1;
    }
    res = arducam_set_raw_callback(camera_instance, raw_callback, NULL);
        if (res) {
            LOG("Failed to start raw data callback.");
            return -1;
        }
 printCurrentMode(camera_instance);
 processData.camera_instance = camera_instance;
 processData.state = state;
  int ret = pthread_create(&processCmd_pt, NULL, prcessCmd,&processData);
  if(ret){
    LOG("pthread create failed");
    return 1;
  }
    processKeyboardEvent(camera_instance,&globalParam);

    //usleep(2*1000*1000);
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
         command_id = commands[j].id;
         *num_parameters = commands[j].num_parameters;
         break;
      }
   }
   return command_id;
}

static int arducam_parse_cmdline(int argc, char **argv,RASPISTILL_STATE *state){
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
             raspipreview_display_help(); valid = 0;
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
          case CommandEnableCS:
              {
                if (sscanf(argv[i + 1], "%d", &state->cs) == 1)
                 {
                   // printf("state->cs = %d\r\n",state->cs);
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
                       fprintf(stderr, "Setting max quality = 100\n");
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
             case CommandAutowhitebalance:
              {
                if (sscanf(argv[i + 1], "%d", &state->awb_state) == 1)
                 {
                 //   printf("state->awb_state = %d\r\n",state->awb_state);
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
                  //  printf("state->ae_state = %d\r\n",state->ae_state);
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
               // printf("len = %d\r\n",len);
                if(len){
                    state->linkname = malloc(len + 10);
                    if (state->linkname)
                    strncpy(state->linkname, argv[i + 1], len+1);
                    i++;
                   //  printf("name= %s\r\n",state->linkname);
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
static void default_status(RASPISTILL_STATE *state)
{
    state->mode = 0;
    state->ae_state =0;
    state->rgain =100;
    state->bgain =100;
    state->awb_state = 1;
    state->quality = 50;
    state->timeout = 5000;
    state->cs = 0;
    state->linkname = NULL;
    state->encoding = IMAGE_ENCODING_JPEG;
}

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
void raspipreview_display_help()
{
   fprintf(stdout, "\nPreview parameter commands:\n\n");
   raspicli_display_help(cmdline_commands, cmdline_commands_size);
   fprintf(stdout, "\nEnd\n\n");
}

void printCurrentMode(CAMERA_INSTANCE camera_instance){
    struct format currentFormat;
    char fourcc[5];
    fourcc[4] = '\0';
    arducam_get_format(camera_instance, &currentFormat);
    strncpy(fourcc, (char *)&currentFormat.pixelformat, 4);
     printf("%c[32;40m",0x1b);  
     printf("Current mode: %d, width: %d, height: %d, pixelformat: %s, desc: %s\r\n", 
            currentFormat.mode, currentFormat.width, currentFormat.height, fourcc, 
            currentFormat.description);
}
void save_image(CAMERA_INSTANCE camera_instance, const char *name, uint32_t encoding, int quality) {
    IMAGE_FORMAT fmt = {encoding, quality};
    // The actual width and height of the IMAGE_ENCODING_RAW_BAYER format and the IMAGE_ENCODING_I420 format are aligned, 
    // width 32 bytes aligned, and height 16 byte aligned.
    BUFFER *buffer = arducam_capture(camera_instance, &fmt, 352000);
    if (!buffer) {
        LOG("capture timeout.");
        return;
    }
    FILE *file = fopen(name, "wb");
    fwrite(buffer->data, buffer->length, 1, file);
    fclose(file);
    arducam_release_buffer(buffer);
}



