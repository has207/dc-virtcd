#include "video.h"
#include "pci.h"
#include "ether.h"
#include "proto.h"
#include "util.h"

#define MAX_BINARY (15*1024*1024)

#define SUBSTART 0x8c008300

static int download_addr, download_size;

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
  unsigned char my_ip[4] = { 10, 0, 1, 64 };
  ip_set_my_ip(my_ip);

  printf("My IP address: ");
  if(ip_get_my_ip(&ip_no.i))
    printf("%d.%d.%d.%d\n",
	   ip_no.b[0], ip_no.b[1], ip_no.b[2], ip_no.b[3]);
  else
    printf("Unknown\n");
  unsigned char ip[4] = { 10, 0, 1, 73 };
  set_server(ip, host_mac);
}

int select_binary(int n)
{
  int c, sz;
  printf("Requesting binary...");
  sz = wait_command_packet(send_command_packet(998, &n, 1));
  if(sz < 0 || sz > MAX_BINARY) {
    printf("FAILED!");
    return -1;
  }
  printf("%d bytes\n", sz);
  download_start = download_addr = 0x8c008000;
  download_size = sz+0x8000;
  return 0;
}

int download()
{
  int tot = download_size;
  int tmp, cur = 10;
  int c[8], param[2];
  int i;
  printf("Downloading [          ]\r             ");
  for(i=0; i<8; i++)
    c[i] = -1;
  i = 0;
  for(;;) {
    tmp = (10*download_size)/tot;
    while(tmp<cur) {
      printf("*");
      --cur;
    }
    if(!download_size) {
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
    tmp = download_size;
    if(tmp > 1024)
      tmp = 1024;
    param[0] = download_addr;
    param[1] = tmp;
    c[i] = send_command_packet(997, param, 2);
    download_addr += tmp;
    download_size -= tmp;
    if(++i >= 8)
      i = 0;
  }
  return 0;
}

void run()
{
  extern char subcode[], subcode_end[];
  unsigned char server_mac[6];
  unsigned int my_ip, server_ip;

  get_server_mac(server_mac);
  get_server_ip(&server_ip);
  ip_get_my_ip(&my_ip);
  printf("Running...");
  ether_teardown();
  setimask(15);
  memcpy((void*)SUBSTART, subcode, subcode_end - subcode);
  ((void (*)(unsigned int, unsigned int, void *))(void*)(SUBSTART+4))
    (my_ip, server_ip, server_mac);
  //launch(0x8c010000);
  launch(download_start + 0x00008000);
  printf("Execution terminated.");
}

void main()
{
  //struct smb2_context *smb2;
  //smb2 = smb2_init_context();
  //cdrom_spin_down();
  setup_video();
  setup_network();
  if (select_binary(0) < 0) return;
  if (download() < 0) return;
  run();
}
