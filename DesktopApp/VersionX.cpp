#include "Version.h"

#define NOTE_SECTION_NAME ".note.mailager.version"

// Exported symbol visible via nm/strings
extern "C" const char app_version[] = APP_NAME " " APP_VERSION;

// Embed metadata into an ELF NOTE section
__attribute__((section(NOTE_SECTION_NAME)))
extern "C" const char app_version_note[] = APP_NAME " " APP_VERSION;
