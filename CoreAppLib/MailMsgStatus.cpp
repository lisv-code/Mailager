#include "MailMsgStatus.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

namespace MailMsgStatus_Imp
{
	// ****** Internal functions declaration ******
	static uint32_t parse_mailager_status(const char* value);
	static uint32_t parse_opera_status(const char* value);
	static uint32_t translate_status_opera_to_mailager(uint32_t value);
}
using namespace MailMsgStatus_Imp;

MailMsgStatus MailMsgStatusCodec::ParseStatusString(const char* value, MailMsgStatusStringType type)
{
	switch (type) {
	case mstMailager: return (MailMsgStatus)parse_mailager_status(value);
	case mstOpera: return (MailMsgStatus)translate_status_opera_to_mailager(parse_opera_status(value));
	default: return (MailMsgStatus)0;
	}
}

void MailMsgStatusCodec::ConvertStatusToString(MailMsgStatus status, std::string& status_value)
{
	status_value += MailMsgStatus_DataSignature;
	char buf[0xF];
	std::snprintf(buf, sizeof(buf), "%08x", status);
	status_value += buf;
}

// ******************************* Internal functions implementation *******************************

// X-Mailager-Status header format
// name (hex chars): version (2) | status_flags1 (8)

static uint32_t MailMsgStatus_Imp::parse_mailager_status(const char* value)
{
	size_t len = strlen(value);
	if (len < 10) return 0; // check length
	if ((MailMsgStatus_DataSignature[0] != value[0])
		&& (MailMsgStatus_DataSignature[1] != value[1])) return 0; // check signature/version
	char buf[9];
	strncpy(buf, value + 2, 8);
	buf[8] = 0;
	return strtoul(buf, NULL, 16);
}

// X-Opera-Status header format
// meaning: ver     ?_1  msg_id  ?_3    ?_4    flags1  ?_6    ?_7 - ?_14
// hex pos: 0-1     2-9  10-17   18-25  26-33  34-41   42-49  50-57 - 106-113
// value  : 0b0101  ~0   <uint>  ?      ?      <enum>  ~?_4   ?

static uint32_t MailMsgStatus_Imp::parse_opera_status(const char* value)
{
	size_t len = strlen(value);
	if (len < 42) return 0; // check length
	if ('0' != value[0] && '5' != value[1]) return 0; // check signature/version
	char buf[9];
	strncpy(buf, value + 34, 8);
	buf[8] = 0;
	return strtoul(buf, NULL, 16);
}

enum MailMsgStatus_OperaFlags1
{
	is_read = 1,
	is_replied = 1 << 1,
	is_forwarded = 1 << 2,
	is_resent = 1 << 3,
	is_outgoing = 1 << 4,
	is_sent = 1 << 5,
	is_queued = 1 << 6,
	is_seen = 1 << 7,
	is_timequeued = 1 << 8,
	is_deleted = 1 << 9,
	has_attachment = 1 << 10,
	has_image_attachment = 1 << 11,
	has_audio_attachment = 1 << 12,
	has_video_attachment = 1 << 13,
	has_zip_attachment = 1 << 14,
	is_onepart_message = 1 << 15,
	_unused_1 = 1 << 16,
	is_waiting_for_indexing = 1 << 17,
	is_newsfeed_message = 1 << 18,
	partially_fetched = 1 << 19,
	is_news_message = 1 << 20,
	allow_8bit = 1 << 21,
	is_imported = 1 << 22,
	has_priority = 1 << 23,
	has_priority_low = 1 << 24,
	has_priority_max = 1 << 25,
	is_control_message = 1 << 26,
	allow_quotestring_qp = 1 << 27,
	has_keywords = 1 << 28,
	permanently_removed = 1 << 29,
	dir_forced_ltr = 1 << 30,
	dir_forced_rtl = 1 << 31
};

static uint32_t MailMsgStatus_Imp::translate_status_opera_to_mailager(uint32_t value)
{
	uint32_t dst = 0;
	dst = dst | ((MailMsgStatus_OperaFlags1::is_read | MailMsgStatus_OperaFlags1::is_seen)
		& value ? mmsIsSeen : 0);
	dst = dst | (MailMsgStatus_OperaFlags1::is_replied & value ? mmsIsAnswered : 0);
	dst = dst | (MailMsgStatus_OperaFlags1::has_priority_max & value ? mmsIsFlagged : 0);
	dst = dst | (MailMsgStatus_OperaFlags1::is_deleted & value ? mmsIsDeleted : 0);
	//dst = dst | (MailMsgStatus_OperaFlags1::<?> & value ? mmsIsDraft : 0);

	dst = dst | (MailMsgStatus_OperaFlags1::is_outgoing & value ? mmsIsOutgoing : 0);
	dst = dst | (MailMsgStatus_OperaFlags1::is_sent & value ? mmsIsSent : 0);
	return dst;
}
