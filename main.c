#include "embedul.ar/source/core/device/board.h"
#include "embedul.ar/source/core/anim.h"
#include "embedul.ar/source/core/misc/rgb888.h"
#include "embedul.ar/source/core/misc/hsl.h"
#include "log_brightness.h"


#define LOG_RECV_COMMAND                1
#define LOG_COMMAND_SET                 0

#define ESP32AT_HOSTNAME                "led-living"
#define ESP32AT_SSID                    "ner***"
#define ESP32AT_PASSWORD                "sal***"
#define ESP32AT_RETRY_DELAY             10000


enum Animation
{
    Animation_None,
    Animation_Random,
    Animation_Hue
};


static enum Animation s_AnimationBoard  = Animation_None;
static enum Animation s_AnimationSpots  = Animation_None;
static enum Animation s_AnimationStrip  = Animation_None;


static const uint8_t s_SpotMapping[16] =
{
    [0] = 14,
    [1] = 13,
    [2] = 12,
    [3] = 11,
    [4] = 10,
    [5] = 9,
    [6] = 8,
    [7] = 7,
    [8] = 6,
    [9] = 5,
    [10] = 4,
    [11] = 3,
    [12] = 2,
    [13] = 1,
    [14] = 0,
    [15] = 15
};

struct ANIM s_SpotTransition[16];

uint8_t g_stripHue;


static void setSpotIref (const uint8_t Number, const uint8_t Value)
{
    BOARD_AssertParams (Number < 16);

    const uint32_t IndexStart = Number * 3;

    MIO_SetOutputDeferred (OUTPUT_PROFILE_Group_LIGHTDEV, IO_Type_Range,
                           OUTPUT_PROFILE_LIGHTDEV_Range_Iref__BEGIN +
                           IndexStart + 0, Value);
    MIO_SetOutputDeferred (OUTPUT_PROFILE_Group_LIGHTDEV, IO_Type_Range,
                           OUTPUT_PROFILE_LIGHTDEV_Range_Iref__BEGIN +
                           IndexStart + 1, Value);
    MIO_SetOutputDeferred (OUTPUT_PROFILE_Group_LIGHTDEV, IO_Type_Range,
                           OUTPUT_PROFILE_LIGHTDEV_Range_Iref__BEGIN +
                           IndexStart + 2, Value);

    #if LOG_COMMAND_SET
        LOG (NOBJ, "spot `0, iref `1", Number, Value);
    #endif
}


static void setSpotPwm (const uint8_t Number, const uint8_t Value)
{
    BOARD_AssertParams (Number < 16);

    const uint32_t IndexStart = Number * 3;

    MIO_SetOutputDeferred (OUTPUT_PROFILE_Group_LIGHTDEV, IO_Type_Range,
                           OUTPUT_PROFILE_LIGHTDEV_Range_Pwm__BEGIN +
                           IndexStart + 0, Value);
    MIO_SetOutputDeferred (OUTPUT_PROFILE_Group_LIGHTDEV, IO_Type_Range,
                           OUTPUT_PROFILE_LIGHTDEV_Range_Pwm__BEGIN +
                           IndexStart + 1, Value);
    MIO_SetOutputDeferred (OUTPUT_PROFILE_Group_LIGHTDEV, IO_Type_Range,
                           OUTPUT_PROFILE_LIGHTDEV_Range_Pwm__BEGIN +
                           IndexStart + 2, Value);

    #if LOG_COMMAND_SET
        LOG (NOBJ, "spot `0, pwm `1", Number, Value);
    #endif
}


static void setSpots (const uint8_t *const Data)
{
    BOARD_AssertParams (Data);

    for (uint32_t i = 0; i < 16; ++i)
    {
        setSpotPwm (s_SpotMapping[i], Data[1 + i]);
    }
}


static void setSpotsToRandom (void)
{
    for (uint32_t i = 0; i < 16; ++i)
    {
        setSpotPwm (i, RANDOM_GetUint32InRange(0, 255));
    }
}


static void resetSpots (void)
{
    for (uint32_t i = 0; i < 16; ++i)
    {
        setSpotIref (i, 0xFF);
        setSpotPwm  (i, 0x00);

        ANIM_SetValue (&s_SpotTransition[i], 0x00);
    }

    #if LOG_RECV_COMMAND
        LOG (NOBJ, "spots reset");
    #endif
}


static uint8_t * beginStripUpdate (const uint16_t RGBStart)
{
    const struct SCREEN_Context *const C = SCREEN_GetContext (
                                                        SCREEN_Role_Primary);
    uint8_t *const Bb = VIDEO_Backbuffer(C->driver) + RGBStart * 3;

    return Bb;
}


static void endStripUpdate (const char *const Description,
                            const uint16_t RGBStart, const uint16_t RGBCount)
{
    #if LOG_COMMAND_SET
        LOG (NOBJ, "strip set from `0", Description);
        LOG (NOBJ, "rgb start `0, rgb count `1", RGBStart, RGBCount);
    #else
        (void) Description;
        (void) RGBStart;
        (void) RGBCount;
    #endif
}


static void setStripFromData (const uint16_t RGBStart, const uint16_t RGBCount,
                              const uint8_t *const Data)
{
    uint8_t *const Bb = beginStripUpdate (RGBStart);

    // For some reason the WS2811 12V led strip arranges components as BRG 
    for (uint32_t i = 0; i < RGBCount * 3; i += 3)
    {
        Bb[i + 0] = Data[i + 2];   // B
        Bb[i + 1] = Data[i + 0];   // R
        Bb[i + 2] = Data[i + 1];   // G
    }

    endStripUpdate ("data", RGBStart, RGBCount);
}


static void setStripToRandom (const uint16_t RGBStart, const uint16_t RGBCount)
{
    uint8_t *const Bb = beginStripUpdate (RGBStart);

    for (uint32_t i = 0; i < RGBCount * 3; i += 3)
    {
        Bb[i + 0] = RANDOM_GetUint32InRange (0, 255);
        Bb[i + 1] = RANDOM_GetUint32InRange (0, 255);
        Bb[i + 2] = RANDOM_GetUint32InRange (0, 255);
    }

    endStripUpdate ("random", RGBStart, RGBCount);
}


static void setStripToColor (const uint16_t RGBStart, const uint16_t RGBCount,
                             const uint8_t R, const uint8_t G, const uint8_t B)
{
    uint8_t *const Bb = beginStripUpdate (RGBStart);

    // For some reason the WS2811 12V led strip arranges components as BRG 
    for (uint32_t i = 0; i < RGBCount * 3; i += 3)
    {
        Bb[i + 0] = B;
        Bb[i + 1] = R;
        Bb[i + 2] = G;
    }

    endStripUpdate ("color", RGBStart, RGBCount);
}


static void setStripToHue (const uint16_t RGBStart, const uint16_t RGBCount)
{
    uint8_t *const Bb = beginStripUpdate (RGBStart);

    // For some reason the WS2811 12V led strip arranges components as BRG 
    for (uint32_t i = 0; i < RGBCount * 3; i += 3)
    {
        const struct RGB888 RGB = HSL_ToRgb (i + g_stripHue, 255, 64);//128);

        Bb[i+0] = RGB.b;   // B
        Bb[i+1] = RGB.r;   // R
        Bb[i+2] = RGB.g;   // G
    }

    g_stripHue += 2;

    endStripUpdate ("hue", RGBStart, RGBCount);
}


static void resetStrip (void)
{
    SCREEN_Zap (SCREEN_Role_Primary, 0x00);

    #if LOG_RECV_COMMAND
        LOG (NOBJ, "strip reset");
    #endif
}


static void setBoardLights (const uint8_t Warning, const uint8_t Backlight)
{
    MIO_SetOutputDeferred (OUTPUT_PROFILE_Group_SIGN, IO_Type_Bit,
                           OUTPUT_PROFILE_SIGN_Bit_Warning,
                           Warning);

    MIO_SetOutputDeferred (OUTPUT_PROFILE_Group_CONTROL, IO_Type_Bit,
                           OUTPUT_PROFILE_CONTROL_Bit_Backlight,
                           Backlight);

    #if LOG_RECV_COMMAND
        LOG (NOBJ, "board lights set");
    #endif
}


static void setBoardLightsToRandom (void)
{
    setBoardLights (RANDOM_GetUint32InRange(0, 1),
                    RANDOM_GetUint32InRange(0, 1));
}


static void resetBoardLights (void)
{
    setBoardLights (0, 0);
}


static void resetAnimation (void)
{
    s_AnimationBoard = Animation_None;
    s_AnimationSpots = Animation_None;
    s_AnimationStrip = Animation_None;

    g_stripHue = 0;

    #if LOG_RECV_COMMAND
        LOG (NOBJ, "animation reset");
    #endif
}


static void resetAll (void)
{
    resetBoardLights ();
    resetSpots ();
    resetStrip ();
    resetAnimation ();
}


static uint32_t getCommandSize (const uint8_t Command)
{
    switch (Command)
    {
        // A0 --> Full State (960 Octets)
        case 0xA0: return 960;
        // A1 --> Board Lights (8 Octets)
        case 0xA1: return 8;
        // A2 --> Spots Immediate (16 Bytes)
        case 0xA2: return 16;
        // A3 --> Spots Transition (16 Octets)
        case 0xA3: return 16;
        // A4 --> RGB Leds (936 Octets [312 * RGB])
        case 0xA4: return 936;
        // A5 --> Select RGB Led (5 Octets)
        case 0xA5: return 5;
        // A6 --> RGB Leds to Color (3 Octets)
        case 0xA6: return 3;
        // A7 --> State to Random or Reset (3 Bytes)
        case 0xA7: return 3;
        // A8 --> Animations (3 Bytes)
        case 0xA8: return 3;
    }

    return 0;
}


static void executeCommand (const uint8_t Command, const uint8_t *const Data)
{
    if (Command != 0xA8)
    {
        resetAnimation ();
    }

    switch (Command)
    {
        case 0xA0:
        {
            /*
                A0 --> Full State (960 Bytes)
                    Board Lights
                    [0] Warning
                    [1] Backlight
                    [2..7] N/A

                    16 Spots
                    [8..23]

                    312 * RGB Leds
                    [24..959]        
            */
            setBoardLights      (Data[0], Data[1]);
            setSpots            (&Data[8]);
            setStripFromData    (0, 312, &Data[24]);
            break;
        }

        case 0xA1:
        {
            /*
                A1 --> Board Lights (8 Bytes)
                    [0] Warning
                    [1] Backlight
                    [2..7] N/A
            */
            setBoardLights (Data[0], Data[1]);
            break;
        }

        case 0xA2:
        {
            /*
                A2 --> Spots Immediate (16 Bytes)
                    [0..15]
            */
            for (uint32_t i = 0; i < 16; ++i)
            {
                setSpotPwm (s_SpotMapping[i], Data[i]);
            }
            break;
        }

        case 0xA3:
        {
            /*
                A3 --> Spots Transition (16 Bytes)
                    [0..15]
            */

            const TIMER_Ticks Now = TICKS_Now ();

            for (uint32_t i = 0; i < 16; ++i)
            {
                const uint32_t Index = (uint32_t)(((float)(Data[i]) * 2047.0f)
                                            / 255.0f);

                ANIM_Start (&s_SpotTransition[s_SpotMapping[i]], 
                            ANIM_Type_Blink,
                            ANIM_GetValue(&s_SpotTransition[i]),
                            Index, i * 50, 600, 600, 0, Now);
            }
            break;
        }

        case 0xA4:
        {
            /*
                A4 --> RGB Leds (936 Bytes [312 * RGB])
                    [0..935]       
            */
            setStripFromData (0, 312, Data);
            break;
        }

        case 0xA5:
        {
            /*
                A5 --> Select RGB Led (5 Bytes)
                    [0] Led Low
                    [1] Led High
                    [2] Led R
                    [3] Led G
                    [4] Led B
            */
            const uint16_t LedIndex = Data[1] << 8 | Data[0];
            setStripToColor (LedIndex, 1, Data[2], Data[3], Data[4]);
            break;
        }

        case 0xA6:
        {
            /*
                A6 --> RGB Leds to Color (3 Bytes)
                    [0] Leds R
                    [1] Leds G
                    [2] Leds B
            */
            setStripToColor (0, 312, Data[0], Data[1], Data[2]);
            break;
        }

        case 0xA7:
        {
            /*
                A7 --> State to Random or Reset (3 Bytes)
                    [0] Board
                    [1] Spots
                    [2] Strip
            */
            if (Data[0])
            {
                setBoardLightsToRandom ();
            }
            else
            {
                resetBoardLights ();
            }

            if (Data[1])
            {
                setSpotsToRandom ();
            }
            else
            {
                resetSpots ();
            }

            if (Data[2])
            {
                setStripToRandom (0, 312);
            }
            else 
            {
                resetStrip (); 
            }
            break;
        }

        case 0xA8:
        {
            /*
                A8 --> Animations (3 Bytes)
                    [0] Board = 0: OFF, 1: Random
                    [1] Spots = 0: OFF, 1: Random
                    [2] Strip = 0: OFF, 1: Random, 2: Hue
            */
            s_AnimationBoard = Data[0];
            s_AnimationSpots = Data[1];
            s_AnimationStrip = Data[2];

            if (!s_AnimationBoard)
            {
                resetBoardLights ();
            }

            if (!s_AnimationSpots)
            {
                resetSpots ();
            }

            if (!s_AnimationStrip)
            {
                resetStrip ();
            }
            break;
        }
    }
}


int EMBEDULAR_Main (const int Argc, const char *const Argv[])
{
    (void) Argc;
    (void) Argv;

    resetAll ();

    if (!COMM_HasDevice (COMM_Device_IPNetwork))
    {
        LOG_Warn (NOBJ, "No comm ipnetwork device available");
        BOARD_AssertState (false);
    }

    struct STREAM *const IpNet = COMM_GetDevice (COMM_Device_IPNetwork);
//    struct STREAM *const Log = COMM_Stream (COMM_Stream_Log);

    STREAM_Command (IpNet, DEVICE_COMMAND_STREAM_SET_HOSTNAME,
                    &VARIANT_SpawnString(ESP32AT_HOSTNAME));

    STREAM_Command (IpNet, DEVICE_COMMAND_STREAM_SET_WIFI_SSID,
                    &VARIANT_SpawnString(ESP32AT_SSID));

    STREAM_Command (IpNet, DEVICE_COMMAND_STREAM_SET_PASSWORD,
                    &VARIANT_SpawnString(ESP32AT_PASSWORD));

#if 0
    struct STREAM *const Debug    = COMM_Stream (COMM_Stream_Log);
    struct STREAM *const Esp32at  = COMM_Stream (COMM_Stream_TCP_AT);

    while (1)
    {
        INPUT_Update ();

        STREAM_IN_FromStream    (Esp32at, Debug);
        STREAM_OUT_ToStream     (Esp32at, Debug);

        BOARD_Delay (2);

        if (INPUT_SWITCH_ACTION(BoardA, Clicked))
        {
            LOG (NODEV, "Enviando AT+GMR");
            STREAM_IN_FromString (Esp32at, "AT+GMR\r\n");
        }
    }
#endif

    uint8_t         commandData[1024];
    TIMER_Ticks     nextConnectionCheck = 0;

    while (1)
    {
        // Max. time to wait for `command id` (must be already there)
        STREAM_Timeout (IpNet, 0);

        const uint8_t Command = STREAM_OUT_ToOctet (IpNet);

        if (!STREAM_Count (IpNet))
        {
            // `command id` not available. If connection check is due, assert
            // connection and reconnect if needed.
            if (nextConnectionCheck < TICKS_Now())
            {
                nextConnectionCheck = TICKS_Now() + 5 * 1000;

                if (!STREAM_IsConnected (IpNet))
                {
                    STREAM_Connect (IpNet);
                }
            }
        }
        else if ((Command & 0xF0) != 0xA0)
        {
            // Unrecognized `command id`
            STREAM_OUT_Discard (IpNet);

            LOG_Warn (NOBJ, "invalid command id");
            LOG_Items (1,
                        "command", Command,
                        LOG_ItemsBases (VARIANT_Base_Hex));
        }
        else
        {
            // Max. time to wait for command data.
            STREAM_Timeout (IpNet, 12);

            const uint32_t DataSize = getCommandSize (Command);

            STREAM_OUT_ToBuffer (IpNet, commandData, DataSize);

            const uint32_t RecvSize = STREAM_Count (IpNet);

            if (RecvSize != DataSize)
            {
                LOG_Warn (NOBJ, "invalid packet size");
                LOG_Items (4,   
                            "command",          Command,
                            "required size",    DataSize,
                            "received size",    RecvSize,
                            "transfer status",  STREAM_TransferStatus (IpNet),
                            LOG_ItemsBases (VARIANT_Base_Hex, 0, 0, 0));
                LOG_BinaryDump (NOBJ, "data", commandData, RecvSize);

                // Check STREAM_TransferStatus ()
            }
            else 
            {
                executeCommand (Command, commandData);
            }
        }

        if (s_AnimationBoard == Animation_Random)
        {
            setBoardLightsToRandom ();
        }

        if (s_AnimationSpots == Animation_Random)
        {
            setSpotsToRandom ();
        }

        switch (s_AnimationStrip)
        {
            case Animation_None:
                break;

            case Animation_Random:
                setStripToRandom (0, 312);
                break;

            case Animation_Hue:
                setStripToHue (0, 312);
                break;
        }

        const TIMER_Ticks Now = TICKS_Now ();

        for (uint32_t i = 0; i < 16; ++i)
        {
            if (ANIM_Pending (&s_SpotTransition[i]))
            {
                ANIM_Update (&s_SpotTransition[i], Now);

                const uint16_t BrightnessIndex = 
                    ANIM_GetValue(&s_SpotTransition[i]) & LOG_BRIGHTNESS_MASK;

                setSpotPwm (i, s_LogBrightness[BrightnessIndex]);
            }
        }

        BOARD_Sync ();
    }

    return 0;
}
