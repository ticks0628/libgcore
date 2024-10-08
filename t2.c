// 定義功能和對應的函數
#define ACTION_LIST \
    X(eACT_SD_RECORD, fACT_SD_RECORD, foo()) \
    X(eACT_SD_SNAPSHOT, fACT_SD_SNAPSHOT, foo())

// 產生 enum 定義
enum ENUM_ACT {
    #define X(ENUM_NAME, DEFINE_NAME, FUNC) ENUM_NAME,
    ACTION_LIST
    #undef X
};

// 產生 define 定義
#define X(ENUM_NAME, DEFINE_NAME, FUNC) \
    #ifndef DEFINE_NAME \
    #define DEFINE_NAME FUNC \
    #endif

// 展開 define
ACTION_LIST

#undef X

