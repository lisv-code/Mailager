#pragma once
#ifndef _LIS_MIME_MEDIA_TYPES_H_
#define _LIS_MIME_MEDIA_TYPES_H_

// RFC 2046 - Media Types.
// RFC 2077 - The Model Primary Content Type. RFC 8081 - The "font" Top-Level Media Type.
// See also: https://www.iana.org/assignments/media-types/media-types.xhtml

// Discrete top-level media types
#define MimeMediaType_Text "text"
#define MimeMediaType_Image "image"
#define MimeMediaType_Audio "audio"
#define MimeMediaType_Video "video"
#define MimeMediaType_Application "application"
#define MimeMediaType_Model "Model"
#define MimeMediaType_Font "Font"

// Composite top-level media types
#define MimeMediaType_Multipart "multipart"
#define MimeMediaType_Message "message"

// Subtypes of Text
#define MimeMediaSubType_Plain "plain"
#define MimeMediaSubType_Html "html"

// Subtypes of Application
#define MimeMediaSubType_OctetStream "octet-stream"

// Subtypes of Multipart
#define MimeMediaSubType_Mixed "mixed"
#define MimeMediaSubType_Alternative "alternative"
#define MimeMediaSubType_Parallel "parallel"
#define MimeMediaSubType_Digest "digest"

// Subtypes of Message
#define MimeMediaSubType_Rfc822 "rfc822"
#define MimeMediaSubType_Partial "partial"
#define MimeMediaSubType_ExternalBody "external-body"

#endif // #ifndef _LIS_MIME_MEDIA_TYPES_H_
