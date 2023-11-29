#include "video.h"
#include "pci.h"
#include "ether.h"
#include "proto.h"
#include "util.h"

#define DREAMCAST_IP { 10, 0, 1, 64 }
#define PC_IP { 10, 0, 1, 73 }

#define IPBIN_SIZE 0x8000
#define DOWNLOAD_ADDR 0x8c008000
#define MAX_BINARY (15*1024*1024)

// Location of syscall patch, must match value in handler/Makefile
#define SUBSTART 0x8c005000

extern char subcode[], subcode_end[];

void debug_dump(unsigned char *data, int sz)
{
  int i;
  for(i=0; i<sz; i++) {
    if(!(i&15))
      printf("\n");
    printf(" %02x", data[i]);
  }
  printf("\n------\n");
}

void halt()
{
  setimask(15);
  for(;;)
    idle();
}

void setup_video()
{
  int tv, cable;
  cable = video_check_cable(&tv);
  video_init_pvr();
  video_init_video(cable, 1, 0, 2, tv&1, tv, 0);
  video_clear_area(0, 0, 640, 480);
  printf("\014");
}

void setup_network()
{
  unsigned char sec[64];
  union {
    int i;
    unsigned char b[4];
  } ip_no;
  int i;
  printf("Initializing GAPSPCI bridge...");
  if(pci_setup()<0) {
    printf("FAILED!");
    halt();
  }
  printf("Ok\n");
  printf("Initializing RTL8139 chip...");
  if(ether_setup()<0) {
    printf("FAILED!");
    halt();
  }
  printf("MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
	 ether_MAC[0], ether_MAC[1], ether_MAC[2], 
	 ether_MAC[3], ether_MAC[4], ether_MAC[5]);
  unsigned char my_ip[4] = DREAMCAST_IP;
  ip_set_my_ip(my_ip);

  printf("My IP address: ");
  if(ip_get_my_ip(&ip_no.i))
    printf("%d.%d.%d.%d\n",
	   ip_no.b[0], ip_no.b[1], ip_no.b[2], ip_no.b[3]);
  else
    printf("Unknown\n");
  unsigned char ip[4] = PC_IP;
  set_server(ip);
}

int get_bin_size()
{
  // which iso to load from the server
  // default to 0 to take the first one
  int disc_id = 0;
  int sz;
  sz = wait_command_packet(send_command_packet(998, &disc_id, 1));
  if(sz < IPBIN_SIZE || sz > MAX_BINARY) {
    printf("FAILED!");
    return -1;
  }
  return sz + IPBIN_SIZE;
}

int download(int addr, int size)
{
  int tot = size;
  int tmp, cur = 10;
  int c[8], param[2];
  int i;
  printf("Downloading [          ]\r             ");
  for(i=0; i<8; i++)
    c[i] = -1;
  i = 0;
  for(;;) {
    tmp = (10*size)/tot;
    while(tmp<cur) {
      printf("*");
      --cur;
    }
    if(size == 0) {
      for(i=0; i<8; i++)
	if(c[i]>=0)
	  if(wait_command_packet(c[i])<0) {
	    printf("  FAILED!");
	    return -1;
	  }
      printf("  Done\n");
      break;
    }
    if(c[i]>=0)
      if(wait_command_packet(c[i])<0) {
	printf("  FAILED!");
	return -1;
      }
    tmp = size;
    if(tmp > 1024)
      tmp = 1024;
    param[0] = addr;
    param[1] = tmp;
    c[i] = send_command_packet(997, param, 2);
    addr += tmp;
    size -= tmp;
    if(++i >= 8)
      i = 0;
  }
  return 0;
}

int install_patch(int patch_location)
{
  int s = getimask();
  setimask(15);
  memcpy((void*)patch_location, subcode, subcode_end - subcode);
  setimask(s);

  return 0;
}

int setup_handler(int patch_location)
{
  unsigned int my_ip, server_ip;

  get_server_ip(&server_ip);
  ip_get_my_ip(&my_ip);

  ((void (*)(unsigned int, unsigned int))(void*)(patch_location))
    (my_ip, server_ip);

}

void main()
{
  int bin_size;

  setup_video();
  setup_network();

  printf("Requesting binary size from server...");
  if ((bin_size = get_bin_size()) < IPBIN_SIZE) return;
  printf("\n");
  printf("IP.BIN %d\n", IPBIN_SIZE);
  printf("1ST_READ.BIN %d\n", bin_size - IPBIN_SIZE);

  printf("Patching syscalls at %x with %d bytes\n", SUBSTART, subcode_end - subcode);
  install_patch(SUBSTART);

  // Not sure why we jump to SUBSTART+4, it seems to match up with
  // the two skipped instrctions at the beginning of "start" routine in sub.s
  // but removing those and not padding the location here does not work so
  // we keep as is
  printf("Installing syscall handler and running init\n");
  setup_handler(SUBSTART+4);

  if (download(DOWNLOAD_ADDR, bin_size) < 0) return;

  //ether_teardown();
  // Start in ip.bin
  launch(0x8c00B800);
  // Start in 1st_read.bin
  //launch(0x8c010000);
}
