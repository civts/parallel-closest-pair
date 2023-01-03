#! /usr/bin/env bash

# The bot token and chat ID are populated by Github's CI.

# Please, do not bother copying the token since this bot is used only for
# university projects and will be reset soon
BOT_TOKEN="<BOT_TOKEN>"
TELEGRAM_CHAT_ID="<TELEGRAM_CHAT_ID>"

SILENT="True"

READING_FILE_NAME=false
HAS_ATTACHMENT=false

for arg in "$@"; do
  if [[ $READING_FILE_NAME == "true" ]]; then
    FILE_PATH="$arg"
    READING_FILE_NAME=false
  elif [[ "$arg" == "--loud" ]]; then
    SILENT="False"
  elif [[ "$arg" == "--file" ]]; then
    READING_FILE_NAME=true
    HAS_ATTACHMENT=true
  fi
done

if [[ "$HAS_ATTACHMENT" == "true" ]]; then
  # Send message with attachment
  curl -s -X POST \
    -F document=@"$FILE_PATH" \
    -F "chat_id=$TELEGRAM_CHAT_ID" \
    -F "caption=${1}" \
    -F "disable_notification=$SILENT" \
    "https://api.telegram.org/bot$BOT_TOKEN/sendDocument" \
    &>/dev/null
else
  # Send simple message
  curl -s -X POST \
    -F "chat_id=$TELEGRAM_CHAT_ID" \
    -F "disable_notification=$SILENT" \
    -F "text=${1}" \
    "https://api.telegram.org/bot$BOT_TOKEN/sendMessage" \
    &>/dev/null
fi
