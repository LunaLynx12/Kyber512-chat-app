@echo off
docker rm -f secure-chat-container 2>nul
docker build -t quantumchat .
docker run --name secure-chat-container quantumchat
docker cp secure-chat-container:/app/bin C:\Users\ASUS\Desktop\SecureChatApp
pause



