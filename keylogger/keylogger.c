#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#define MAX_CHAR 4

pthread_mutex_t mutex_device;
int found = -1;
char *found_path_file;


// returns "/dev/input/eventX" where X is int parameter
char *get_event_file_path(int id_file){
  char id_file_c[MAX_CHAR];
  snprintf(id_file_c, sizeof(id_file_c), "%d", id_file);
  char *partial_path = "/dev/input/event";
  char *complete_path = (char *) malloc(strlen(partial_path)+strlen(id_file_c));
  strcpy(complete_path, partial_path);
  strcat(complete_path, id_file_c);
  return complete_path;
}

// thread listening function
void *listen_event_file(void *arg)
{
  int id_file = (int) *((int *) arg);
  char * path_file = get_event_file_path(id_file);
  printf("listen to %s\n",path_file);
  int fd = open(path_file, O_RDONLY | O_NONBLOCK);

  struct input_event ev;
  while (found < 0)
    {
      int count = read(fd, &ev, sizeof(struct input_event));
      if (ev.type == 1)
	{
	  pthread_mutex_lock(&mutex_device);
	  found = id_file;
	  pthread_mutex_unlock(&mutex_device);
	  found_path_file = path_file;
	  
	}
      pthread_yield();
    }
  pthread_exit(NULL);
}

int main(int argc, char **argv)
{
  
  // count number of devices 
  char count_device_command[] = "ls /dev/input/event* | wc -w";
  char count_device_value[MAX_CHAR];
 
  FILE* fp = popen(count_device_command, "r");
  while (fgets(count_device_value, MAX_CHAR, fp) != NULL);
  pclose(fp);

  int nb_device = atoi(count_device_value);
  pthread_mutex_init(&mutex_device, NULL);
  
  // one thread to listen each device
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_t listener[nb_device];
  int device_id[nb_device];
  void *status;

  printf("found %d devices\n",nb_device);

  int i;
  for (i=0; i<nb_device;i++)
    device_id[i] = i;

  for (i=0; i<nb_device; i++) 
    pthread_create(&listener[i], &attr, listen_event_file, (void *) &device_id[i]);
  
  pthread_attr_destroy(&attr);
  
  for (i=0; i<nb_device; i++) 
    pthread_join(listener[i], &status);

  pthread_mutex_destroy(&mutex_device);


  printf("keyboard found in file %s\n",found_path_file);

  
  int fd = open(found_path_file, O_RDONLY);
  struct input_event ev;
  
  while (1)
    {
      read(fd, &ev, sizeof(struct input_event));

      if(ev.type == 1)
    	printf("key %i state %i\n", ev.code, ev.value);
      
    }
}
