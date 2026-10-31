cmake_minimum_required(VERSION 3.21)
project(KyronextRecovery VERSION 0.1.0 LANGUAGES NONE)

install(PROGRAMS kyronext_recovery.py
        DESTINATION bin
        RENAME kyronext-recovery)
