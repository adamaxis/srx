@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0build_map_entity_viewer.ps1"
if errorlevel 1 exit /b %errorlevel%
