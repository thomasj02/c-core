#include "pubnub_sync.h"
#include "pnc_config.h"
#include "pnc_subscriptions.h"

void pnc_ops_init();
void pnc_ops_subscribe(pubnub_t *pn);
void pnc_ops_unsubscribe(pubnub_t *pn);
void pnc_ops_parse_response(const char *method_name, enum pubnub_res res, pubnub_t *pn);