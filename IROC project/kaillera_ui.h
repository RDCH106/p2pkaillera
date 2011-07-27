#pragma once

#include "kcore/kaillera_core.h"

bool kaillera_SelectServerDlgStep();
void kaillera_EndGame();
void kaillera_GUI();
void kaillera_ui_chat_send(char * xxx);
//char * callback_checksize(char * msg);

bool kaillera_RecordingEnabled();
bool ChatLogEnabled();
bool GameChatLogEnabled();
void ChatLog(char * msg);
void GameChatLog(char * msg);


