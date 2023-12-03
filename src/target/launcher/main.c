#include <stdint.h>
#include "video.h"
#include "pci.h"
#include "ether.h"
#include "proto.h"
#include "util.h"
#include "cdfs.h"

#define DREAMCAST_IP { 10, 0, 1, 64 }
#define PC_IP { 10, 0, 1, 73 }

#define IPBIN_SIZE 0x8000
#define IPBIN_ADDR 0x8c008000
#define START_LBA 45150
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

int get_boot_info(char *bootname, char *device)
{
  int namelen;
  if (memcmp(IPBIN_ADDR, "SEGA SEGAKATANA ", 16)) {
    printf("Invalid bootsector\n");
    return -1;
  }
  memset(device, 0, 8);
  memcpy(device, IPBIN_ADDR+0x25, 6);
  memcpy(bootname, IPBIN_ADDR+0x60, 16);
  for(namelen=16; namelen>0; --namelen)
    if(bootname[namelen-1] != ' ')
      break;
  bootname[namelen] = 0;
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
  setup_video();
  setup_network();

  // which iso to load from the server
  // default to 0 to take the first one
  int disc_id = 0;
  int sz;
  sz = wait_command_packet(send_command_packet(998, &disc_id, 1));
  if(sz < IPBIN_SIZE || sz > MAX_BINARY) {
    printf("FAILED!");
  }

  printf("Patching syscalls at %x with %d bytes\n", SUBSTART, subcode_end - subcode);
  install_patch(SUBSTART);

  // Not sure why we jump to SUBSTART+4, it seems to match up with
  // the two skipped instructions at the beginning of "start" routine in sub.s
  // but removing those and not padding the location here does not work so
  // we keep as is ¯\_(ツ)_/¯
  printf("Installing syscall handler and network init\n");
  setup_handler(SUBSTART+4);

  // Now we can get the binaries using gdrom commands
  cdfs_init();

  int num_sectors = IPBIN_SIZE>>11;
  printf("Fetching first %d sectors\n", num_sectors);
  read_sectors(IPBIN_ADDR, START_LBA, num_sectors);

  char bootname[20];
  char device[8];
  get_boot_info(bootname, device);
  printf("Device info: \"%s\"\n", device);
  printf("Downloading boot filename: \"%s\"\n", bootname);

  int fd, r;
  fd = open(bootname, O_RDONLY);
  if (fd < 0)
    printf("File not found: %s\n", bootname);
  else {
    static char *buf = IPBIN_ADDR + (16<<11);
    int i = 0;
    int bufsize = 2048;
    while(r = read(fd, buf, bufsize) > 0) {
      buf += (r * bufsize);
      if (i++ % 15 == 0)
        printf(".");
      if (i % (15 * 50) == 0)
        printf("\n");
    }
    printf("\n");
  }

  //ether_teardown();
  // Start in ip.bin
  printf("Launching IP.BIN bootloader\n");
  launch(0x8c00B800);
  // Start in 1st_read.bin
  // printf("Launching %s\n", bootname);
  //launch(0x8c010000);
}
