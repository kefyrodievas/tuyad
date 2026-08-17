#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include "memory.h"

enum
{
	TOTAL_MEMORY,
	FREE_MEMORY,
	SHARED_MEMORY,
	BUFFERED_MEMORY,
	__MEMORY_MAX,
};

enum
{
	MEMORY_DATA,
	__INFO_MAX,
};

int rc = 0;

static const struct blobmsg_policy memory_policy[__MEMORY_MAX] = {
	[TOTAL_MEMORY] = {.name = "total", .type = BLOBMSG_TYPE_INT64},
	[FREE_MEMORY] = {.name = "free", .type = BLOBMSG_TYPE_INT64},
	[SHARED_MEMORY] = {.name = "shared", .type = BLOBMSG_TYPE_INT64},
	[BUFFERED_MEMORY] = {.name = "buffered", .type = BLOBMSG_TYPE_INT64},
};

static const struct blobmsg_policy info_policy[__INFO_MAX] = {
	[MEMORY_DATA] = {.name = "memory", .type = BLOBMSG_TYPE_TABLE},
};

static void board_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	struct meminfo *memoryData = (struct meminfo *)req->priv;
	struct blob_attr *tb[__INFO_MAX];
	struct blob_attr *memory[__MEMORY_MAX];

	blobmsg_parse(info_policy, __INFO_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[MEMORY_DATA])
	{
		fprintf(stderr, "No memory data received\n");
		rc = -1;
		return;
	}

	blobmsg_parse(memory_policy, __MEMORY_MAX, memory, blobmsg_data(tb[MEMORY_DATA]),
				  blobmsg_data_len(tb[MEMORY_DATA]));

	memoryData->total = blobmsg_get_u64(memory[TOTAL_MEMORY]);
	memoryData->free = blobmsg_get_u64(memory[FREE_MEMORY]);
	memoryData->shared = blobmsg_get_u64(memory[SHARED_MEMORY]);
	memoryData->buffered = blobmsg_get_u64(memory[BUFFERED_MEMORY]);
}

int get_memory_info(struct meminfo *memory)
{
	struct ubus_context *ctx;
	uint32_t id;

	memory->buffered = 0;
	memory->free = 0;
	memory->shared = 0;
	memory->total = 0;

	ctx = ubus_connect(NULL);
	if (!ctx)
	{
		rc = -1;
		goto end;
	}

	if (ubus_lookup_id(ctx, "system", &id) ||
		ubus_invoke(ctx, id, "info", NULL, board_cb, memory, 3000))
	{
		rc = -1;
		goto end;
	}
end:
	if (ctx)
		ubus_free(ctx);
	return rc;
}