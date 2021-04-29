#include <SFML/Audio.h>
#include <SFML/Graphics.h>
#include <SFML/OpenGL.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

volatile char runflag = 1;
void sig_int(int sig){
  runflag = 0;
}

void show_devices(){
  size_t devnum;
  const char **devs = sfSoundRecorder_getAvailableDevices(&devnum);
  for(size_t i=0; i<devnum; i++){
    printf("%li\t[%s]\n", i, devs[i]);
  }
}

sfVideoMode wndsize = {.width=200, .height=200, 32};
long int range_min = -33000;
long int range_max = 33000;
char use_gl_flag = 0;
char show_window_flag = 0;
char gl_line_flag = 0;
char data_count_flag = 0;
char data_count_capt = 0;
char pause_flag = 0;
sfRenderWindow *wnd = NULL;

int16_t *data_buf = NULL;
uint64_t data_size = 10000;
uint64_t data_pos = 0;

void data_update(const int16_t data[], uint64_t size){
  if(pause_flag)return;
  if(!use_gl_flag){
    for(uint64_t i=0; i<size; i++){
      printf("%"PRIi16"\n", data[i]);
    }
    return;
  }
  uint64_t min = data_size - data_pos;
  if(min > size)min=size;
  memcpy( &data_buf[data_pos], data, min*sizeof(int16_t) );
  data_pos += min;
  size -= min;
  if(size == 0)return;
  if(size > data_size)size=data_size;
  
  memcpy( data_buf, &data[min], size*sizeof(int16_t) );
  data_pos = size;
}
void gl_draw(){
  glClear(GL_COLOR_BUFFER_BIT);
  glColor3f(0,1,0);
  glBegin(GL_LINES);
    glVertex2f(0,0);
    glVertex2f(data_size, 0);
  glEnd();
  glColor3f(0,0,0);
  if(gl_line_flag)glBegin(GL_LINE_STRIP); else glBegin(GL_POINTS);
    uint64_t x = 0;
    for(uint64_t i=data_pos; i<data_size; i++)
      glVertex2f(x++, data_buf[i]);
    for(uint64_t i=0; i<data_pos; i++)
      glVertex2f(x++, data_buf[i]);
  glEnd();
}

void wnd_init(){
  if(!show_window_flag)return;
  wnd = sfRenderWindow_create( wndsize, "Microphone", sfResize | sfClose, NULL);
  if(!use_gl_flag)sfRenderWindow_setPosition(wnd, (sfVector2i){0,0});
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0,data_size, range_min,range_max);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glClearColor(1,1,1,0);
  glEnable(GL_BLEND); //разрешение полупрозрачности
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

char wnd_update(){
  if(!show_window_flag)return 1;
  sfEvent event;
  while(sfRenderWindow_pollEvent(wnd, &event)){
    if(event.type == sfEvtClosed){
      sfRenderWindow_close(wnd);
      return 0;
    }else if(event.type == sfEvtKeyPressed){
      if(((sfKeyEvent*)&event)->code == sfKeyEscape){
        sfRenderWindow_close(wnd);
        return 0;
      }else if(((sfKeyEvent*)&event)->code == sfKeySpace){
        pause_flag = !pause_flag;
      }else if(((sfKeyEvent*)&event)->code == sfKeyM){
        gl_line_flag = !gl_line_flag;
      }else if(((sfKeyEvent*)&event)->code == sfKeyC){ //^C
        if(((sfKeyEvent*)&event)->control){
          sfRenderWindow_close(wnd);
          return 0;
        }
      }
    }else if(event.type == sfEvtResized){
      wndsize.width = ((sfSizeEvent*)&event)->width;
      wndsize.height = ((sfSizeEvent*)&event)->height;
      if(use_gl_flag){
        glViewport(0,0, wndsize.width, wndsize.height);
      }
    }
  }
  if(use_gl_flag){
    gl_draw();
  }
  sfRenderWindow_display(wnd);
  return 1;
}

unsigned long get_time_ms(){
  struct timeval tv;
  unsigned long time_ms;
  gettimeofday(&tv,NULL);
  time_ms = tv.tv_usec/1000 + tv.tv_sec*1000;
  return time_ms;
}

void help(char *name){
  printf("Usage: %s [flags]\n"
    "\t-r samplerate    \tset sample rate (samples per second)\n"
    "\t-t sampletime    \tset sampling time (seconds, may be fractional)\n"
    "\t-T               \tshow real frames per sample\n"
    "\t-C               \tshow real frames in window caption\n"
    "\t-v               \tdisplay all avaible devices\n"
    "\t-d name          \tset device by name\n"
    "\t-s               \tshow window\n"
    "\t-S               \tshow window and data in it\n"
    "\t-W width         \tset window width\n"
    "\t-H hwight        \tset window height\n"
    "\t-m [points/lines]\tmarker type: points or lines\n"
    "\t-p numpoints     \tset point number at graph\n"
    "\t-h               \tshow this help\n"
    "Interactive control:\n"
    "\t^C     \texit\n"
    "\t[ESC]  \t(in window) - exit\n"
    "\t[SPACE]\t(in window) - pause\n"
    "\tm      \t(in window) - switch marker type: lines or points\n"
  ,name);
}
unsigned int samplerate = 22000;
unsigned int sampletime_ms = 100;
char *devname = NULL;
#define StrEq(str, eq) (strncmp(str, eq, sizeof(eq))==0)
int main(int argc, char **argv){
  for(int i=1; i<argc; i++){
    if(StrEq(argv[i], "-t")){
      double time_s;
      sscanf(argv[i+1], "%lg", &time_s);
      sampletime_ms = time_s*1000;
      i++;
    }else if(StrEq(argv[i], "-r")){
      sscanf(argv[i+1], "%u", &samplerate);
      i++;
    }else if(StrEq(argv[i], "-T")){
      data_count_flag = 1;
    }else if(StrEq(argv[i], "-C")){
      data_count_capt = 1;
    }else if(StrEq(argv[i], "-v")){
      show_devices();
      return 0;
    }else if(StrEq(argv[i], "-d")){
      devname = argv[i+1];
      i++;
    }else if(StrEq(argv[i], "-s")){
      show_window_flag = 1; use_gl_flag = 0;
    }else if(StrEq(argv[i], "-S")){
      show_window_flag = 1; use_gl_flag = 1;
    }else if(StrEq(argv[i], "-W")){
      sscanf(argv[i+1], "%u", &wndsize.width);
      i++;
    }else if(StrEq(argv[i], "-H")){
      sscanf(argv[i+1], "%u", &wndsize.height);
      i++;
    }else if(StrEq(argv[i], "-m")){
      if(StrEq(argv[i+1], "lines"))gl_line_flag = 1; else gl_line_flag = 0;
      i++;
    }else if(StrEq(argv[i], "-p")){
      sscanf(argv[i+1], "%"SCNu64, &data_size);
    }else if(StrEq(argv[i], "-h")){
      help(argv[0]);
      return 0;
    }
  }
  
  data_buf = malloc(sizeof(int16_t)*data_size);
  
  signal(SIGINT, sig_int);
  wnd_init();
  
  sfSoundBufferRecorder *mic = sfSoundBufferRecorder_create();
  if(devname != NULL){
    sfSoundBufferRecorder_setDevice(mic, devname);
  }
  
  const char *name = sfSoundBufferRecorder_getDevice(mic);
  fprintf(stderr, "[%s]: ", name);
  if(sfSoundRecorder_isAvailable()){
    fprintf(stderr, "avaible\n");
  }else{
    fprintf(stderr, "fail\n");
    runflag = 0;
  }
  float sps = 0;
  unsigned long t_prev=0, t_cur=0;
  //unsigned long samplerate=22000, time_us=1000000;
  sfSoundBufferRecorder_start(mic, samplerate);
  while(runflag && wnd_update()){
    usleep( sampletime_ms*1000 );
    
    sfSoundBufferRecorder_stop(mic);
    const sfSoundBuffer *sndbuf = sfSoundBufferRecorder_getBuffer(mic);
    uint64_t count = sfSoundBuffer_getSampleCount(sndbuf);
    const int16_t *data = sfSoundBuffer_getSamples(sndbuf);
    data_update(data, count);
    
    sfSoundBufferRecorder_start(mic, samplerate);
    
    if(data_count_flag)fprintf(stderr, "%"PRIu64"\n", count);
    if(data_count_capt && show_window_flag){
      t_cur = get_time_ms();
      float sps_cur = (float)count/(t_cur-t_prev)*1000;
      sps = sps*0.9 + 0.1*sps_cur;
      char buf[50];
      sfVector2i cur_pos = sfMouse_getPositionRenderWindow(wnd);
      sps_cur = cur_pos.y;
      sps_cur = range_min + (sps_cur/wndsize.height)*(range_max-range_min);
      sprintf(buf, "%6.6lu -> %9.2f sps ; cur=%9.2f", count, sps, sps_cur);
      sfRenderWindow_setTitle(wnd, buf);
      t_prev = t_cur;
    }
  }
  
  sfSoundBufferRecorder_destroy(mic);
  if(wnd)sfRenderWindow_destroy(wnd);
  free(data_buf);
  fprintf(stderr, "\nDone\n");
  return 0;
}
