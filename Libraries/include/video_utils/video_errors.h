#ifndef SMIRVIDEOTOOL_VIDEO_ERRORS_H
#define SMIRVIDEOTOOL_VIDEO_ERRORS_H

typedef enum {
    SUCCESS_VAL = 0,
    INSUFFICIENT_MEMORY_ERROR = 100,
    EMPTY_FRAME_ERROR = 101,
    CAP_NOT_OPENED_ERROR = 102,
    CAP_WRONG_FPS_RES_ERROR = 103,
    FILE_ALREADY_OPEN = 104,
    FILE_COULD_NOT_OPEN = 105,
    MAX_FILE_COUNTER_ACHIEVED = 106,
} FFMPEG_Error_Types;

#endif //SMIRVIDEOTOOL_VIDEO_ERRORS_H
