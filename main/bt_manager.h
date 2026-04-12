#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void bt_init(void);
int  bt_available(void);
char bt_read(void);

#ifdef __cplusplus
}
#endif
