#include "arducam_mipicamera.h"
#include <linux/v4l2-controls.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)
#define SET_CONTROL 0

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
   CommandHelp,
};

static COMMAND_LIST cmdline_commands[] =
{
   { CommandTimeout, "-timeout",    "t",  "Time (in ms) before takes picture and shuts down (if not specified, loop)", 1 },
   { CommandQuality, "-quality",    "q",  "Set jpeg quality <0 to 100>", 1 },
   { CommandMode, "-mode",    "m",    "Set sensor mode", 1},
   { CommandAutowhitebalance, "-autowhitebalance",    "awb",    "Enable or disable awb", 1 },
   { CommandAutoexposure, "-autoexposure",    "ae",    "Enable or disable ae", 1 },
   { CommandRgain, "-awbrgain",    "rgain",  "Set R channel gian vaue <0 to 65535>", 1 },
   { CommandBgain, "-awbbgain",    "bgain",  "Set B channel gian vaue <0 to 65535>", 1 },
   { CommandHelp, "-help",    "?",    "This help information", 0},
};

typedef struct
{
   int timeout;                        // Time taken before frame is grabbed and app then shuts down. Units are milliseconds
   int quality;                        // JPEG quality setting (1-100)
   int rgain;
   int bgain;
   int mode;                         // sensor mode
   int awb_state;                      // auto white balance state
   int ae_state;                       // auto exposure state
} RASPISTILL_STATE;

static int arducam_parse_cmdline(int argc, char **argv, RASPISTILL_STATE *state);
int raspicli_get_command_id(const COMMAND_LIST *commands, const int num_commands, const char *arg, int *num_parameters);
static int cmdline_commands_size = sizeof(cmdline_commands) / sizeof(cmdline_commands[0]);
static void default_status(RASPISTILL_STATE *state);
void raspicli_display_help(const COMMAND_LIST *commands, const int num_commands);
void raspipreview_display_help();
void printCurrentMode(CAMERA_INSTANCE camera_instance);

time_t begin = 0;
unsigned int frame_count = 0;
int raw_callback(BUFFER *buffer) {
     frame_count++;
         if(time(NULL) - begin >= 1){
             printf("\rCurrent framerate: ");
             printf("%d fps",frame_count);   
             fflush(stdout);  
            frame_count = 0;
            begin = time(NULL);
         }
    return 0;
}
int main(int argc, char **argv) {
  CAMERA_INSTANCE camera_instance;
  RASPISTILL_STATE state;
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
    res =  arducam_set_mode(camera_instance, state.mode);
       
    if (res) {
        LOG("set resolution status = %d", res);
        return -1;
    } 
    LOG("Start preview...");
    PREVIEW_PARAMS preview_params = {
        .fullscreen = 0,             // 0 is use previewRect, non-zero to use full screen
        .opacity = 255,              // Opacity of window - 0 = transparent, 255 = opaque
        .window = {0, 0, 1280, 720}, // Destination rectangle for the preview window.
    };
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
    if(state.ae_state){
        if (arducam_software_auto_exposure(camera_instance, 1)) {
        LOG("Mono camera does not support automatic white balance.");
        }
    }
    if(state.awb_state){
        if (arducam_software_auto_white_balance(camera_instance, 1)) {
        LOG("Mono camera does not support automatic white balance.");
        }
    }
    
    if (arducam_reset_control(camera_instance, V4L2_CID_FOCUS_ABSOLUTE)) {
               LOG("Failed to set focus, the camera may not support this control.");
           }
    if(state.timeout == 0){
        while(1){
            usleep(1000);
            }
        }else{
            usleep(1000 * state.timeout);
        }
    
#if SET_CONTROL
    LOG("Reset the focus...");
    if (arducam_reset_control(camera_instance, V4L2_CID_FOCUS_ABSOLUTE)) {
        LOG("Failed to set focus, the camera may not support this control.");
    }
    usleep(1000 * 1000 * 2);
    LOG("Setting the exposure...");
    if (arducam_set_control(camera_instance, V4L2_CID_EXPOSURE, 0x1F00)) {
        LOG("Failed to set exposure, the camera may not support this control.");
        LOG("Notice:You can use the list_format sample program to see the resolution and control supported by the camera.");
    }
    usleep(1000 * 1000 * 2);
    LOG("Setting the exposure...");
    if (arducam_set_control(camera_instance, V4L2_CID_EXPOSURE, 0x1F00)) {
        LOG("Failed to set exposure, the camera may not support this control.");
    }
    usleep(1000 * 1000 * 2);
    LOG("Setting the hfilp...");
    if (arducam_set_control(camera_instance, V4L2_CID_HFLIP, 1)) {
        LOG("Failed to set hflip, the camera may not support this control.");
    }
    usleep(1000 * 1000 * 2);
    LOG("Enable Auto Exposure...");
    //arducam_software_auto_exposure(camera_instance, 1);

    usleep(1000 * 1000 * 2);
    LOG("Enable Auto White Balance...");
    if (arducam_software_auto_white_balance(camera_instance, 1)) {
        LOG("Mono camera does not support automatic white balance.");
    }
#endif
   
    LOG("Stop preview...");
    res = arducam_stop_preview(camera_instance);
    if (res) {
        LOG("stop preview status = %d", res);
    }

    LOG("Close camera...");
    res = arducam_close_camera(camera_instance);
    if (res) {
        LOG("close camera status = %d", res);
    }
    return 0;
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
        }
    }

   if (!valid)
   {
      //fprintf(stderr, "Invalid command line option (%s)\n", argv[i-1]);
      return 1;
   }
   return 0;
}


static void default_status(RASPISTILL_STATE *state)
{
    state->mode = 0;
    state->ae_state =1;
    state->rgain =100;
    state->bgain =100;
    state->awb_state = 1;
    state->quality = 50;
    state->timeout = 5000;
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


