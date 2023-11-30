typedef struct isofs_s *isofs;
extern int isofs_init(isofs iso, unsigned char *data);
extern int isofs_find_file(isofs iso, const char *filename,
			    uint32_t *sector, uint32_t *length);
