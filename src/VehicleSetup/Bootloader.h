/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include "FirmwareImage.h"

#include <QSerialPort>

#include <stdint.h>

/// Bootloader Utility routines. Works with PX4 and 3DR Radio bootloaders.
class Bootloader : public QObject
{
    Q_OBJECT
    
public:
    explicit Bootloader(bool sikRadio, QObject *parent = 0);
    
    QString errorString(void) { return _errorString; }
    
    bool open               (const QString portName);
    void close              (void) { _port.close(); }
    bool getBoardInfo       (uint32_t& bootloaderVersion, uint32_t& boardID, uint32_t& flashSize);
    bool initFlashSequence  (void);
    bool erase              (void);
    bool program            (const FirmwareImage* image);
    bool verify             (const FirmwareImage* image);
    bool reboot             (void);
    
    // Supported bootloader board ids
    static const int boardIDPX4FMUV1    = 5;        ///< PX4 V1 board, as from USB PID
    static const int boardIDPX4FMUV2    = 9;        ///< PX4 V2 board, as from USB PID
    static const int boardIDPX4FMUV4    = 11;       ///< PX4 V4 board, as from USB PID
    static const int boardIDPX4FMUV4PRO = 13;       ///< PX4 V4PRO board, as from USB PID
    static const int boardIDPX4FMUV5    = 50;       ///< PX4 V5 board, as from USB PID
    static const int boardIDPX4Flow     = 6;        ///< PX4 Flow board, as from USB PID
    static const int boardIDAeroCore    = 98;       ///< Gumstix AeroCore board, as from USB PID
    static const int boardIDAUAVX2_1    = 33;       ///< AUAV X2.1 board, as from USB PID
    static const int boardID3DRRadio    = 78;       ///< 3DR Radio. This is an arbitrary value unrelated to the PID
    static const int boardIDMINDPXFMUV2 = 88;       ///< MindPX V2 board, as from USB PID
    static const int boardIDTAPV1       = 64;       ///< TAP V1 board, as from USB PID
    static const int boardIDASCV1       = 65;       ///< ASC V1 board, as from USB PID
    static const int boardIDCrazyflie2  = 12;       ///< Crazyflie 2.0 board, as from USB PID
    static const int boardIDOmnibusF4SD = 42;       ///< Omnibus F4 SD, as from USB PID
    static const int boardIDFMUK66V3    = 28;       ///< FMUK66V3 board, as from USB PID
    static const int boardIDKakuteF7    = 123;      ///< Holybro KakuteF7 board, as from USB PID
    static const int boardIDDurandalV1  = 139;      ///< Holybro Durandal-v1 board, as from USB PID
    static const int boardIDmRoCtrlZeroF7 = 141;    ///< mRo Control Zero F7 board, as from USB PID
    static const int boardIDModalFCV1   = 41775;    ///< ModalAI FC V1 board, as from USB PID
    static const int boardIDUVifyCore   = 20;       ///< UVify Core board, as from USB PID

    /// Simulated board id for V3 which is a V2 board which supports larger flash space
    /// IMPORTANT: Make sure this id does not conflict with any newly added real board ids
    static const int boardIDPX4FMUV3 = 255;

signals:
    /// @brief Signals progress indicator for long running bootloader utility routines
    void updateProgress(int curr, int total);
    
private:
    bool    _sync               (void);
    bool    _syncWorker         (void);
    bool    _binProgram         (const FirmwareImage* image);
    bool    _ihxProgram         (const FirmwareImage* image);
    bool    _write              (const uint8_t* data, qint64 maxSize);
    bool    _write              (const uint8_t byte);
    bool    _write              (const char* data);
    bool    _read               (uint8_t* data, qint64 cBytesExpected, int readTimeout = _readTimout);
    bool    _sendCommand        (uint8_t cmd, int responseTimeout = _responseTimeout);
    bool    _getCommandResponse (const int responseTimeout = _responseTimeout);
    bool    _protoGetDevice     (uint8_t param, uint32_t& value);
    bool    _verifyBytes        (const FirmwareImage* image);
    bool    _binVerifyBytes     (const FirmwareImage* image);
    bool    _ihxVerifyBytes     (const FirmwareImage* image);
    bool    _verifyCRC          (void);
    QString _getNextLine        (int timeoutMsecs);
    bool    _get3DRRadioBoardId (uint32_t& boardID);

    enum {
        // protocol bytes
        PROTO_INSYNC =          0x12,   ///< 'in sync' byte sent before status
        PROTO_BAD_SILICON_REV = 0x14,   ///< device is using silicon not suitable for the target the bootloader was used for
        PROTO_EOC =             0x20,   ///< end of command
        
        // Reply bytes
        PROTO_OK =              0x10,   ///< INSYNC/OK      - 'ok' response
        PROTO_FAILED =          0x11,   ///< INSYNC/FAILED  - 'fail' response
        PROTO_INVALID =         0x13,	///< INSYNC/INVALID - 'invalid' response for bad commands
        
        // Command bytes
        PROTO_GET_SYNC =        0x21,   ///< NOP for re-establishing sync
        PROTO_GET_DEVICE =      0x22,   ///< get device ID bytes
        PROTO_CHIP_ERASE =      0x23,   ///< erase program area and reset program address
        PROTO_LOAD_ADDRESS =    0x24,	///< set next programming address
        PROTO_PROG_MULTI =      0x27,   ///< write bytes at program address and increment
        PROTO_GET_CRC =         0x29,	///< compute & return a CRC
        PROTO_BOOT =            0x30,   ///< boot the application
        
        // Command bytes - Rev 2 boootloader only
        PROTO_CHIP_VERIFY	=   0x24, ///< begin verify mode
        PROTO_READ_MULTI	=   0x28, ///< read bytes at programm address and increment
        
        INFO_BL_REV         =   1,    ///< bootloader protocol revision
        BL_REV_MIN          =   2,    ///< Minimum supported bootlader protocol
        BL_REV_MAX			=   5,    ///< Maximum supported bootloader protocol
        INFO_BOARD_ID		=   2,    ///< board type
        INFO_BOARD_REV		=   3,    ///< board revision
        INFO_FLASH_SIZE		=   4,    ///< max firmware size in bytes
        
        PROG_MULTI_MAX		=   64,     ///< write size for PROTO_PROG_MULTI, must be multiple of 4
        READ_MULTI_MAX		=   0x28    ///< read size for PROTO_READ_MULTI, must be multiple of 4. Sik Radio max size is 0x28
    };
    
    QSerialPort _port;
    bool        _sikRadio           = false;
    bool        _inBootloaderMode   = false;    ///< true: board is in bootloader mode, false: special case for SiK Radio, board is in command mode
    uint32_t    _boardID            = 0;        ///< board id for currently connected board
    uint32_t    _boardFlashSize     = 0;        ///< flash size for currently connected board
    uint32_t    _bootloaderVersion  = 0;        ///< Bootloader version
    uint32_t    _imageCRC           = 0;        ///< CRC for image in currently selected firmware file
    QString     _firmwareFilename;              ///< Currently selected firmware file to flash
    QString     _errorString;                   ///< Last error
    
    static const int _eraseTimeout                      = 20000;    ///< Msecs to wait for response from erase command
    static const int _rebootTimeout                     = 10000;    ///< Msecs to wait for reboot command to cause serial port to disconnect
    static const int _verifyTimeout                     = 5000;     ///< Msecs to wait for response to PROTO_GET_CRC command
    static const int _readTimout                        = 2000;     ///< Msecs to wait for read bytes to become available
    static const int _responseTimeout                   = 2000;     ///< Msecs to wait for command response bytes
    static const int _flashSizeSmall                    = 1032192;  ///< Flash size for boards with silicon error
    static const int _bootloaderVersionV2CorrectFlash   = 5;        ///< Anything below this bootloader version on V2 boards cannot trust flash size
};
