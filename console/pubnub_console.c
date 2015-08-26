/* -*- c-file-style:"stroustrup" -*- */
#include "pubnub_console.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static const char pubkey[] = "demo";
static const char subkey[] = "demo";
static const char channel[] = "hello_world";

int main()
{
  pubnub_t *pn = pubnub_alloc();
  pubnub_t *pn_sub = pubnub_alloc();

  if (NULL == pn || NULL == pn_sub) {
    printf("Failed to allocate pubnub context");
    return -1;
  }

  pnc_ops_init(pn, pn_sub);

  pubnub_init(pn, pubkey, pubkey);
  pubnub_init(pn_sub, pubkey, pubkey);
  pubnub_set_uuid(pn, PNC_DEFAULT_UUID);
  pubnub_set_uuid(pn, PNC_DEFAULT_SUBSCRIBE_UUID);

  displayMenuOptions(pn);

  int option = 0;
  char opt_string[4];
  char channel[PNC_CHANNEL_NAME_SIZE];
  char channel_group[PNC_CHANNEL_NAME_SIZE];
  char auth_key[PNC_AUTH_KEY_SIZE];
  char uuid[PNC_UUID_SIZE];
  char state[2048];

  while(option != 99) {
    fgets(opt_string, sizeof(opt_string), stdin);
    opt_string[strlen(opt_string) - 1] = '\0';
    option = atoi(opt_string);
    printf("You entered %d(%s)\n", option, opt_string);

    switch (option) {
      case 0:
        break;
      case 1:
        // Publish
        {
          bool store = false;
          // TODO: add json builded string

          pnc_read_string_from_console("Channel Name",
              channel, PNC_CHANNEL_NAME_SIZE);
          pnc_read_bool_from_console_optional("Store", &store, true);

          puts("Enter the message for publish. To exit loop enter Q");

          char message[PNC_READ_STRING_SIZE];
          int attempt_count = 0;

          while (fgets(message, PNC_READ_STRING_SIZE, stdin) != NULL && strcmp(message, "Q\n")) {
            if (strlen(message) == 1) {
              puts("Invalid input");
              continue;
            }

            message[strlen(message) - 1] = '\0';

            if (message[0] == '{' && message[strlen(message) - 1] == '}') {
              pnc_ops_parse_response("pubnub_publish()",
                  pubnub_publish(pn, channel, message), pn);
            } else {
              char encoded_message[strlen(message) + 2];
              sprintf(encoded_message, "\"%s\"", message);
              pnc_ops_parse_response("pubnub_publish()",
                  pubnub_publish(pn, channel, encoded_message), pn);
            }

            puts("Enter the message for publish. To exit loop enter Q.");
            attempt_count++;
          }
        }

        break;
      case 2:
        // History
        {
          int count = 100;
          bool include_token = false;

          pnc_read_int_from_console_optional("Count", &count, true);

          if (count <= 0) {
            count = 100;
            puts("Count value cannot be less than 1. Default value(100) was applied.");
          }

          pnc_read_string_from_console("Channel Name",
              channel, PNC_CHANNEL_NAME_SIZE);

          // WARNING: Default buffer can be not big enough to get 100-items
          // history response with timetokens
          pnc_read_bool_from_console("Include timetokens", &include_token);

          pnc_ops_parse_response("pubnub_historyv2()",
              pubnub_historyv2(pn, channel, NULL, count, include_token), pn);
        }

        break;
      case 3:
        // Here now
        pnc_read_string_from_console_optional("Channel Name",
            channel, PNC_CHANNEL_NAME_SIZE, true);

        pnc_ops_parse_response("pubnub_here_now()",
            pubnub_here_now(pn, channel, NULL), pn);
        break;
      case 4:
        // Time
        pnc_ops_parse_response("pubnub_time()", pubnub_time(pn), pn);
        break;
      case 5:
        // Set Auth
        pnc_read_string_from_console("Auth key", auth_key, PNC_AUTH_KEY_SIZE);
        pubnub_set_auth(pn, auth_key);

        break;
      case 6:
        // Set UUID
        pnc_read_string_from_console("UUID", uuid, PNC_UUID_SIZE);
        pubnub_set_uuid(pn, uuid);

        break;
      case 7:
        // Get State
        pnc_read_string_from_console("Channel Name",
            channel, PNC_CHANNEL_NAME_SIZE);
        pnc_read_string_from_console_optional("UUID",
            uuid, PNC_UUID_SIZE, true);

        if (strlen(uuid) == 0) {
          strcpy(uuid, pubnub_get_uuid(pn));
        }

        pnc_ops_parse_response("pubnub_get_state()",
            pubnub_state_get(pn, channel, NULL, uuid), pn);
        break;
      case 8:
        // Set State
        pnc_read_string_from_console("Channel Name",
            channel, PNC_CHANNEL_NAME_SIZE);
        pnc_read_string_from_console("JSON state without escaping",
            state, PNC_READ_STRING_SIZE);
        pnc_ops_parse_response("pubnub_set_state()",
            pubnub_set_state(pn, channel, NULL, pubnub_get_uuid(pn), state), pn);
        break;
      case 9:
        // Where Now
        pnc_read_string_from_console_optional("UUID",
            uuid, PNC_UUID_SIZE, true);
        if (strlen(uuid) == 0) {
          puts("Using current UUID");
          strcpy(uuid, PNC_DEFAULT_UUID);
        }

        pnc_ops_parse_response("pubnub_where_now()",
            pubnub_where_now(pn, uuid), pn);
        break;
      case 10:
        // [Current Subscription] Add Presence Channel
        {
          pnc_read_string_from_console("Channel name",
              channel, PNC_CHANNEL_NAME_SIZE);

          char presence_channel[PNC_CHANNEL_NAME_SIZE + strlen(PNC_PRESENCE_SUFFIX)];

          strcpy(presence_channel, channel);
          strcat(presence_channel, PNC_PRESENCE_SUFFIX);

          pnc_subscribe_add_channel(presence_channel);
        }

        break;
      case 11:
        // [Current Subscription] Subscribe
        pnc_ops_subscribe(pn_sub);
        break;
      case 12:
        // [Current Subscription] Unsubscribe
        pnc_ops_unsubscribe(pn_sub);
        break;
      case 13:
        // [Current Subscription] Add Channel
        pnc_read_string_from_console("Enter Channel name",
            channel, PNC_CHANNEL_NAME_SIZE);
        pnc_subscribe_add_channel(channel);
        break;
      case 14:
        // [Current Subscription] Add Channel Group
        pnc_read_string_from_console("Enter Group name",
            channel, PNC_CHANNEL_NAME_SIZE);
        pnc_subscribe_add_channel_group(channel);
        break;
      case 15:
        // [Current Subscription] Remove Channel
        pnc_read_string_from_console("Enter Channel name",
            channel, PNC_CHANNEL_NAME_SIZE);
        pnc_subscribe_remove_channel(channel);
        break;
      case 16:
        // [Current Subscription] Remove Channel Group
        pnc_read_string_from_console("Enter Group name",
            channel, PNC_CHANNEL_NAME_SIZE);
        pnc_subscribe_remove_channel_group(channel);
        break;
      case 17:
        // [Channel Group] Add channel
        pnc_read_string_from_console("Group name",
            channel_group, PNC_CHANNEL_NAME_SIZE);
        pnc_read_string_from_console("Channel to add",
            channel, PNC_CHANNEL_NAME_SIZE);

        pnc_ops_parse_response("pubnub_add_channel_to_group()",
            pubnub_add_channel_to_group(pn, channel, channel_group), pn);

        break;
      case 18:
        // Remove channel
        pnc_read_string_from_console("Group name",
            channel_group, PNC_CHANNEL_NAME_SIZE);
        pnc_read_string_from_console("Channel to remove",
            channel, PNC_CHANNEL_NAME_SIZE);

        pnc_ops_parse_response("pubnub_remove_channel_from_group()",
            pubnub_remove_channel_from_group(pn, channel, channel_group), pn);

        break;
      case 19:
        // List channels
        pnc_read_string_from_console("Group to list",
            channel_group, PNC_CHANNEL_NAME_SIZE);

        pnc_ops_parse_response("pubnub_list_channel_group()",
            pubnub_list_channel_group(pn, channel_group), pn);

        break;
      case 20:
        // Remove Group
        pnc_read_string_from_console("Group to remove",
            channel, PNC_CHANNEL_NAME_SIZE);
        pubnub_remove_channel_group(pn, channel);
        break;
      default:
        printf("Invalid input: %d\n", option);
    }

    printf("\n================================\n\n");
    displayMenuOptions(pn);
  }

  puts("Exiting");

  if (pubnub_free(pn) != 0) {
    puts("Failed to free the Pubnub context");
  }

  return 0;
}

void displayMenuOptions(pubnub_t *pn) {
  char channels_string[PNC_SUBSCRIBE_CHANNELS_LIMIT * PNC_CHANNEL_NAME_SIZE];
  char channel_groups_string[PNC_SUBSCRIBE_CHANNELS_LIMIT * PNC_CHANNEL_NAME_SIZE];
  pnc_subscribe_list_channels(channels_string);
  pnc_subscribe_list_channel_groups(channel_groups_string);


  printf("Active Channels/Channel Groups are: \"%s\"/\"%s\"\n\n",
      channels_string, channel_groups_string);

  puts("ENTER 1  FOR Publish");
  puts("ENTER 2  FOR History");
  puts("ENTER 3  FOR Here Now");
  puts("ENTER 4  FOR Time");
  printf("ENTER 5  FOR Setting/Unsetting auth key ( current: %s )\n", pnc_get_auth(pn));
  printf("ENTER 6  FOR Setting UUID ( current: %s)\n", pnc_get_uuid(pn));
  puts("ENTER 7  FOR Getting Subscriber State");
  puts("ENTER 8  FOR Setting Subscriber State");
  puts("ENTER 9  FOR Where Now");
  puts("ENTER 10 FOR [Current Subscription] Add Presence Channel");
  puts("ENTER 11 FOR [Current Subscription] Subscribe");
  puts("ENTER 12 FOR [Current Subscription] Unsubscribe");
  puts("ENTER 13 FOR [Current Subscription] Add Channel");
  puts("ENTER 14 FOR [Current Subscription] Add Channel Group");
  puts("ENTER 15 FOR [Current Subscription] Remove Channel");
  puts("ENTER 16 FOR [Current Subscription] Remove Channel Group");
  puts("ENTER 17 FOR [Channel Group] Add Channel");
  puts("ENTER 18 FOR [Channel Group] Remove Channel");
  puts("ENTER 19 FOR [Channel Group] List Channels");
  puts("ENTER 20 FOR [Channel Group] Remove Group");
  puts("\nENTER 0 to display this menu");
  puts("ENTER 99 FOR EXIT OR QUIT");
  printf("> ");
}

char const *pnc_get_auth(pubnub_t *pb) {
  char const *auth = pb->core.auth;

  if (auth == NULL) {
    return "(NOT SET)";
  } else {
    return auth;
  }
}

char const *pnc_get_uuid(pubnub_t *pb) {
  char const *uuid = pb->core.uuid;

  if (uuid == NULL) {
    return "(NOT SET)";
  } else {
    return uuid;
  }
}