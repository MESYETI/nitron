#ifndef N_DRIVER_H
#define N_DRIVER_H

typedef struct {
	void (*init)(VM*);
	void (*free)(VM*);

	size_t callAmount;
	void   (**calls)(VM*);
} Driver;

#endif
